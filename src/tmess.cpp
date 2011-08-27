
//OpenSCADA system file: tmess.cpp
/***************************************************************************
 *   Copyright (C) 2003-2010 by Roman Savochenko                           *
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

#include <sys/types.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <iconv.h>
#include <langinfo.h>
#include <getopt.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>
#include <string.h>
#include <errno.h>

#include "tsys.h"
#include "resalloc.h"
#include "tmess.h"

using namespace OSCADA;

//*************************************************
//* TMess                                         *
//*************************************************
TMess::TMess(  ) : IOCharSet("UTF-8"), mMessLevel(0), mLogDir(0x2), mConvCode(true), mIsUTF8(true)
{
    setenv("LC_NUMERIC","C",1);
    openlog(PACKAGE,0,LOG_USER);
    setlocale(LC_ALL,"");
    IOCharSet = nl_langinfo(CODESET);

    bindtextdomain(PACKAGE,LOCALEDIR);
    textdomain(PACKAGE);

    mLang2Code = lang();
    if( mLang2Code.size() < 2 || mLang2Code == "POSIX" || mLang2Code == "C" ) mLang2Code = "en";
    else mLang2Code = mLang2Code.substr(0,2);
    mIsUTF8 = (IOCharSet == "UTF-8" || IOCharSet == "UTF8" || IOCharSet == "utf8");

    if(mLang2Code == "en" && IOCharSet.compare(0,10,"ISO-8859-1")==0) mConvCode = false;
}

TMess::~TMess(  )
{
    closelog();
}

void TMess::setMessLevel( int level )
{
    mMessLevel = vmax(0,vmin(5,level));
    SYS->modif();
}

void TMess::setLogDirect( int dir )
{
    mLogDir = dir;
    SYS->modif();
}

void TMess::put( const char *categ, int8_t level, const char *fmt,  ... )
{
    char mess[STR_BUF_LEN];
    va_list argptr;

    va_start(argptr,fmt);
    vsnprintf(mess,sizeof(mess),fmt,argptr);
    va_end(argptr);

    level = vmin(Emerg,vmax(-Emerg,level));
    if( abs(level) < messLevel() ) return;

    int64_t ctm = TSYS::curTime();
    string s_mess = TSYS::int2str(level) + "|" + categ + " | " + mess;

    if( mLogDir&1 )
    {
	int level_sys;
	switch( abs(level) )
	{
	    case Debug:		level_sys = LOG_DEBUG;	break;
	    case Info:		level_sys = LOG_INFO;	break;
	    case Notice:	level_sys = LOG_NOTICE;	break;
	    case Warning:	level_sys = LOG_WARNING;break;
	    case Error:		level_sys = LOG_ERR;	break;
	    case Crit:		level_sys = LOG_CRIT;	break;
	    case Alert:		level_sys = LOG_ALERT;	break;
	    case Emerg:		level_sys = LOG_EMERG;	break;
	    default: 		level_sys = LOG_DEBUG;
	}
	syslog(level_sys,"%s",s_mess.c_str());
    }
    if( mLogDir&2 ) fprintf(stdout,"%s \n",s_mess.c_str());
    if( mLogDir&4 ) fprintf(stderr,"%s \n",s_mess.c_str());
    if( (mLogDir&8) && SYS->present("Archive") )
	SYS->archive().at().messPut( ctm/1000000, ctm%1000000, categ, level, mess );
}

void TMess::get( time_t b_tm, time_t e_tm, vector<TMess::SRec> &recs, const string &category, int8_t level )
{
    if( mLogDir&8 ) SYS->archive().at().messGet(b_tm,e_tm,recs,category,level);
}

string TMess::lang( )
{
    char *lng = NULL;
    if( ((lng=getenv("LANGUAGE")) && strlen(lng)) ||
	    ((lng=getenv("LC_MESSAGES")) && strlen(lng)) ||
	    ((lng=getenv("LANG")) && strlen(lng)) )
	return lng;
    else return "C";
}

void TMess::setLang( const string &lng )
{
    char *prvLng = NULL;
    if( (prvLng=getenv("LANGUAGE")) && strlen(prvLng) ) setenv( "LANGUAGE", lng.c_str(), 1 );
    else setenv( "LC_MESSAGES", lng.c_str(), 1 );
    setlocale(LC_ALL,"");

    IOCharSet = nl_langinfo(CODESET);

    mLang2Code = lang();
    if( mLang2Code.size() < 2 || mLang2Code == "POSIX" || mLang2Code == "C" ) mLang2Code = "en";
    else mLang2Code = mLang2Code.substr(0,2);
    mIsUTF8 = (IOCharSet == "UTF-8" || IOCharSet == "UTF8" || IOCharSet == "utf8");
}

string TMess::codeConv( const string &fromCH, const string &toCH, const string &mess )
{
    if(!mConvCode || fromCH == toCH) return mess;

    //> Make convert to blocks 1000 bytes
    string buf;
    buf.reserve(mess.size());
    char   *ibuf, outbuf[1000], *obuf;
    size_t ilen, olen;
    iconv_t hd;

    hd = iconv_open(toCH.c_str(), fromCH.c_str());
    if( hd == (iconv_t)(-1) )
    {
	mess_crit("IConv",_("Error iconv open: %s"),strerror(errno));
	return mess;
    }

    ibuf = (char *)mess.data();
    ilen = mess.size();

    while(ilen)
    {
	obuf = outbuf;
	olen = sizeof(outbuf)-1;
	size_t rez = iconv(hd,&ibuf,&ilen,&obuf,&olen);
	if( rez == (size_t)(-1) && errno != E2BIG )
	{
	    mess_crit("IConv",_("Error input sequence convert: %s"),strerror(errno));
	    mess_debug("IConv",_("Error converting from %s to %s for message part: '%s'"),
		fromCH.c_str(),toCH.c_str(),mess.substr(vmax((int)mess.size()-(int)ilen-10,0),20).c_str());
	    buf = mess;
	    break;
	}
	if( obuf > outbuf )
	    buf.append(outbuf,obuf-outbuf);
    }
    iconv_close(hd);

    return buf;
}

const char *TMess::I18N( const char *mess, const char *d_name )
{
    return dgettext(d_name, mess);
}

void TMess::setLang2CodeBase( const string &vl )
{
    mLang2CodeBase = vl;
    if((!mLang2CodeBase.empty() && mLang2CodeBase.size() < 2) || mLang2CodeBase == "POSIX" || mLang2CodeBase == "C")
	mLang2CodeBase = "en";
    else mLang2CodeBase = mLang2CodeBase.substr(0,2);

    SYS->modif();
}

void TMess::load()
{
    //- Load params from command line -
    int i,next_opt;
    const char *short_opt="h";
    struct option long_opt[] =
    {
	{"help"     ,0,NULL,'h'},
	{"MessLev"  ,1,NULL,'d'},
	{"log"      ,1,NULL,'l'},
	{NULL       ,0,NULL,0  }
    };

    optind=opterr=0;
    do
    {
	next_opt=getopt_long(SYS->argc,(char * const *)SYS->argv,short_opt,long_opt,NULL);
	switch(next_opt)
	{
	    case 'h': return;
	    case 'd': i = atoi(optarg); if(i>=0&&i<=7) setMessLevel(i); break;
	    case 'l': setLogDirect(atoi(optarg)); break;
	    case -1 : break;
	}
    } while(next_opt != -1);

    //> Load params config file
    setMessLevel(atoi(TBDS::genDBGet(SYS->nodePath()+"MessLev",TSYS::int2str(messLevel())).c_str()));
    setLogDirect(atoi(TBDS::genDBGet(SYS->nodePath()+"LogTarget",TSYS::int2str(logDirect())).c_str()));
    mLang2CodeBase = TBDS::genDBGet(SYS->nodePath()+"Lang2CodeBase",mLang2CodeBase);
}

void TMess::save()
{
    TBDS::genDBSet(SYS->nodePath()+"MessLev",TSYS::int2str(messLevel()));
    TBDS::genDBSet(SYS->nodePath()+"LogTarget",TSYS::int2str(logDirect()));
    TBDS::genDBSet(SYS->nodePath()+"Lang2CodeBase",mLang2CodeBase);
}

const char *TMess::labDB( )
{
    return _("DB address in format [<DB module>.<DB name>].\n"
	     "For use main work DB set '*.*'.");
}

const char *TMess::labSecCRON( )
{
    return _("Schedule is writed in seconds periodic form or in standard CRON form.\n"
             "Seconds form is one real number (1.5, 1e-3).\n"
             "Cron it is standard form \"* * * * *\".\n"
             "  Where items by order:\n"
             "    - minutes (0-59);\n"
             "    - hours (0-23);\n"
             "    - days (1-31);\n"
             "    - month (1-12);\n"
             "    - week day (0[sunday]-6).\n"
             "  Where an item variants:\n"
             "    - \"*\" - any value;\n"
             "    - \"1,2,3\" - allowed values;\n"
             "    - \"1-5\" - raw range of allowed values;\n"
             "    - \"*/2\" - range divider for allowed values.\n"
             "Examples:\n"
             "  \"1e-3\" - periodic call by one millisecond;\n"
             "  \"* * * * *\" - each minute;\n"
             "  \"10 23 * * *\" - only 23 hour and 10 minute for any day and month;\n"
             "  \"*/2 * * * *\" - for minutes: 0,2,4,...,56,58;\n"
             "  \"* 2-4 * * *\" - for any minutes in hours from 2 to 4(include).");
}

const char *TMess::labSecCRONsel( )
{
    return "1;1e-3;* * * * *;10 * * * *;10-20 2 */2 * *";
}