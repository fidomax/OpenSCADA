
//OpenSCADA system module Protocol.UserProtocol file: user_prt.cpp
/***************************************************************************
 *   Copyright (C) 2010 by Roman Savochenko                                *
 *   rom_as@oscada.org, rom_as@fromru.com                                  *
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

#include <string.h>

#include <config.h>
#include <tsys.h>
#include <tmess.h>
#include <tmodule.h>
#include <tuis.h>

#include "user_prt.h"

//*************************************************
//* Modul info!                                   *
#define MOD_ID		"UserProtocol"
#define MOD_NAME	"User protocol"
#define MOD_TYPE	"Protocol"
#define VER_TYPE	VER_PROT
#define M_VERSION	"0.5.0"
#define AUTORS		"Roman Savochenko"
#define DESCRIPTION	"Allow creation self-user protocols on any OpenSCADA language."
#define LICENSE		"GPL"
//*************************************************

UserProtocol::TProt *UserProtocol::mod;

extern "C"
{
    TModule::SAt module( int n_mod )
    {
	if( n_mod==0 )	return TModule::SAt(MOD_ID,MOD_TYPE,VER_TYPE);
	return TModule::SAt("");
    }

    TModule *attach( const TModule::SAt &AtMod, const string &source )
    {
	if( AtMod == TModule::SAt(MOD_ID,MOD_TYPE,VER_TYPE) )
	    return new UserProtocol::TProt( source );
	return NULL;
    }
}

using namespace UserProtocol;

//*************************************************
//* TProt                                         *
//*************************************************
TProt::TProt( string name )
{
    mId		= MOD_ID;
    mType	= MOD_TYPE;
    mName	= MOD_NAME;
    mVers	= M_VERSION;
    mAutor	= AUTORS;
    mDescr	= DESCRIPTION;
    mLicense	= LICENSE;
    mSource	= name;

    mod		= this;

    mPrtU = grpAdd("up_");

    //> User protocol DB structure
    mUPrtEl.fldAdd( new TFld("ID",_("ID"),TFld::String,TCfg::Key|TFld::NoWrite,"20") );
    mUPrtEl.fldAdd( new TFld("NAME",_("Name"),TFld::String,TCfg::TransltText,"50") );
    mUPrtEl.fldAdd( new TFld("DESCR",_("Description"),TFld::String,TFld::FullText|TCfg::TransltText,"300") );
    mUPrtEl.fldAdd( new TFld("EN",_("To enable"),TFld::Boolean,0,"1","0") );
    mUPrtEl.fldAdd( new TFld("InPROG",_("Input program"),TFld::String,TFld::FullText|TCfg::TransltText,"10000") );
    mUPrtEl.fldAdd( new TFld("OutTR",_("Output transport"),TFld::String,0,"20","*") );
    mUPrtEl.fldAdd( new TFld("OutPROG",_("Output program"),TFld::String,TFld::FullText|TCfg::TransltText,"10000") );
}

TProt::~TProt()
{
    nodeDelAll();
}

void TProt::itemListIn( vector<string> &ls, const string &curIt )
{
    ls.clear();
    if( TSYS::strParse(curIt,1,".").empty() )	uPrtList(ls);
}

void TProt::uPrtAdd( const string &iid, const string &db )
{
    if( chldPresent(mPrtU,iid) ) return;
    chldAdd( mPrtU, new UserPrt(iid,db,&uPrtEl()) );
}

void TProt::load_( )
{
    //> Load DB
    //>> Search and create new user protocols
    try
    {
	TConfig g_cfg(&uPrtEl());
	g_cfg.cfgViewAll(false);
	vector<string> db_ls;

	//>>> Search into DB
	SYS->db().at().dbList(db_ls,true);
	for( int i_db = 0; i_db < db_ls.size(); i_db++ )
	    for( int fld_cnt=0; SYS->db().at().dataSeek(db_ls[i_db]+"."+modId()+"_uPrt","",fld_cnt++,g_cfg); )
	    {
		string id = g_cfg.cfg("ID").getS();
		if( !uPrtPresent(id) )	uPrtAdd(id,(db_ls[i_db]==SYS->workDB())?"*.*":db_ls[i_db]);
	    }

	//>>> Search into config file
	if( SYS->chkSelDB("<cfg>") )
	    for( int fld_cnt=0; SYS->db().at().dataSeek("",nodePath()+modId()+"_uPrt",fld_cnt++,g_cfg); )
	    {
		string id = g_cfg.cfg("ID").getS();
		if( !uPrtPresent(id) )	uPrtAdd(id,"*.*");
	    }
    }catch(TError err)
    {
	mess_err(err.cat.c_str(),"%s",err.mess.c_str());
	mess_err(nodePath().c_str(),_("Search and create new user protocol error."));
    }
}

void TProt::save_( )
{

}

void TProt::modStart( )
{
    vector<string> ls;
    uPrtList(ls);
    for( int i_n = 0; i_n < ls.size(); i_n++ )
	if( uPrtAt(ls[i_n]).at().toEnable( ) )
	    uPrtAt(ls[i_n]).at().setEnable(true);
}

void TProt::modStop( )
{
    vector<string> ls;
    uPrtList(ls);
    for( int i_n = 0; i_n < ls.size(); i_n++ )
	uPrtAt(ls[i_n]).at().setEnable(false);
}

TProtocolIn *TProt::in_open( const string &name )
{
    return new TProtIn(name);
}

void TProt::outMess( XMLNode &io, TTransportOut &tro )
{
    TValFunc funcV;

    //> Find user protocol for using
    vector<string> upLs;
    uPrtList(upLs);
    AutoHD<UserPrt> up, tup;
    for( int i_up = 0; i_up < upLs.size(); i_up++ )
    {
	tup = uPrtAt(upLs[i_up]);
	if( !tup.at().enableStat() || tup.at().workOutProg().empty() ) continue;
	if( tup.at().outTransport() == tro.workId() )	{ up = tup; break; }
	if( tup.at().outTransport() == "*" && up.freeStat() ) up = tup;
    }
    if( up.freeStat() ) return;
    funcV.setFunc(&((AutoHD<TFunction>)SYS->nodeAt(up.at().workOutProg(),1)).at());

    ResAlloc res( tro.nodeRes(), true );

    //> Load inputs
    funcV.setO(0,new XMLNodeObj());
    ((XMLNodeObj*)funcV.getO(0))->fromXMLNode(io);
    funcV.setO(1,new TCntrNodeObj(AutoHD<TCntrNode>(&tro),"root"));
    //> Call processing
    funcV.calc( );
    //> Get outputs
    ((XMLNodeObj*)funcV.getO(0))->toXMLNode(io);

    up.at().cntOutReq++;
}

void TProt::cntrCmdProc( XMLNode *opt )
{
    //> Get page info
    if( opt->name() == "info" )
    {
	TProtocol::cntrCmdProc(opt);
	ctrMkNode("grp",opt,-1,"/br/up_",_("User protocol"),0664,"root","root",2,"idm","1","idSz","20");
	if( ctrMkNode("area",opt,0,"/up",_("User protocols")) )
	    ctrMkNode("list",opt,-1,"/up/up",_("Protocols"),0664,"root","root",5,"tp","br","idm","1","s_com","add,del","br_pref","up_","idSz","20");
	return;
    }

    //> Process command to page
    string a_path = opt->attr("path");
    if( a_path == "/br/up_" || a_path == "/up/up" )
    {
	if( ctrChkNode(opt,"get",0664,"root","root",SEQ_RD) )
	{
	    vector<string> lst;
	    uPrtList(lst);
	    for( unsigned i_f=0; i_f < lst.size(); i_f++ )
		opt->childAdd("el")->setAttr("id",lst[i_f])->setText(uPrtAt(lst[i_f]).at().name());
	}
	if( ctrChkNode(opt,"add",0664,"root","root",SEQ_WR) )
	{
	    string vid = TSYS::strEncode(opt->attr("id"),TSYS::oscdID);
	    uPrtAdd(vid); uPrtAt(vid).at().setName(opt->text());
	}
	if( ctrChkNode(opt,"del",0664,"root","root",SEQ_WR) )	chldDel(mPrtU,opt->attr("id"),-1,1);
    }

    else TProtocol::cntrCmdProc(opt);
}

//*************************************************
//* TProtIn                                       *
//*************************************************
TProtIn::TProtIn( string name ) : TProtocolIn(name)
{

}

TProtIn::~TProtIn()
{

}

TProt &TProtIn::owner( )	{ return *(TProt*)nodePrev(); }

bool TProtIn::mess( const string &reqst, string &answer, const string &sender )
{
    try
    {
	//> Find user protocol for using
	if( !funcV.func() )
	{
	    string selNode = TSYS::strParse(SYS->transport().at().at(TSYS::strParse(srcTr(),0,".")).at().
				    inAt(TSYS::strParse(srcTr(),1,".")).at().protocolFull(),1,".");
	    if( !owner().uPrtPresent(selNode) ) return false;
	    up = owner().uPrtAt(selNode);
	    if( !up.at().enableStat() || up.at().workInProg().empty() ) return false;
	    funcV.setFunc(&((AutoHD<TFunction>)SYS->nodeAt(up.at().workInProg(),1)).at());
	}

	//> Load inputs
	funcV.setB(0,false);
	funcV.setS(1,funcV.getS(1)+reqst);
	funcV.setS(2,"");
	funcV.setS(3,sender);
	//> Call processing
	funcV.calc( );
	//> Get outputs
	bool rez = funcV.getB(0);
	if( !rez ) funcV.setS(1,"");
	answer = funcV.getS(2);

	up.at().cntInReq++;

	return rez;
    }
    catch(TError err) { mess_err(err.cat.c_str(),"%s",err.mess.c_str()); }

    return false;
}

//*************************************************
//* UserPrt                                       *
//*************************************************
UserPrt::UserPrt( const string &iid, const string &idb, TElem *el ) :
    TConfig(el), mDB(idb), mEn(false), cntInReq(0), cntOutReq(0),
    mId(cfg("ID").getSd()), mName(cfg("NAME").getSd()), mDscr(cfg("DESCR").getSd()), mAEn(cfg("EN").getBd())
{
    mId = iid;
}

UserPrt::~UserPrt( )
{
    try{ setEnable(false); } catch(...) { }
}

TCntrNode &UserPrt::operator=( TCntrNode &node )
{
    UserPrt *src_n = dynamic_cast<UserPrt*>(&node);
    if( !src_n ) return *this;

    if( enableStat( ) )	setEnable(false);

    //> Copy parameters
    string prevId = mId;
    *(TConfig*)this = *(TConfig*)src_n;
    mId = prevId;
    setDB(src_n->DB());

    return *this;
}

void UserPrt::postDisable( int flag )
{
    try
    {
	if( flag ) SYS->db().at().dataDel(fullDB(),owner().nodePath()+tbl(),*this,true);
    }catch(TError err)
    { mess_err(err.cat.c_str(),"%s",err.mess.c_str()); }
}

TProt &UserPrt::owner( )	{ return *(TProt*)nodePrev(); }

string UserPrt::name( )		{ return mName.size() ? mName : id(); }

string UserPrt::tbl( )		{ return owner().modId()+"_uPrt"; }

string UserPrt::outTransport( )	{ return cfg("OutTR").getS(); }

string UserPrt::inProgLang( )
{
    string mProg = cfg("InPROG").getS();
    return mProg.substr(0,mProg.find("\n"));
}

string UserPrt::inProg( )
{
    string mProg = cfg("InPROG").getS();
    int lngEnd = mProg.find("\n");
    return mProg.substr( (lngEnd==string::npos)?0:lngEnd+1 );
}

void UserPrt::setInProgLang( const string &ilng )
{
    cfg("InPROG").setS( ilng+"\n"+inProg() );
    modif();
}

void UserPrt::setInProg( const string &iprg )
{
    cfg("InPROG").setS( inProgLang()+"\n"+iprg );
    modif();
}

string UserPrt::outProgLang( )
{
    string mProg = cfg("OutPROG").getS();
    return mProg.substr(0,mProg.find("\n"));
}

string UserPrt::outProg( )
{
    string mProg = cfg("OutPROG").getS();
    int lngEnd = mProg.find("\n");
    return mProg.substr( (lngEnd==string::npos)?0:lngEnd+1 );
}

void UserPrt::setOutTransport( const string &it )	{ cfg("OutTR").setS(it); }

void UserPrt::setOutProgLang( const string &ilng )
{
    cfg("OutPROG").setS( ilng+"\n"+outProg() );
    modif();
}

void UserPrt::setOutProg( const string &iprg )
{
    cfg("OutPROG").setS( outProgLang()+"\n"+iprg );
    modif();
}

void UserPrt::load_( )
{
    if( !SYS->chkSelDB(DB()) ) return;
    cfgViewAll(true);
    SYS->db().at().dataGet(fullDB(),owner().nodePath()+tbl(),*this);
}

void UserPrt::save_( )
{
    SYS->db().at().dataSet(fullDB(),owner().nodePath()+tbl(),*this);
}

bool UserPrt::cfgChange( TCfg &cfg )
{
    modif( );

    return true;
}

void UserPrt::setEnable( bool vl )
{
    if( mEn == vl ) return;

    cntInReq = cntOutReq = 0;

    if( vl )
    {
	//> Prepare and compile input transport function
	if( !inProg().empty() )
	{
	    TFunction funcIO("uprt_"+id()+"_in");
	    funcIO.ioIns( new IO("rez",_("Result"),IO::Boolean,IO::Return),0);
	    funcIO.ioIns( new IO("request",_("Request"),IO::String,IO::Default),1);
	    funcIO.ioIns( new IO("answer",_("Answer"),IO::String,IO::Output),2);
	    funcIO.ioIns( new IO("sender",_("Sender"),IO::String,IO::Default),3);

	    mWorkInProg = SYS->daq().at().at(TSYS::strSepParse(inProgLang(),0,'.')).at().
		compileFunc(TSYS::strSepParse(inProgLang(),1,'.'),funcIO,inProg());
	} else mWorkInProg = "";
	//> Prepare and compile input transport function
	if( !outProg().empty() )
	{
	    TFunction funcIO("uprt_"+id()+"_out");
	    funcIO.ioIns( new IO("io",_("IO"),IO::Object,IO::Default),0);
	    funcIO.ioIns( new IO("tr",_("Transport"),IO::Object,IO::Default),1);

	    mWorkOutProg = SYS->daq().at().at(TSYS::strSepParse(outProgLang(),0,'.')).at().
		compileFunc(TSYS::strSepParse(outProgLang(),1,'.'),funcIO,outProg());
	} else mWorkOutProg = "";
    }

    mEn = vl;
}

string UserPrt::getStatus( )
{
    string rez = _("Disabled. ");
    if( enableStat( ) )
    {
	rez = _("Enabled. ");
	rez += TSYS::strMess( _("Requests input %.4g, output %.4g."), cntInReq, cntOutReq );
    }

    return rez;
}

void UserPrt::cntrCmdProc( XMLNode *opt )
{
    //> Get page info
    if( opt->name() == "info" )
    {
	TCntrNode::cntrCmdProc(opt);
	ctrMkNode("oscada_cntr",opt,-1,"/",_("User protocol: ")+name());
	if( ctrMkNode("area",opt,-1,"/up",_("User protocol")) )
	{
	    if( ctrMkNode("area",opt,-1,"/up/st",_("State")) )
	    {
		ctrMkNode("fld",opt,-1,"/up/st/status",_("Status"),R_R_R_,"root","root",1,"tp","str");
		ctrMkNode("fld",opt,-1,"/up/st/en_st",_("Enable"),RWRWR_,"root","root",1,"tp","bool");
		ctrMkNode("fld",opt,-1,"/up/st/db",_("DB"),RWRWR_,"root","root",4,"tp","str","dest","select","select","/db/list",
		    "help",_("DB address in format [<DB module>.<DB name>].\nFor use main work DB set '*.*'."));
	    }
	    if( ctrMkNode("area",opt,-1,"/up/cfg",_("Config")) )
	    {
		TConfig::cntrCmdMake(opt,"/up/cfg",0,"root","root",RWRWR_);
		ctrRemoveNode(opt,"/up/cfg/InPROG");
		ctrRemoveNode(opt,"/up/cfg/OutTR");
		ctrRemoveNode(opt,"/up/cfg/OutPROG");
	    }
	    if( ctrMkNode("area",opt,-1,"/in",_("Input")) )
	    {
		ctrMkNode("fld",opt,-1,"/in/PROGLang",_("Input program language"),RWRWR_,"root","root",3,"tp","str","dest","sel_ed","select","/up/cfg/plangIls");
		ctrMkNode("fld",opt,-1,"/in/PROG",_("Input program"),RWRWR_,"root","root",3,"tp","str","rows","10",
		    "help",_("Next attributes has defined for input requests processing:\n"
			    "   'rez' - processing result (false-full request;true-not full request);\n"
			    "   'request' - request message;\n"
			    "   'answer' - answer message;\n"
			    "   'sender' - request sender."));
	    }
	    if( ctrMkNode("area",opt,-1,"/out",_("Output")) )
	    {
		ctrMkNode("fld",opt,-1,"/out/TR",cfg("OutTR").fld().descr().c_str(),RWRWR_,"root","root",3,"tp","str","dest","sel_ed","select","/up/cfg/lsOtr");
		ctrMkNode("fld",opt,-1,"/out/PROGLang",_("Output program language"),RWRWR_,"root","root",3,"tp","str","dest","sel_ed","select","/up/cfg/plangOls");
		ctrMkNode("fld",opt,-1,"/out/PROG",_("Output program"),RWRWR_,"root","root",3,"tp","str","rows","10",
		    "help",_("Next attributes has defined for output requests processing:\n"
			    "   'io' - input/output interface's XMLNode object;\n"
			    "   'tr' - associated transport."));
	    }
	}
	return;
    }
    //> Process command to page
    string a_path = opt->attr("path");
    if( a_path == "/up/st/status" && ctrChkNode(opt) )	opt->setText(getStatus());
    else if( a_path == "/up/st/en_st" )
    {
	if( ctrChkNode(opt,"get",RWRWR_,"root","root",SEQ_RD) )	opt->setText(enableStat()?"1":"0");
	if( ctrChkNode(opt,"set",RWRWR_,"root","root",SEQ_WR) )	setEnable(atoi(opt->text().c_str()));
    }
    else if( a_path == "/up/st/db" )
    {
	if( ctrChkNode(opt,"get",RWRWR_,"root","root",SEQ_RD) )	opt->setText(DB());
	if( ctrChkNode(opt,"set",RWRWR_,"root","root",SEQ_WR) )	setDB(opt->text());
    }
    else if( a_path == "/up/cfg/lsOtr" && ctrChkNode(opt) )
    {
	opt->childAdd("el")->setText("*");
	vector<string> sls;
	SYS->transport().at().outTrList(sls);
	for( int i_s = 0; i_s < sls.size(); i_s++ )
	opt->childAdd("el")->setText(sls[i_s]);
    }
    else if( (a_path == "/up/cfg/plangIls" || a_path == "/up/cfg/plangOls") && ctrChkNode(opt) )
    {
	string tplng = (a_path=="/up/cfg/plangIls") ? inProgLang() : outProgLang();
	int c_lv = 0;
	string c_path = "", c_el;
	opt->childAdd("el")->setText(c_path);
	for( int c_off = 0; (c_el=TSYS::strSepParse(tplng,0,'.',&c_off)).size(); c_lv++ )
	{
	    c_path += c_lv ? "."+c_el : c_el;
	    opt->childAdd("el")->setText(c_path);
	}
	if(c_lv) c_path+=".";
	vector<string>  ls;
	switch(c_lv)
	{
	    case 0:
		SYS->daq().at().modList(ls);
		for( int i_l = 0; i_l < ls.size(); i_l++ )
		    if( !SYS->daq().at().at(ls[i_l]).at().compileFuncLangs() )
		    { ls.erase(ls.begin()+i_l); i_l--; }
		break;
	    case 1:
		if( SYS->daq().at().modPresent(TSYS::strSepParse(tplng,0,'.')) )
		    SYS->daq().at().at(TSYS::strSepParse(tplng,0,'.')).at().compileFuncLangs(&ls);
		break;
	}
	for(int i_l = 0; i_l < ls.size(); i_l++)
	    opt->childAdd("el")->setText(c_path+ls[i_l]);
    }
    else if( a_path.substr(0,7) == "/up/cfg" ) TConfig::cntrCmdProc(opt,TSYS::pathLev(a_path,2),"root","root",RWRWR_);
    else if( a_path == "/in/PROGLang" )
    {
	if( ctrChkNode(opt,"get",RWRWR_,"root","root",SEQ_RD) )	opt->setText(inProgLang());
	if( ctrChkNode(opt,"set",RWRWR_,"root","root",SEQ_WR) )	setInProgLang(opt->text());
    }
    else if( a_path == "/in/PROG" )
    {
	if( ctrChkNode(opt,"get",RWRWR_,"root","root",SEQ_RD) )	opt->setText(inProg());
	if( ctrChkNode(opt,"set",RWRWR_,"root","root",SEQ_WR) )	setInProg(opt->text());
    }
    else if( a_path == "/out/TR" )
    {
	if( ctrChkNode(opt,"get",RWRWR_,"root","root",SEQ_RD) )	opt->setText(outTransport());
	if( ctrChkNode(opt,"set",RWRWR_,"root","root",SEQ_WR) )	setOutTransport(opt->text());
    }
    else if( a_path == "/out/PROGLang" )
    {
	if( ctrChkNode(opt,"get",RWRWR_,"root","root",SEQ_RD) )	opt->setText(outProgLang());
	if( ctrChkNode(opt,"set",RWRWR_,"root","root",SEQ_WR) )	setOutProgLang(opt->text());
    }
    else if( a_path == "/out/PROG" )
    {
	if( ctrChkNode(opt,"get",RWRWR_,"root","root",SEQ_RD) )	opt->setText(outProg());
	if( ctrChkNode(opt,"set",RWRWR_,"root","root",SEQ_WR) )	setOutProg(opt->text());
    }
    else TCntrNode::cntrCmdProc(opt);
}
