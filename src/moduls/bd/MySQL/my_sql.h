
//OpenSCADA module BD.MySQL file: my_sql.h
/***************************************************************************
 *   Copyright (C) 2003-2022 by Roman Savochenko, <roman@oscada.org>       *
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

#ifndef MY_SQL_H
#define MY_SQL_H

#include <string>

#include <tmodule.h>
#include <tbds.h>

#undef _
#define _(mess) mod->I18N(mess).c_str()
#undef trS
#define trS(mess) mod->I18N(mess,mess_PreSave)

using std::string;
using namespace OSCADA;

namespace BDMySQL
{

//************************************************
//* MBDMySQL::Table				 *
//************************************************
class MBD;
class MTable : public TTable
{
    public:
	//Public methods
	MTable( string name, MBD *iown, vector<TStrIt> *tblStrct = NULL );
	~MTable( );

	void fieldStruct( TConfig &cfg );
	bool fieldSeek( int row, TConfig &cfg, const string &cacheKey = "" )
	{ return fieldSQLSeek(row, cfg, cacheKey); }
	void fieldGet( TConfig &cfg )	{ fieldSQLGet(cfg); }
	void fieldSet( TConfig &cfg )	{ fieldSQLSet(cfg); }
	void fieldDel( TConfig &cfg )	{ fieldSQLDel(cfg); }

	void fieldFix( TConfig &cfg, const string &langLs = "" );
	void fieldPrmSet( TCfg &cfg, const string &last, string &req );

	MBD &owner( ) const;

    private:
	//Private methods
	void postDisable( int flag );
	bool isEmpty( );

	string getSQLVal( TCfg &cfg, uint8_t RqFlg = 0 );
	void   setSQLVal( TCfg &cfg, const string &vl, bool tr = false );

	string UTCtoSQL( time_t val );
	time_t SQLtoUTC( const string &val );
};

//************************************************
//* BDMySQL::MBD				 *
//************************************************
class BDMod;
class MBD : public TBD
{
    friend class MTable;

    public:
	//Public methods
	MBD( string iid, TElem *cf_el );
	~MBD( );

	void enable( );
	void disable( );

	void allowList( vector<string> &list ) const;
	void sqlReq( const string &req, vector< vector<string> > *tbl = NULL, char intoTrans = EVAL_BOOL );

	void transOpen( );
	void transCommit( );
	void transCloseCheck( );

	void getStructDB( const string &nm, vector<TTable::TStrIt> &tblStrct );

    protected:
	//Protected methods
	void cntrCmdProc( XMLNode *opt );	//Control interface command process

    private:
	//Private methods
	void postDisable( int flag );
	TTable *openTable( const string &name, bool create );

	//Private attributes
	string	host, user, pass, bd, u_sock, cd_pg;
	int	port;

	int	reqCnt;
	int64_t	reqCntTm, trOpenTm;
	MYSQL	connect;
};

//************************************************
//* BDMySQL::BDMod                               *
//************************************************
class BDMod: public TTypeBD
{
    public:
	//Public methods
	BDMod( string name );
	~BDMod( );

	int lsPr( )	{ return 5; }
	string features( );

    private:
	//Private methods
	TBD *openBD( const string &iid );
};

extern BDMod *mod;

}

#endif // MY_SQL_H
