
//OpenSCADA system module UI.VCAEngine file: session.cpp
/***************************************************************************
 *   Copyright (C) 2007-2008 by Roman Savochenko                           *
 *   rom_as@fromru.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <pthread.h>
#include <signal.h>

#include <tsys.h>

#include "vcaengine.h"
#include "session.h"


using namespace VCA;

//************************************************
//* Session: Project's session			 *
//************************************************
Session::Session( const string &iid, const string &iproj ) :
    mEnable(false), mStart(false), endrun_req(false), tm_calc(0.0),
    mUser("root"), mPrjnm(iproj), mPer(100), mCalcClk(1),
    mOwner("root"), mGrp("UI"), mPermit(R_R_R_),
    mBackgrnd(false), mConnects(0)
{
    mId = iid;
    mPage = grpAdd("pg_");
    sec = SYS->security();
}

Session::~Session( )
{

}

void Session::postEnable( int flag )
{
    if( flag&TCntrNode::NodeRestore )	setEnable(true);
}

void Session::preDisable( int flag )
{
    if( enable() )  setEnable(false);
}

void Session::setEnable( bool val )
{
    if( val == enable() )	return;

    vector<string> pg_ls;

    if( val )
    {
	try
	{
	    //- Connect to project -
	    mParent = mod->prjAt(mPrjnm);

	    //- Get data from project -
	    mOwner	= parent().at().owner( );
	    mGrp	= parent().at().grp( );
	    mPermit	= parent().at().permit( );
	    setPeriod( parent().at().period( ) );

	    //- Create root pages -
	    parent().at().list(pg_ls);
	    for( int i_p = 0; i_p < pg_ls.size(); i_p++ )
		if( !present(pg_ls[i_p]) )
		    add(pg_ls[i_p],parent().at().at(pg_ls[i_p]).at().path());

	    //- Pages enable -
	    list(pg_ls);
	    for( int i_ls = 0; i_ls < pg_ls.size(); i_ls++ )
		try{ at(pg_ls[i_ls]).at().setEnable(true); }
		catch( TError err )	{ mess_err( err.cat.c_str(), "%s", err.mess.c_str() ); }
	
	    modifGClr();
	}
	catch(...){ mParent.free(); }
    }
    else
    {
	if( start() )	setStart(false);

	//- Pages disable -
	list(pg_ls);
	for( int i_ls = 0; i_ls < pg_ls.size(); i_ls++ )
	    at(pg_ls[i_ls]).at().setEnable(false);

	//- Delete pages -
	for( int i_ls = 0; i_ls < pg_ls.size(); i_ls++ )
	    del(pg_ls[i_ls]);

	//- Disconnect from project -
	mParent.free();
    }

    mEnable = val;
}

void Session::setStart( bool val )
{
    vector<string> pg_ls;

    if( val )
    {
	//- Enable session if it disabled -
	if( !enable() )	setEnable(true);

	//- Process all pages is on -
	list(pg_ls);
	for( int i_ls = 0; i_ls < pg_ls.size(); i_ls++ )
	    at(pg_ls[i_ls]).at().setProcess(true);

	//- Start process task -
	if( !mStart )
	{
	    pthread_attr_t pthr_attr;
	    pthread_attr_init(&pthr_attr);
	    struct sched_param prior;
	    pthread_attr_setschedpolicy(&pthr_attr,SCHED_OTHER);
	    prior.__sched_priority=2;
	    pthread_attr_setschedparam(&pthr_attr,&prior);

	    pthread_create(&calcPthr,&pthr_attr,Session::Task,this);
	    pthread_attr_destroy(&pthr_attr);
	    if( TSYS::eventWait(mStart, true, nodePath()+"start",5) )
		throw TError(nodePath().c_str(),_("Session processing task is not started!"));
	}
    }
    else
    {
	//- Stop process task -
	if( mStart )
	{
	    endrun_req = true;
	    pthread_kill( calcPthr, SIGALRM );
	    if( TSYS::eventWait(mStart,false,nodePath()+"stop",5) )
		throw TError(nodePath().c_str(),_("Sesion processing task is not stoped!"));
	    pthread_join( calcPthr, NULL );
	}

	//- Process all pages is off -
	list(pg_ls);
	for( int i_ls = 0; i_ls < pg_ls.size(); i_ls++ )
	    at(pg_ls[i_ls]).at().setProcess(false);
    }
}

string Session::ico( )
{
    if( !parent().freeStat() )	return parent().at().ico();
    return "";
}

AutoHD<Project> Session::parent( )
{
    return mParent;
}

void Session::add( const string &iid, const string &iparent )
{
    if( present(iid) )	return;
    chldAdd(mPage,new SessPage(iid,iparent,this));
}

void Session::openReg( const string &iid )
{
    int i_op;
    for( i_op = 0; i_op < mOpen.size(); i_op++ )
	if( iid == mOpen[i_op] ) break;
    if( i_op >= mOpen.size() )	mOpen.push_back(iid);
}

void Session::openUnreg( const string &iid )
{
    for( int i_op = 0; i_op < mOpen.size(); i_op++ )
	if( iid == mOpen[i_op] ) mOpen.erase(mOpen.begin()+i_op);
}

AutoHD<SessPage> Session::at( const string &id )
{
    return chldAt(mPage,id);
}

void Session::uiComm( const string &com, const string &prm, SessWdg *src )
{
    //> Find of pattern adequancy for opened page
    string oppg;		//Opened page according of pattern

    vector<string> &op_ls = openList();
    for( int i_op = 0; i_op < op_ls.size(); i_op++ )
    {
	string cur_pt_el, cur_el;
	int i_el = 0;
	while((cur_pt_el=TSYS::pathLev(prm,i_el++)).size())
	{
	    cur_el = TSYS::pathLev(op_ls[i_op],i_el);
	    if( cur_el.empty() || (cur_pt_el.substr(0,3) == "pg_" && cur_pt_el != cur_el) ) break;
	}
	if( cur_pt_el.empty() ) { oppg = op_ls[i_op]; break; }
    }
    //> Individual commands process
    try
    {
	//>> Go to destination page
	string cur_pt_el;
	int i_el = 0;
	AutoHD<SessPage> cpg;
	while((cur_pt_el=TSYS::pathLev(prm,i_el++)).size())
	{
	    string op_pg;
	    if( cur_pt_el.substr(0,3) == "pg_" ) op_pg = cur_pt_el.substr(3);
	    else if( cur_pt_el == "*" || (cur_pt_el == "$" && ( com == "next" || com == "prev")) )
	    {
		vector<string> pls;
		if( cpg.freeStat() ) list(pls); else cpg.at().pageList(pls);
		if( pls.empty() )	return;
		string cur_el = TSYS::pathLev(oppg,i_el);
		if( cur_el.empty() )
		{
		    if( cur_pt_el == "$" )	return;
		    op_pg = pls[0];
		}
		else
		{
		    cur_el = cur_el.substr(3);
		    int i_l;
		    for( i_l = 0; i_l < pls.size(); i_l++ )
			if( cur_el == pls[i_l] ) break;
		    if( i_l < pls.size() )
		    {
			if( cur_pt_el == "$" )
			{
			    if( com == "next" ) i_l++;
			    if( com == "prev" ) i_l--;
			    i_l = (i_l < 0) ? pls.size()-1 : (i_l >= pls.size()) ? 0 : i_l;
			    op_pg = pls[i_l];
			    if( op_pg == cur_el ) return;
			}
			else op_pg = cur_el;
		    }
		    else
		    {
			if( cur_pt_el == "$" )  return;
			op_pg = pls[0];
		    }
		}
	    }
	    else op_pg = cur_pt_el;
	    //>> Go to next page
	    cpg = cpg.freeStat() ? at(op_pg) : cpg.at().pageAt(op_pg);
	}
	//> Open found page
	if( !cpg.freeStat() )
	{
	    //>> Close previous page
	    if( !oppg.empty() )	((AutoHD<SessPage>)mod->nodeAt(oppg)).at().attrAt("pgOpen").at().setB(false);
	    //>>> Set interwidget's links for new page
	    bool emptyPresnt = false;
	    string atr_id, prm_lnk;
	    vector<string> cAtrLs;
	    cpg.at().attrList(cAtrLs);
	    for( int i_al = 0; i_al < cAtrLs.size(); i_al++ )
	    {
		AutoHD<Attr> attr = cpg.at().attrAt(cAtrLs[i_al]);
		if( !(attr.at().flgSelf()&(Attr::CfgLnkIn|Attr::CfgLnkOut) && TSYS::strSepParse(attr.at().cfgTempl(),0,'|') == "<page>" ) ) continue;
		atr_id = TSYS::strSepParse(attr.at().cfgTempl(),1,'|');
		if( src->attrPresent(atr_id) )
		{
		    if( src->attrAt(atr_id).at().cfgVal().size() > 4 && src->attrAt(atr_id).at().cfgVal().substr(0,4) == "prm:" )
		    {
			if( prm_lnk.empty() ) prm_lnk = src->attrAt(atr_id).at().cfgVal().substr(4);
			attr.at().setCfgVal(src->attrAt(atr_id).at().cfgVal());
		    }
		    else attr.at().setCfgVal("wdg:"+src->path()+"/a_"+atr_id);
//		    if( prm_lnk.empty() && src->attrAt(atr_id).at().cfgVal().size() > 4 &&
//			    src->attrAt(atr_id).at().cfgVal().substr(0,4) == "prm:" )
//			prm_lnk = src->attrAt(atr_id).at().cfgVal().substr(4);
		}
		else
		{
		    attr.at().setCfgVal("");
		    attr.at().setS(EVAL_STR);
		    emptyPresnt = true;
		}
	    }
	    //>>> Fill parameter's links for other attributes
	    if( emptyPresnt && !prm_lnk.empty() )
	    {
		AutoHD<TValue> prml;
		prm_lnk = "/"+TSYS::pathLev(prm_lnk,0)+"/"+TSYS::pathLev(prm_lnk,1)+"/"+TSYS::pathLev(prm_lnk,2);
		try{ prml = SYS->daq().at().nodeAt(prm_lnk); } catch(TError err) { }
		for( int i_al = 0; !prml.freeStat() && i_al < cAtrLs.size(); i_al++ )
		{
		    AutoHD<Attr> attr = cpg.at().attrAt(cAtrLs[i_al]);
		    if( !(attr.at().flgSelf()&(Attr::CfgLnkIn|Attr::CfgLnkOut) &&
			    TSYS::strSepParse(attr.at().cfgTempl(),0,'|') == "<page>" &&
			    (attr.at().cfgVal().empty() || attr.at().flgGlob()&Attr::Address) ) )	continue;
		    atr_id = TSYS::strSepParse(attr.at().cfgTempl(),1,'|');
		    if( prml.at().vlPresent(atr_id) )	attr.at().setCfgVal("prm:"+prm_lnk+"/a_"+atr_id);
		}
	    }
	    //>>> Open new page
	    cpg.at().attrAt("pgOpen").at().setB(true);
	}
    }catch(...){ }
}

void Session::alarmSet( const string &wpath, const string &alrm )
{
    if( wpath.empty() ) return;

    //- Alarms queue process -
    ResAlloc res( mAlrmRes, true );

    Alarm aobj( wpath, alrm, calcClk( ) );

    int i_q = 0;
    while( i_q < mAlrm.size() && mAlrm[i_q].path != aobj.path )	i_q++;
    if( !aobj.lev )
    {
	if( i_q < mAlrm.size() ) mAlrm.erase( mAlrm.begin()+i_q );
	return;
    }
    if( i_q < mAlrm.size() && aobj.lev == mAlrm[i_q].lev )	mAlrm[i_q] = aobj;
    else
    {
	if( i_q < mAlrm.size() )	mAlrm.erase( mAlrm.begin()+i_q );
	int i_q1 = 0;
	while( i_q1 < mAlrm.size() && aobj.lev >= mAlrm[i_q].lev ) i_q1++;
	if( i_q1 < mAlrm.size() )	mAlrm.insert(mAlrm.begin()+i_q1,aobj);
	else mAlrm.push_back(aobj);
    }
}

int Session::alarmStat( )
{
    ui8 alev = 0, atp = 0, aqtp = 0;
    vector<string> ls;
    list( ls );
    for( int i_p = 0; i_p < ls.size(); i_p++ )
    {
	int ast = at(ls[i_p]).at().attrAt("alarmSt").at().getI();
	alev = vmax(alev,ast&0xFF);
	atp |= (ast>>8)&0xFF;
	aqtp |= (ast>>16)&0xFF;
    }

    return (aqtp<<16)|(atp<<8)|alev;
}

void Session::alarmQuittance( const string &wpath, ui8 quit_tmpl )
{
    if( !wpath.empty() ) ((AutoHD<SessWdg>)mod->nodeAt(wpath)).at().alarmQuittance( quit_tmpl, true );
    else
    {
	vector<string> ls;
	list( ls );
	for( int i_p = 0; i_p < ls.size(); i_p++ )
	    at(ls[i_p]).at().alarmQuittance( quit_tmpl, true );
    }

    //- Queue alarms quittance -
    ResAlloc res( mAlrmRes, false );

    for( int i_q = 0; i_q < mAlrm.size(); i_q++ )
	if( mAlrm[i_q].path.substr(0,wpath.size()) == wpath )
	    mAlrm[i_q].qtp &= quit_tmpl;
}

void *Session::Task( void *icontr )
{
    vector<string> pls;
    Session &ses = *(Session *)icontr;

#if OSC_DEBUG >= 2
    mess_debug(ses.nodePath().c_str(),_("Thread <%u> is started. TID: %ld"),pthread_self(),(long int)syscall(224));
#endif

    ses.endrun_req = false;
    ses.mStart    = true;

    ses.list(pls);
    while( !ses.endrun_req )
    {
	//- Check calk time -
	unsigned long long t_cnt = SYS->shrtCnt();

	//- Calc session pages and all other items at recursion -
	for( int i_l = 0; i_l < pls.size(); i_l++ )
	    try{ ses.at(pls[i_l]).at().calc(false,false); }
	    catch( TError err )
	    {
		mess_err(err.cat.c_str(),"%s",err.mess.c_str());
		mess_err(ses.nodePath().c_str(),_("Session '%s' calc error."),pls[i_l].c_str());
	    }

	if( (ses.mCalcClk++) == 0 ) ses.mCalcClk = 1;

	ses.tm_calc = 1.0e3*((float)(SYS->shrtCnt()-t_cnt))/((float)SYS->sysClk());
	/*ses.rez_calc+=ses.tm_calc;
	if( !(ses.calcClk()%10) )
	{
	    printf("Session calc time: %d = %f\n",ses.calcClk(),ses.rez_calc);
	    ses.rez_calc=0;
	}*/

	TSYS::taskSleep((long long)ses.period()*1000000);
    }

    ses.mStart = false;

    return NULL;
}

void Session::cntrCmdProc( XMLNode *opt )
{
    string a_path = opt->attr("path");
    //- Service commands process -
    if( a_path == "/serv/pg" )	//Pages operations
    {
	if( ctrChkNode(opt,"openlist",permit(),owner().c_str(),grp().c_str(),SEQ_RD) )	//Open pages list
	{
	    unsigned tm = strtoul(opt->attr("tm").c_str(),NULL,10);
	    vector<string> &lst = openList();
	    for( int i_f = 0; i_f < lst.size(); i_f++ )
	    {
		XMLNode *pel = opt->childAdd("pg");
		pel->setText(lst[i_f]);

		if( tm )
		{
		    //- Permission to view page check -
		    AutoHD<SessWdg> swdg = nodeAt(lst[i_f],1);
		    if( !SYS->security().at().access(opt->attr("user"),SEQ_RD,swdg.at().owner(),swdg.at().grp(),swdg.at().permit()) )
			continue;
		    //- Changed widgets list add -
		    vector<string> updEl;
		    swdg.at().getUpdtWdg("",tm,updEl);
		    pel->setAttr("updWdg",TSYS::int2str(updEl.size()));
		}
	    }
	    opt->setAttr("tm",TSYS::uint2str(calcClk()));
	}
	else if( ctrChkNode(opt,"open",permit(),owner().c_str(),grp().c_str(),SEQ_WR) )		//Open pages
	    ((AutoHD<SessWdg>)nodeAt(opt->attr("pg"),1)).at().attrAt("pgOpen").at().setB(true);
	else if( ctrChkNode(opt,"close",permit(),owner().c_str(),grp().c_str(),SEQ_WR) )	//Close open pages
	    ((AutoHD<SessWdg>)nodeAt(opt->attr("pg"),1)).at().attrAt("pgOpen").at().setB(false);
	return;
    }
    else if( a_path == "/serv/alarm" )	//Alarm operations
    {
	if( ctrChkNode(opt,"get",permit(),owner().c_str(),grp().c_str(),SEQ_RD) )
	{
	    //- Get alarm status -
	    opt->setAttr("alarmSt",TSYS::int2str(alarmStat()));

	    //- Get alarm from sound queue -
	    if( opt->attr("mode") == "sound" )
	    {
		unsigned a_tm  = strtoul(opt->attr("tm").c_str(),NULL,10);
		opt->setAttr("tm",TSYS::uint2str(calcClk()));

		//- Find event, return it and alarm resource -
		ResAlloc res( mAlrmRes, false );
		string wdg = opt->attr("wdg");
		int i_q, i_first = -1, i_next = -1;
		for( i_q = mAlrm.size()-1; i_q >= 0; i_q-- )
		{
		    if( !(mAlrm[i_q].qtp & Engine::Sound) ) continue;
		    if( wdg.empty() || mAlrm[i_q].clc >= a_tm || i_next > 0 )	break;	//First, new and next alarms break
		    if( i_first < 0 ) i_first = i_q;
		    if( wdg == mAlrm[i_q].path ) i_next = i_q;
		}
		if( i_q < 0 && i_first >= 0 ) i_q = i_first;
		if( i_q >= 0 )
		{
		    opt->setAttr( "wdg", mAlrm[i_q].path );
		    if( !mAlrm[i_q].tpArg.empty() )
			opt->setText( ((AutoHD<SessWdg>)mod->nodeAt(mAlrm[i_q].path)).at().resourceGet(mAlrm[i_q].tpArg) );
		    else opt->setText( mod->callSynth(mAlrm[i_q].mess) );
		}
	    }
	}
	else if( ctrChkNode(opt,"quittance",permit(),owner().c_str(),grp().c_str(),SEQ_WR) )
	    alarmQuittance(opt->attr("wdg"),~atoi(opt->attr("tmpl").c_str()));
	return;
    }

    //- Get page info -
    if( opt->name() == "info" )
    {
	TCntrNode::cntrCmdProc(opt);
	ctrMkNode("oscada_cntr",opt,-1,"/",_("Session: ")+id(),permit(),owner().c_str(),grp().c_str());
	if(ico().size()) ctrMkNode("img",opt,-1,"/ico","",R_R_R_);
	if(ctrMkNode("branches",opt,-1,"/br","",R_R_R_))
	    ctrMkNode("grp",opt,-1,"/br/pg_",_("Page"),R_R_R_,"root","UI",1,"idm","1");
	if(ctrMkNode("area",opt,-1,"/obj",_("Session")))
	{
	    if(ctrMkNode("area",opt,-1,"/obj/st",_("State")))
	    {
		ctrMkNode("fld",opt,-1,"/obj/st/en",_("Enable"),permit(),owner().c_str(),grp().c_str(),1,"tp","bool");
		ctrMkNode("fld",opt,-1,"/obj/st/start",_("Start"),permit(),owner().c_str(),grp().c_str(),1,"tp","bool");
		ctrMkNode("fld",opt,-1,"/obj/st/user",_("User"),R_R_R_,"root","UI",1,"tp","str");
		ctrMkNode("fld",opt,-1,"/obj/st/owner",_("Owner"),R_R_R_,"root","UI",1,"tp","str");
		ctrMkNode("fld",opt,-1,"/obj/st/grp",_("Group"),R_R_R_,"root","UI",1,"tp","str");
		ctrMkNode("fld",opt,-1,"/obj/st/u_a",_("Access"),R_R_R_,"root","UI",4,"tp","dec","dest","select",
		    "sel_id","0;4;6","sel_list",_("No access;View;View and control"));
		ctrMkNode("fld",opt,-1,"/obj/st/g_a","",R_R_R_,"root","UI",4,"tp","dec","dest","select",
		    "sel_id","0;4;6","sel_list",_("No access;View;View and control"));
		ctrMkNode("fld",opt,-1,"/obj/st/o_a","",R_R_R_,"root","UI",4,"tp","dec","dest","select",
		    "sel_id","0;4;6","sel_list",_("No access;View;View and control"));
		ctrMkNode("fld",opt,-1,"/obj/st/prj",_("Project"),permit(),owner().c_str(),grp().c_str(),3,"tp","str","dest","sel_ed","select","/obj/prj_ls");
		ctrMkNode("fld",opt,-1,"/obj/st/backgrnd",_("Background execution mode"),R_R_R_,"root","UI",1,"tp","bool");
		ctrMkNode("fld",opt,-1,"/obj/st/connect",_("Connections counter"),R_R_R_,"root","UI",1,"tp","int");
		ctrMkNode("fld",opt,-1,"/obj/st/calc_tm",_("Calc session time (ms)"),R_R_R_,"root","UI",1,"tp","real");
	    }
	    if(ctrMkNode("area",opt,-1,"/obj/cfg",_("Config")))
	    {
		ctrMkNode("fld",opt,-1,"/obj/cfg/per",_("Period (ms)"),permit(),owner().c_str(),grp().c_str(),1,"tp","dec");
		ctrMkNode("list",opt,-1,"/obj/cfg/openPg",_("Opened pages"),R_R_R_,"root","UI",1,"tp","str");
	    }
	}
	if(ctrMkNode("area",opt,-1,"/page",_("Pages")))
	    ctrMkNode("list",opt,-1,"/page/page",_("Pages"),R_R_R_,"root","UI",3,"tp","br","idm","1","br_pref","pg_");
	if(ctrMkNode("area",opt,-1,"/alarm",_("Alarms")))
	    if(ctrMkNode("table",opt,-1,"/alarm/alarm",_("Alarms list"),R_R_R_,"root","UI"))
	    {
		ctrMkNode("list",opt,-1,"/alarm/alarm/wdg",_("Widget"),R_R_R_,"root","UI",1,"tp","str");
		ctrMkNode("list",opt,-1,"/alarm/alarm/lev",_("Level"),R_R_R_,"root","UI",1,"tp","dec");
		ctrMkNode("list",opt,-1,"/alarm/alarm/cat",_("Category"),R_R_R_,"root","UI",1,"tp","str");
		ctrMkNode("list",opt,-1,"/alarm/alarm/mess",_("Messages"),R_R_R_,"root","UI",1,"tp","str");
		ctrMkNode("list",opt,-1,"/alarm/alarm/tp",_("Type"),R_R_R_,"root","UI",1,"tp","hex");
		ctrMkNode("list",opt,-1,"/alarm/alarm/tpArg",_("Type argument"),R_R_R_,"root","UI",1,"tp","str");
	    }
	return;
    }

    //- Process command to page -
    if( a_path == "/ico" && ctrChkNode(opt) )   opt->setText(ico());
    else if( a_path == "/obj/st/en" )
    {
	if( ctrChkNode(opt,"get",permit(),owner().c_str(),grp().c_str(),SEQ_RD) )	opt->setText(TSYS::int2str(enable()));
	if( ctrChkNode(opt,"set",permit(),owner().c_str(),grp().c_str(),SEQ_WR) )	setEnable(atoi(opt->text().c_str()));
    }
    else if( a_path == "/obj/st/start" )
    {
	if( ctrChkNode(opt,"get",permit(),owner().c_str(),grp().c_str(),SEQ_RD) )	opt->setText(TSYS::int2str(start()));
	if( ctrChkNode(opt,"set",permit(),owner().c_str(),grp().c_str(),SEQ_WR) )	setStart(atoi(opt->text().c_str()));
    }
    else if( a_path == "/obj/st/user" && ctrChkNode(opt) )	opt->setText(user());
    else if( a_path == "/obj/st/owner" && ctrChkNode(opt) )	opt->setText(owner());
    else if( a_path == "/obj/st/grp" && ctrChkNode(opt) )	opt->setText(grp());
    else if( (a_path == "/obj/st/u_a" || a_path == "/obj/st/g_a" || a_path == "/obj/st/o_a") && ctrChkNode(opt) )
    {
	if( a_path == "/obj/st/u_a" )	opt->setText( TSYS::int2str((permit()>>6)&0x7) );
	if( a_path == "/obj/st/g_a" )	opt->setText( TSYS::int2str((permit()>>3)&0x7) );
	if( a_path == "/obj/st/o_a" )	opt->setText( TSYS::int2str(permit()&0x7) );
    }
    else if( a_path == "/obj/st/prj" )
    {
	if( ctrChkNode(opt,"get",permit(),owner().c_str(),grp().c_str(),SEQ_RD) )	opt->setText(projNm());
	if( ctrChkNode(opt,"set",permit(),owner().c_str(),grp().c_str(),SEQ_WR) )	setProjNm(opt->text());
    }
    else if( a_path == "/obj/st/backgrnd" && ctrChkNode(opt) )	opt->setText(TSYS::int2str(backgrnd()));
    else if( a_path == "/obj/st/connect" && ctrChkNode(opt) )	opt->setText(TSYS::int2str(connects()));
    else if( a_path == "/obj/st/calc_tm" && ctrChkNode(opt) )	opt->setText(TSYS::real2str(calcTm(),6));
    else if( a_path == "/obj/prj_ls" && ctrChkNode(opt) )
    {
	vector<string> lst;
	mod->prjList(lst);
	for( unsigned i_f=0; i_f < lst.size(); i_f++ )
	    opt->childAdd("el")->setAttr("id",lst[i_f])->setText(mod->prjAt(lst[i_f]).at().name());
    }
    else if( a_path == "/obj/cfg/per" )
    {
	if( ctrChkNode(opt,"get",permit(),owner().c_str(),grp().c_str(),SEQ_RD) )	opt->setText(TSYS::int2str(period()));
	if( ctrChkNode(opt,"set",permit(),owner().c_str(),grp().c_str(),SEQ_WR) )	setPeriod(atoi(opt->text().c_str()));
    }
    else if( a_path == "/obj/cfg/openPg" && ctrChkNode(opt) )
    {
	vector<string> &lst = openList();
	for( unsigned i_f=0; i_f < lst.size(); i_f++ )
	    opt->childAdd("el")->setText(lst[i_f]);
    }
    else if( (a_path == "/br/pg_" || a_path == "/page/page") && ctrChkNode(opt) )
    {
	vector<string> lst;
	list(lst);
	for( unsigned i_f=0; i_f < lst.size(); i_f++ )
	    opt->childAdd("el")->setAttr("id",lst[i_f])->setText(at(lst[i_f]).at().name());
    }
    else if( a_path == "/alarm/alarm" && ctrChkNode(opt) )
    {
	//Fill Archivators table
	XMLNode *n_wdg	= ctrMkNode("list",opt,-1,"/alarm/alarm/wdg","",0444);
	XMLNode *n_lev	= ctrMkNode("list",opt,-1,"/alarm/alarm/lev","",0444);
	XMLNode *n_cat	= ctrMkNode("list",opt,-1,"/alarm/alarm/cat","",0444);
	XMLNode *n_mess	= ctrMkNode("list",opt,-1,"/alarm/alarm/mess","",0444);
	XMLNode *n_tp	= ctrMkNode("list",opt,-1,"/alarm/alarm/tp","",0444);
	XMLNode *n_tpArg= ctrMkNode("list",opt,-1,"/alarm/alarm/tpArg","",0444);

	ResAlloc res( mAlrmRes, false );
	for( int i_q = mAlrm.size()-1; i_q >= 0; i_q-- )
	{
	    if( n_wdg )	n_wdg->childAdd("el")->setText( mAlrm[i_q].path );
	    if( n_lev )	n_lev->childAdd("el")->setText( TSYS::int2str(mAlrm[i_q].lev) );
	    if( n_cat )	n_cat->childAdd("el")->setText( mAlrm[i_q].cat );
	    if( n_mess )n_mess->childAdd("el")->setText( mAlrm[i_q].mess );
	    if( n_tp )	n_tp->childAdd("el")->setText( TSYS::int2str(mAlrm[i_q].tp) );
	    if( n_tpArg ) n_tpArg->childAdd("el")->setText( mAlrm[i_q].tpArg );
	}
    }
    else TCntrNode::cntrCmdProc(opt);
}

//* Alarm: Alarm object				 *
//************************************************
Session::Alarm::Alarm( const string &ipath, const string &alrm, unsigned iclc ) : path(ipath), clc(iclc)
{
    int a_off = 0;
    lev   = atoi( TSYS::strSepParse(alrm,0,'|',&a_off).c_str() );
    cat   = TSYS::strSepParse(alrm,0,'|',&a_off);
    mess  = TSYS::strSepParse(alrm,0,'|',&a_off);
    qtp   = tp = atoi( TSYS::strSepParse(alrm,0,'|',&a_off).c_str() );
    tpArg = TSYS::strSepParse(alrm,0,'|',&a_off);
}

//************************************************
//* SessPage: Page of Project's session          *
//************************************************
SessPage::SessPage( const string &iid, const string &ipage, Session *sess ) : SessWdg(iid,ipage,sess)
{
    mPage = grpAdd("pg_");
}

SessPage::~SessPage( )
{

}

string SessPage::path( )
{
    return ownerFullId(true)+"/pg_"+id();
}

void SessPage::setEnable( bool val )
{
    if( val == enable() ) return;

    if( !val )
    {
	vector<string> pg_ls;
	//- Unregister opened page -
	if( !(parent().at().prjFlags( )&Page::Empty) && attrPresent("pgOpen") && attrAt("pgOpen").at().getB() ) 
	    ownerSess()->openUnreg(path());
	//- Disable include pages -
	pageList(pg_ls);
	for(int i_l = 0; i_l < pg_ls.size(); i_l++ )
	    pageAt(pg_ls[i_l]).at().setEnable(false);
	//- Delete included widgets -
	for(int i_l = 0; i_l < pg_ls.size(); i_l++ )
	    pageDel(pg_ls[i_l]);
    }

    //- Call parrent enable method -
    SessWdg::setEnable(val);

    if( val )
    {
	vector<string> pg_ls;
	//- Register opened page -
	if( !(parent().at().prjFlags( )&Page::Empty) && attrAt("pgOpen").at().getB() ) ownerSess()->openReg(path());
	//- Create included pages -
	parent().at().pageList(pg_ls);
	for( int i_p = 0; i_p < pg_ls.size(); i_p++ )
	    if( !pagePresent(pg_ls[i_p]) )
		pageAdd(pg_ls[i_p],parent().at().pageAt(pg_ls[i_p]).at().path());
	//- Enable included pages -
	pageList(pg_ls);
	for(int i_l = 0; i_l < pg_ls.size(); i_l++ )
	    try{ pageAt(pg_ls[i_l]).at().setEnable(true); }
	    catch( TError err )	{ mess_err( err.cat.c_str(), "%s", err.mess.c_str() ); }
    }
}

void SessPage::setProcess( bool val )
{
    if( !enable() ) return;

    //> Change process state for included pages
    vector<string> ls;
    pageList(ls);
    for(int i_l = 0; i_l < ls.size(); i_l++ )
        pageAt(ls[i_l]).at().setProcess(val);

    //> Change self process state
    bool diff = (val!=process());
    if( val && !parent().at().parent().freeStat() && ( attrAt("pgOpen").at().getB() || attrAt("pgNoOpenProc").at().getB() ) )
    {
	SessWdg::setProcess(true);
	//>> First calc
	if( diff ) calc(true,false);
    }
    else if( !val )
    {
	//>> Last calc
	if( diff ) calc(false,true);

	SessWdg::setProcess(false);
    }
}

AutoHD<Page> SessPage::parent( )
{
    return Widget::parent();
}

void SessPage::pageAdd( const string &iid, const string &iparent )
{
    if( pagePresent(iid) )return;
    chldAdd(mPage,new SessPage(iid,iparent,ownerSess()));
}

AutoHD<SessPage> SessPage::pageAt( const string &iid )
{
    return chldAt(mPage,iid);
}

void SessPage::calc( bool first, bool last )
{
    //- Process self data -
    if( process() ) SessWdg::calc(first,last);

    //- Put calculate to include pages -
    vector<string> ls;
    pageList(ls);
    for(int i_l = 0; i_l < ls.size(); i_l++ )
        pageAt(ls[i_l]).at().calc(first,last);
}

bool SessPage::attrChange( Attr &cfg, void *prev )
{
    //- Page open process -
    if( enable() && prev && cfg.id() == "pgOpen" )
    {
	if( cfg.getB() )
	{
	    if( !process() )	setProcess(true);
	    ownerSess()->openReg(path());
	}
	else
	{
	    ownerSess()->openUnreg(path());
	    if( process() && !attrAt("pgNoOpenProc").at().getB() )	setProcess(false);
	}
    }

    return SessWdg::attrChange( cfg, prev );
}

void SessPage::alarmSet( bool isSet )
{
    int aStCur  = attrAt("alarmSt").at().getI( );
    string aCur = attrAt("alarm").at().getS( );
    int alev = atoi(TSYS::strSepParse(aCur,0,'|').c_str()) & 0xFF;
    int atp  = atoi(TSYS::strSepParse(aCur,3,'|').c_str()) & 0xFF;
    int aqtp = (aStCur>>16) & 0xFF & atp;
    if( isSet ) aqtp |= atp;

    vector<string> lst;
    //- Included pages process -
    pageList( lst );
    for( int i_p = 0; i_p < lst.size(); i_p++ )
    {
	int iacur = pageAt( lst[i_p] ).at().attrAt("alarmSt").at().getI( );
	alev = vmax( alev, iacur&0xFF );
	atp |= (iacur>>8) & 0xFF;
	aqtp |= (iacur>>16) & 0xFF;
    }
    //- Included widgets process -
    wdgList( lst );
    for( int i_w = 0; i_w < lst.size(); i_w++ )
    {
	int iacur = wdgAt( lst[i_w] ).at().attrAt("alarmSt").at().getI( );
	alev = vmax( alev, iacur&0xFF );
	atp |= (iacur>>8) & 0xFF;
	aqtp |= (iacur>>16) & 0xFF;
    }

    attrAt("alarmSt").at().setI( (alev && atp) ? (aqtp<<16)|(atp<<8)|alev : 0 );

    if( ownerSessWdg(true) )	ownerSessWdg(true)->alarmSet();
    if( isSet )	ownerSess( )->alarmSet( path(), aCur );
}

void SessPage::alarmQuittance( ui8 quit_tmpl, bool isSet )
{
    int alarmSt = attrAt("alarmSt").at().getI();
    if( !((((alarmSt>>16)&0xFF)^quit_tmpl)&((alarmSt>>16)&0xFF)) ) return;

    //- Self quittance -
    attrAt("alarmSt").at().setI( alarmSt & (quit_tmpl<<16|0xFFFF) );

    vector<string> lst;
    //- Included pages quitance -
    pageList( lst );
    for( int i_p = 0; i_p < lst.size(); i_p++ )
	pageAt( lst[i_p] ).at().alarmQuittance(quit_tmpl);
    //- Include widgets quittance -
    wdgList( lst );
    for( int i_w = 0; i_w < lst.size(); i_w++ )
	wdgAt( lst[i_w] ).at().alarmQuittance(quit_tmpl);

    if( isSet && ownerSessWdg(true) )	ownerSessWdg(true)->alarmSet();
}

bool SessPage::cntrCmdGeneric( XMLNode *opt )
{
    //- Get page info -
    if( opt->name() == "info" )
    {
	SessWdg::cntrCmdGeneric(opt);
	ctrMkNode("oscada_cntr",opt,-1,"/",_("Session page: ")+ownerFullId()+"/"+id());
	if( enable() && !(parent().at().prjFlags( )&Page::Empty) )
	    ctrMkNode("fld",opt,1,"/wdg/st/open",_("Open"),RWRWR_,owner().c_str(),grp().c_str(),1,"tp","bool");
	if( enable() && parent().at().prjFlags()&(Page::Template|Page::Container) )
	{
	    if(ctrMkNode("area",opt,1,"/page",_("Pages")))
	        ctrMkNode("list",opt,-1,"/page/page",_("Pages"),R_R_R_,"root","UI",3,"tp","br","idm","1","br_pref","pg_");
	    if(ctrMkNode("branches",opt,-1,"/br","",R_R_R_))
	        ctrMkNode("grp",opt,-1,"/br/pg_",_("Page"),R_R_R_,"root","UI",1,"idm","1");
	}
        return true;
    }

    //> Process command to page
    string a_path = opt->attr("path");
    if( a_path == "/wdg/st/open" && enable() && !(parent().at().prjFlags( )&Page::Empty) )
    {
	if( ctrChkNode(opt,"get",RWRWR_,owner().c_str(),grp().c_str(),SEQ_RD) )
	    opt->setText(TSYS::int2str(attrAt("pgOpen").at().getB()));
	if( ctrChkNode(opt,"set",RWRWR_,owner().c_str(),grp().c_str(),SEQ_WR) )
	    attrAt("pgOpen").at().setB(atoi(opt->text().c_str()));
    }
    else if( (a_path == "/br/pg_" || a_path == "/page/page") && ctrChkNode(opt) )
    {
	vector<string> lst;
	pageList(lst);
	for( unsigned i_f=0; i_f < lst.size(); i_f++ )
	    opt->childAdd("el")->setAttr("id",lst[i_f])->setText(pageAt(lst[i_f]).at().name());
    }
    else return SessWdg::cntrCmdGeneric(opt);

    return true;
}


//************************************************
//* SessWdg: Session page's widget               *
//************************************************
SessWdg::SessWdg( const string &iid, const string &iparent, Session *isess ) : 
    Widget(iid,iparent), m_proc(false), TValFunc(iid+"_wdg",NULL), m_mdfClc(0), 
    mSess(isess), inLnkGet(true)
{

}

SessWdg::~SessWdg( )
{

}

void SessWdg::postEnable( int flag )
{
    Widget::postEnable(flag);

    if( flag&TCntrNode::NodeConnect )
    {
	attrAdd( new TFld("event",_("Events"),TFld::String,TFld::FullText) );
	attrAdd( new TFld("alarmSt",_("Alarm status"),TFld::Integer,TFld::HexDec,"5","0") );
	attrAdd( new TFld("alarm",_("Alarm"),TFld::String,TFld::NoFlag,"200") );
    }
}

SessWdg *SessWdg::ownerSessWdg( bool base )
{
    if( nodePrev(true) )
    {
	if( !base && dynamic_cast<SessPage*>(nodePrev()) )   return NULL;
	return dynamic_cast<SessWdg*>(nodePrev());
    }
    return NULL;
}

SessPage *SessWdg::ownerPage()
{
    if( nodePrev(true) && dynamic_cast<SessPage*>(nodePrev()) ) return (SessPage*)nodePrev();
    SessWdg *own = ownerSessWdg( );
    if( own )	return own->ownerPage( );
    return NULL;
}

string SessWdg::path( )
{
    return ownerFullId(true)+"/wdg_"+id();
}

string SessWdg::ownerFullId( bool contr )
{
    SessWdg *ownW = ownerSessWdg( );
    if( ownW )	return ownW->ownerFullId(contr)+(contr?"/wdg_":"/")+ownW->id();
    SessPage *ownP = ownerPage( );
    if( ownP )	return ownP->ownerFullId(contr)+(contr?"/pg_":"/")+ownP->id();
    return string(contr?"/ses_":"/")+ownerSess()->id();
}

void SessWdg::setEnable( bool val )
{
    Widget::setEnable(val);

    if( !val )
    {
	//- Delete included widgets -
	vector<string> ls;
	wdgList(ls);
	for(int i_l = 0; i_l < ls.size(); i_l++ )
	    wdgDel(ls[i_l]);
    }
}

void SessWdg::setProcess( bool val )
{
    if( val && !enable() ) setEnable(true);

    //- Prepare process function value level -
    if( val && !TSYS::strEmpty(calcProg()) )
    {
	//-- Prepare function io structure --
	TFunction fio(parent().at().calcId());
	//--- Add generic io ---
	fio.ioIns( new IO("f_frq",_("Function calculate frequency (Hz)"),IO::Real,IO::Default,"1000",false),0);
	fio.ioIns( new IO("f_start",_("Function start flag"),IO::Boolean,IO::Default,"0",false),1);
	fio.ioIns( new IO("f_stop",_("Function stop flag"),IO::Boolean,IO::Default,"0",false),2);
	//--- Add calc widget's attributes ---
	vector<string> iwls, als;
	//--- Self attributes check ---
	attrList(als);
	AutoHD<Widget> fulw = parentNoLink();
	for( int i_a = 0; i_a < als.size(); i_a++ )
	{
	    AutoHD<Attr> cattr = attrAt(als[i_a]);
	    if( (fulw.at().attrPresent(als[i_a])&&fulw.at().attrAt(als[i_a]).at().flgSelf()&Attr::ProcAttr) || als[i_a] == "focus" )
	    {
		IO::Type tp = IO::String;
		switch( cattr.at().type() )
		{
		    case TFld::Boolean: tp = IO::Boolean;	break;
		    case TFld::Integer: tp = IO::Integer;	break;
		    case TFld::Real:    tp = IO::Real;		break;
		    case TFld::String:  tp = IO::String;	break;
		}
		fio.ioAdd( new IO(als[i_a].c_str(),cattr.at().name().c_str(),tp,IO::Output,"",false,("./"+als[i_a]).c_str()) );
	    }
	}
	//--- Include attributes check ---
	wdgList(iwls);
	for( int i_w = 0; i_w < iwls.size(); i_w++ )
	{
	    AutoHD<Widget> curw = wdgAt(iwls[i_w]);
	    curw.at().attrList(als);
	    for( int i_a = 0; i_a < als.size(); i_a++ )
	    {
		AutoHD<Attr> cattr = curw.at().attrAt(als[i_a]);
		if( cattr.at().flgSelf()&Attr::ProcAttr || als[i_a] == "focus" )
		{
		    IO::Type tp = IO::String;
		    switch( cattr.at().type() )
		    {
			case TFld::Boolean: tp = IO::Boolean;	break;
			case TFld::Integer: tp = IO::Integer;	break;
			case TFld::Real:    tp = IO::Real;	break;
			case TFld::String:  tp = IO::String;	break;
		    }
		    fio.ioAdd( new IO((iwls[i_w]+"_"+als[i_a]).c_str(),(curw.at().name()+"."+cattr.at().name()).c_str(),tp,IO::Output,"",false,
					(iwls[i_w]+"/"+als[i_a]).c_str()) );
		}
	    }
	}
	fio.ioAdd( new IO("event",_("Event"),IO::String,IO::Output) );
	fio.ioAdd( new IO("alarmSt",_("Alarm status"),IO::Integer,IO::Output,"",false,"./alarmSt") );
	fio.ioAdd( new IO("alarm",_("Alarm"),IO::String,IO::Output,"",false,"./alarm") );

	//-- Compile function --
	try
	{
	    work_prog = SYS->daq().at().at(TSYS::strSepParse(calcLang(),0,'.')).at().
		compileFunc(TSYS::strSepParse(calcLang(),1,'.'),fio,calcProg());
	    //-- Connect to compiled function --
	    TValFunc::setFunc(&((AutoHD<TFunction>)SYS->nodeAt(work_prog,1)).at());
	}catch( TError err )
	{ mess_err(nodePath().c_str(),_("Compile function '%s' by language '%s' for widget error: %s"),fio.id().c_str(),calcLang().c_str(),err.mess.c_str()); }
    }
    if( !val )
    {
	//-- Free function link --
	m_proc = false;
	TValFunc::setFunc(NULL);
    }

    //- Change process for included widgets -
    vector<string> ls;
    wdgList(ls);
    for(int i_l = 0; i_l < ls.size(); i_l++ )
	wdgAt(ls[i_l]).at().setProcess(val);

    //-- Make process element's lists --
    if( val ) prcElListUpdate( );

    m_proc = val;
}

string SessWdg::ico( )
{
    if( !parent().freeStat() )  return parent().at().ico();
    return "";
}

string SessWdg::owner( )
{
    SessWdg *sWdg = ownerSessWdg(false);
    if( sWdg )	return sWdg->owner();
    if( !parent().freeStat() )  return parent().at().owner();
    return Widget::owner();
}

string SessWdg::grp( )
{
    SessWdg *sWdg = ownerSessWdg(false);
    if( sWdg )	return sWdg->grp();
    if( !parent().freeStat() )  return parent().at().grp();
    return Widget::grp();
}

short SessWdg::permit( )
{
    SessWdg *sWdg = ownerSessWdg(false);
    if( sWdg )	return sWdg->permit();
    if( !parent().freeStat() )	return parent().at().permit();
    return Widget::permit();
}

string SessWdg::calcLang( )
{
    if( !parent().freeStat() )    return parent().at().calcLang();
    return "";
}

string SessWdg::calcProg( )
{
    if( !parent().freeStat() )    return parent().at().calcProg();
    return "";
}

int SessWdg::calcPer( )
{
    if( !parent().freeStat() )    return parent().at().calcPer( );
    return 0;
}

string SessWdg::resourceGet( const string &id, string *mime )
{
    string mimeType, mimeData;

    mimeData = parent().at().resourceGet( id, &mimeType );
    if( mime )	*mime = mimeType;

    return mimeData;
}

void SessWdg::wdgAdd( const string &iid, const string &name, const string &iparent )
{
    if( !isContainer() )  throw TError(nodePath().c_str(),_("Widget is not container!"));
    if( wdgPresent(iid) ) return;

    chldAdd(inclWdg,new SessWdg(iid,iparent,ownerSess()));
}

AutoHD<SessWdg> SessWdg::wdgAt( const string &wdg )
{
    return Widget::wdgAt(wdg);
}

void SessWdg::eventAdd( const string &ev )
{
    if( !attrPresent("event") )	return;
    Res &res = ownerSess()->eventRes();
    res.resRequestW( );
    attrAt("event").at().setS(attrAt("event").at().getS()+ev);
    res.resReleaseW( );
}

string SessWdg::eventGet( bool clear )
{
    if( !attrPresent("event") )	return "";
    Res &res = ownerSess()->eventRes();

    res.resRequestW( );
    string rez = attrAt("event").at().getS();
    if( clear )	attrAt("event").at().setS("");
    res.resReleaseW( );

    return rez;
}

void SessWdg::alarmSet( bool isSet )
{
    int aStCur  = attrAt("alarmSt").at().getI( );
    string aCur = attrAt("alarm").at().getS( );
    int alev = atoi(TSYS::strSepParse(aCur,0,'|').c_str()) & 0xFF;
    int atp  = atoi(TSYS::strSepParse(aCur,3,'|').c_str()) & 0xFF;
    int aqtp = (aStCur>>16) & 0xFF;
    if( isSet )	aqtp |= atp;

    vector<string> lst;
    //- Included widgets process -
    wdgList( lst );
    for( int i_w = 0; i_w < lst.size(); i_w++ )
    {
	int iacur = wdgAt( lst[i_w] ).at().attrAt("alarmSt").at().getI( );
	alev = vmax( alev, iacur&0xFF );
	atp |= (iacur>>8) & 0xFF;
	aqtp |= (iacur>>16) & 0xFF;
    }

    attrAt("alarmSt").at().setI( (alev && atp) ? (aqtp<<16)|(atp<<8)|alev : 0 );

    if( ownerSessWdg(true) )	ownerSessWdg(true)->alarmSet();
    if( isSet )	ownerSess( )->alarmSet( path(), aCur );
}

void SessWdg::alarmQuittance( ui8 quit_tmpl, bool isSet )
{
    int alarmSt = attrAt("alarmSt").at().getI();
    if( !((((alarmSt>>16)&0xFF)^quit_tmpl)&((alarmSt>>16)&0xFF)) ) return;

    //- Self quittance -
    attrAt("alarmSt").at().setI( alarmSt & (quit_tmpl<<16|0xFFFF) );

    vector<string> lst;
    //- Include widgets quittance -
    wdgList( lst );
    for( int i_w = 0; i_w < lst.size(); i_w++ )
	wdgAt( lst[i_w] ).at().alarmQuittance(quit_tmpl);

    if( isSet && ownerSessWdg(true) )	ownerSessWdg(true)->alarmSet();
}

void SessWdg::prcElListUpdate( )
{
    vector<string> ls;
    wdgList(ls);
    m_wdgChldAct.clear();
    for( int i_l = 0; i_l < ls.size(); i_l++ )
	if( wdgAt(ls[i_l]).at().process() )
	    m_wdgChldAct.push_back(ls[i_l]);
    attrList(ls);
    m_attrLnkLs.clear();
    m_attrUILs.clear();
    for( int i_a = 0; i_a < ls.size(); i_a++ )
    {
	AutoHD<Attr> attr = attrAt(ls[i_a]);
	if( attr.at().flgSelf()&(Attr::CfgConst|Attr::CfgLnkIn|Attr::CfgLnkOut) && !attr.at().cfgVal().empty() )
	    m_attrLnkLs.push_back(ls[i_a]);
	if( !(attr.at().flgGlob()&Attr::IsUser) )	m_attrUILs.push_back(ls[i_a]);
	if( attr.at().flgSelf()&Attr::CfgLnkIn && attr.at().cfgVal().empty() )
	    attr.at().setS(EVAL_STR);
    }
}

void SessWdg::getUpdtWdg( const string &path, unsigned int tm, vector<string> &els )
{
    string wpath = path+"/"+id();
    if( m_mdfClc >= tm ) els.push_back(wpath);
    for( int i_ch = 0; i_ch < m_wdgChldAct.size(); i_ch++ )
	wdgAt(m_wdgChldAct[i_ch]).at().getUpdtWdg(wpath,tm,els);
}

unsigned int SessWdg::modifVal( Attr &cfg )
{
    int m_clc = ownerSess()->calcClk( );
    if( atoi(cfg.fld().reserve().c_str()) ) m_mdfClc = m_clc;
    return m_clc;
}

void SessWdg::calc( bool first, bool last )
{
    if( !process() )	return;

    ResAlloc res(mCalcRes,true);

    string sw_attr, s_attr, obj_tp;

    if( !(ownerSess()->calcClk()%100) ) prcElListUpdate( );

    //- Calculate include widgets -
    for(int i_l = 0; i_l < m_wdgChldAct.size(); i_l++ )
	if( wdgPresent(m_wdgChldAct[i_l]) )
	    wdgAt(m_wdgChldAct[i_l]).at().calc(first,last);

    try
    {
	//- Load events to process -
	if( !((ownerSess()->calcClk())%(vmax(calcPer()/ownerSess()->period(),1))) || first || last )
	{
	    string wevent = eventGet(true);
	    //- Process input links and constants -
	    AutoHD<Attr> attr, attr1;
	    AutoHD<TVal> vl;
	    inLnkGet = true;
	    for( int i_a = 0; i_a < m_attrLnkLs.size(); i_a++ )
	    {
		attr = attrAt(m_attrLnkLs[i_a]);
		if( attr.at().flgSelf()&Attr::CfgConst )	attr.at().setS(attr.at().cfgVal());
		else if( attr.at().flgSelf()&Attr::CfgLnkIn && !attr.at().cfgVal().empty() )
		{
		    obj_tp = TSYS::strSepParse(attr.at().cfgVal(),0,':')+":";
		    if( obj_tp == "val:" )	attr.at().setS(attr.at().cfgVal().substr(obj_tp.size()));
		    else if( obj_tp == "prm:" )
		    {
			try{ vl = SYS->daq().at().nodeAt(attr.at().cfgVal(),0,0,obj_tp.size()); }
			catch(TError err) { attr.at().setS(EVAL_STR); continue; }

			if( attr.at().flgGlob()&Attr::Address )	
			    attr.at().setS("/DAQ"+attr.at().cfgVal().substr(obj_tp.size()));
			else switch( attr.at().type() )
			{
			    case TFld::Boolean:	attr.at().setB(vl.at().getB());	break;
			    case TFld::Integer: attr.at().setI(vl.at().getI());	break;
			    case TFld::Real:	attr.at().setR(vl.at().getR());	break;
			    case TFld::String:	attr.at().setS(vl.at().getS());	break;
			}
		    }
		    else if( obj_tp == "addr:" )
			attr.at().setS(attr.at().cfgVal().substr(obj_tp.size()));
		    else if( obj_tp == "wdg:" )
		    {
			try{ attr1 = mod->nodeAt(attr.at().cfgVal(),0,0,obj_tp.size()); }
			catch(TError err) { attr.at().setS(EVAL_STR); continue; }
			
			switch( attr.at().type() )
			{
			    case TFld::Boolean:	attr.at().setB(attr1.at().getB()); break;
			    case TFld::Integer:	attr.at().setI(attr1.at().getI()); break;
			    case TFld::Real:	attr.at().setR(attr1.at().getR()); break;
			    case TFld::String:	attr.at().setS(attr1.at().getS()); break;
			}
		    }
		}
		else if( attr.at().flgSelf()&Attr::CfgLnkIn )	attr.at().setS(EVAL_STR);
	    }
	    inLnkGet = false;

	    if( TValFunc::func() )
	    {
		//- Load events to calc procedure -
		int evId = ioId("event");
		if( evId >= 0 )	setS(evId,wevent);

		//-- Load data to calc area --
		setR(0,1000./ownerSess()->period());
		setB(1,first);
		setB(2,last);
		for( int i_io = 3; i_io < ioSize( ); i_io++ )
		{
		    if( func()->io(i_io)->rez().empty() ) continue;
		    sw_attr = TSYS::pathLev(func()->io(i_io)->rez(),0);
		    s_attr  = TSYS::pathLev(func()->io(i_io)->rez(),1);
		    attr = (sw_attr==".")?attrAt(s_attr):wdgAt(sw_attr).at().attrAt(s_attr);
		    switch(ioType(i_io))
		    {
			case IO::String:	setS(i_io,attr.at().getS());	break;
			case IO::Integer:	setI(i_io,attr.at().getI());	break;
			case IO::Real:	setR(i_io,attr.at().getR());	break;
			case IO::Boolean:	setB(i_io,attr.at().getB());	break;
		    }
		}
		//-- Calc --
		TValFunc::calc();
		//-- Load data from calc area --
		for( int i_io = 3; i_io < ioSize( ); i_io++ )
		{
		    if( func()->io(i_io)->rez().empty() ) continue;
		    sw_attr = TSYS::pathLev(func()->io(i_io)->rez(),0);
		    s_attr  = TSYS::pathLev(func()->io(i_io)->rez(),1);
		    attr = (sw_attr==".")?attrAt(s_attr):wdgAt(sw_attr).at().attrAt(s_attr);
		    switch(ioType(i_io))
		    {
			case IO::String:	attr.at().setS(getS(i_io));	break;
			case IO::Integer:	attr.at().setI(getI(i_io));	break;
			case IO::Real:		attr.at().setR(getR(i_io));	break;
			case IO::Boolean:	attr.at().setB(getB(i_io));	break;
		    }
		}
		//-- Save events from calc procedure --
		if( evId >= 0 ) wevent = getS(evId);
	    }

	    //-- Process widget's events --
	    if( !wevent.empty() )
	    {
		int t_off;
		string sevup, sev, sev_ev, sev_path, sprc_lst, sprc, sprc_ev, sprc_path;
		for( int el_off = 0; (sev=TSYS::strSepParse(wevent,0,'\n',&el_off)).size(); )
		{
		    //-- Check for process events --
		    t_off = 0;
		    sev_ev   = TSYS::strSepParse(sev,0,':',&t_off);
		    sev_path = TSYS::strSepParse(sev,0,':',&t_off);
		    sprc_lst = attrAt("evProc").at().getS();
		    bool evProc = false;
		    for( int elp_off = 0; (sprc=TSYS::strSepParse(sprc_lst,0,'\n',&elp_off)).size(); )
		    {
			t_off = 0;
			sprc_ev   = TSYS::strSepParse(sprc,0,':',&t_off);
			sprc_path = TSYS::strSepParse(sprc,0,':',&t_off);
			if( sprc_ev == sev_ev && (sprc_path == "*" || sprc_path == sev_path) )
			{
			    sprc_path = TSYS::strSepParse(sprc,0,':',&t_off);
			    ownerSess()->uiComm(sprc_path,TSYS::strSepParse(sprc,0,':',&t_off),
				sev_path.empty() ? this : &((AutoHD<SessWdg>)nodeAt(sev_path)).at());
			    evProc = true;
			}
		    }
		    if( !evProc ) sevup+=sev_ev+":/"+id()+sev_path+"\n";
		}
		//-- Put left events to parent widget --
		SessWdg *owner = ownerSessWdg(true);
		if( owner && !sevup.empty() ) owner->eventAdd(sevup);
	    }
	}
    }
    catch(TError err)
    {
	res.release();
	mess_err(err.cat.c_str(),err.mess.c_str());
	mess_err(nodePath().c_str(),_("Widget calculation error. Process is disabled."));
	if( !last )	setProcess(false);
    }
}

bool SessWdg::attrChange( Attr &cfg, void *prev )
{
    Widget::attrChange( cfg, prev );

    //- Special session atributes process -
    //-- Focus attribute process for active active --
    if( cfg.id() == "active" )
    {
	if( cfg.getB() )
	    cfg.owner()->attrAdd( new TFld("focus",_("Focus"),TFld::Boolean,TFld::NoFlag,"1","false","","","-2") );
	else	cfg.owner()->attrDel("focus");
    }
    //- Alarm event for widget process -
    else if( cfg.id() == "alarm" && enable() && prev )		alarmSet( true );
    //- Alarm status process -
    else if( cfg.id() == "alarmSt" && cfg.getI()&0x10000 )	ownerSess( )->alarmQuittance( path(), (cfg.getI()>>16)&0xFF );

    //- External link process -
    if( !inLnkGet && prev && cfg.flgSelf()&Attr::CfgLnkOut && !cfg.cfgVal().empty() )
    {
	string obj_tp = TSYS::strSepParse(cfg.cfgVal(),0,':')+":";
	try
	{
	    if( obj_tp == "prm:" )
	        switch( cfg.type() )
	        {
		    case TFld::Boolean:
		        ((AutoHD<TVal>)SYS->daq().at().nodeAt(cfg.cfgVal(),0,0,obj_tp.size())).at().setB(cfg.getB());
		        break;
		    case TFld::Integer:
		        ((AutoHD<TVal>)SYS->daq().at().nodeAt(cfg.cfgVal(),0,0,obj_tp.size())).at().setI(cfg.getI());
		        break;
		    case TFld::Real:
		        ((AutoHD<TVal>)SYS->daq().at().nodeAt(cfg.cfgVal(),0,0,obj_tp.size())).at().setR(cfg.getR());
		        break;
		    case TFld::String:
		        ((AutoHD<TVal>)SYS->daq().at().nodeAt(cfg.cfgVal(),0,0,obj_tp.size())).at().setS(cfg.getS());
		        break;
		}
	    else if( obj_tp == "wdg:" )
		switch( cfg.type() )
		{
		    case TFld::Boolean:
		        ((AutoHD<Attr>)mod->nodeAt(cfg.cfgVal(),0,0,obj_tp.size())).at().setB(cfg.getB());
		        break;
		    case TFld::Integer:
		        ((AutoHD<Attr>)mod->nodeAt(cfg.cfgVal(),0,0,obj_tp.size())).at().setI(cfg.getI());
		        break;
		    case TFld::Real:
		        ((AutoHD<Attr>)mod->nodeAt(cfg.cfgVal(),0,0,obj_tp.size())).at().setR(cfg.getR());
		        break;
		    case TFld::String:
		        ((AutoHD<Attr>)mod->nodeAt(cfg.cfgVal(),0,0,obj_tp.size())).at().setS(cfg.getS());
		        break;
		}
	}catch(...)	{ }
    }
    return true;
}

bool SessWdg::cntrCmdServ( XMLNode *opt )
{
    string a_path = opt->attr("path");
    if( a_path == "/serv/attr" )		//Attribute's value operations
    {
	if( ctrChkNode(opt,"get",R_R_R_,"root","UI",SEQ_RD) )	//Get values
	{
	    unsigned  tm = strtoul(opt->attr("tm").c_str(),0,10);
	    if( !tm )
	    {
		opt->childAdd("el")->setAttr("id","root")->setAttr("p","-4")->setText(rootId());
		opt->childAdd("el")->setAttr("id","perm")->setAttr("p","-3")->
		    setText(TSYS::int2str(ownerSess()->sec.at().access(opt->attr("user"),SEQ_RD|SEQ_WR,owner(),grp(),permit())) );
	    }
	    if( !tm || m_mdfClc >= tm )
	    {
		AutoHD<Attr> attr;
		for( int i_l = 0; i_l < m_attrUILs.size(); i_l++ )
		{
		    attr = attrAt(m_attrUILs[i_l]);
		    if( attr.at().modif() >= tm && atoi(attr.at().fld().reserve().c_str()) )
			opt->childAdd("el")->setAttr("id",m_attrUILs[i_l].c_str())->
					     setAttr("p",attr.at().fld().reserve())->
					     setText(attr.at().getS());
		}
	    }
	}
	else if( ctrChkNode(opt,"set",permit(),owner().c_str(),grp().c_str(),SEQ_WR) )	//Set values
	    for( int i_ch = 0; i_ch < opt->childSize(); i_ch++ )
	    {
	        string aid = opt->childGet(i_ch)->attr("id");
	        if( aid == "event" ) eventAdd(opt->childGet(i_ch)->text()+"\n");
	        else attrAt(aid).at().setS(opt->childGet(i_ch)->text());
	    }
    }
    else if( a_path == "/serv/attrBr" && ctrChkNode(opt,"get",R_R_R_,"root","UI",SEQ_RD) )	//Get attributes all updated elements' of the branch
    {
	unsigned tm = strtoul(opt->attr("tm").c_str(),NULL,10);
	int perm = ownerSess()->sec.at().access(opt->attr("user"),(tm?SEQ_RD:SEQ_RD|SEQ_WR),owner(),grp(),permit());

	//>> Self attributes put
	if( !tm || m_mdfClc >= tm )
	{
	    if( !tm )
	    {
		opt->childAdd("el")->setAttr("id","root")->setAttr("p","-4")->setText(rootId());
		opt->childAdd("el")->setAttr("id","perm")->setAttr("p","-3")->setText(TSYS::int2str(perm));
	    }
	    AutoHD<Attr> attr;
	    for( int i_l = 0; i_l < m_attrUILs.size(); i_l++ )
	    {
		attr = attrAt(m_attrUILs[i_l]);
		if( attr.at().modif() >= tm && atoi(attr.at().fld().reserve().c_str()) )
		    opt->childAdd("el")->setAttr("id",m_attrUILs[i_l].c_str())->
				     setAttr("p",attr.at().fld().reserve())->
				     setText(attr.at().getS());
	    }
	}

	//>> Child widgets process
	if( enable() && perm&SEQ_RD )
	{
	    vector<string>	lst;
	    wdgList(lst);

	    for( unsigned i_f=0; i_f < lst.size(); i_f++ )
	    {
		AutoHD<SessWdg> iwdg = wdgAt(lst[i_f]);
		XMLNode *wn = new XMLNode("get");
		wn->setAttr("path",a_path)->setAttr("user",opt->attr("user"))->setAttr("tm",opt->attr("tm"));
		iwdg.at().cntrCmdServ(wn);
		if( wn->childSize() )
		{
		    wn->setName("w")->attrDel("path")->attrDel("user")->attrDel("rez")->attrDel("tm")->setAttr("id",lst[i_f]);
		    opt->childAdd(wn);
		}
		else delete wn;
	    }
	}
    }
    else return Widget::cntrCmdServ(opt);

    return true;
}

bool SessWdg::cntrCmdGeneric( XMLNode *opt )
{
    //- Get page info -
    if( opt->name() == "info" )
    {
	Widget::cntrCmdGeneric(opt);
	ctrMkNode("fld",opt,1,"/wdg/st/proc",_("Process"),RWRWR_,owner().c_str(),grp().c_str(),1,"tp","bool");
	return true;
    }

    //- Process command to page -
    string a_path = opt->attr("path");
    if( a_path == "/wdg/st/proc" )
    {
	if( ctrChkNode(opt,"get",RWRWR_,owner().c_str(),grp().c_str(),SEQ_RD) ) opt->setText(TSYS::int2str(process()));
	if( ctrChkNode(opt,"set",RWRWR_,owner().c_str(),grp().c_str(),SEQ_WR) ) setProcess(atoi(opt->text().c_str()));
    }
    else return Widget::cntrCmdGeneric(opt);

    return true;
}

bool SessWdg::cntrCmdAttributes( XMLNode *opt )
{
    //- Get page info -
    if( opt->name() == "info" )
    {
	Widget::cntrCmdAttributes(opt);
	if( ctrMkNode("area",opt,-1,"/attr",_("Attributes")) )
	{
	    //-- Properties form create --
	    vector<string> list_a;
	    attrList(list_a);
	    for( unsigned i_el = 0; i_el < list_a.size(); i_el++ )
	    {
		XMLNode *el = attrAt(list_a[i_el]).at().fld().cntrCmdMake(opt,"/attr",-1,owner().c_str(),grp().c_str(),permit()|R_R_R_);
		if( el ) el->setAttr("wdgFlg",TSYS::int2str(attrAt(list_a[i_el]).at().flgGlob()));
	    }
	}
	return true;
    }

    //- Process command to page -
    string a_path = opt->attr("path");
    if( a_path.substr(0,6) == "/attr/" )
    {
	//unsigned tm = ownerSess()->calcClk( );//time(NULL);
	AutoHD<Attr> attr = attrAt(TSYS::pathLev(a_path,1));
	if( ctrChkNode(opt,"get",((attr.at().fld().flg()&TFld::NoWrite)?(permit()&~0222):permit())|R_R_R_,owner().c_str(),grp().c_str(),SEQ_RD) )
	{
	    if( attr.at().fld().flg()&TFld::Selected )  opt->setText(attr.at().getSEL());
	    else                                        opt->setText(attr.at().getS());
	}
	if( ctrChkNode(opt,"set",((attr.at().fld().flg()&TFld::NoWrite)?(permit()&~0222):permit())|R_R_R_,owner().c_str(),grp().c_str(),SEQ_WR) )
	{
	    if( attr.at().id() == "event" )	eventAdd(opt->text()+"\n");
	    else if( attr.at().fld().flg()&TFld::Selected )
						attr.at().setSEL(opt->text());
	    else				attr.at().setS(opt->text());
	}
    }
    else return Widget::cntrCmdAttributes(opt);

    return true;
}

void SessWdg::cntrCmdProc( XMLNode *opt )
{
    if( cntrCmdServ(opt) ) return;

    //- Get page info -
    if( opt->name() == "info" )
    {
	cntrCmdGeneric(opt);
	cntrCmdAttributes(opt);
	return;
    }
    if( !(cntrCmdGeneric(opt) || cntrCmdAttributes(opt)) )
	TCntrNode::cntrCmdProc(opt);
}
