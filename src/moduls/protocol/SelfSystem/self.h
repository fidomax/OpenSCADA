
//OpenSCADA system module Protocol.SelfSystem file: self.h
/***************************************************************************
 *   Copyright (C) 2003-2006 by Roman Savochenko                           *
 *   rom_as@fromru.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
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
 
#ifndef SELF_H
#define SELF_H

#include <tprotocols.h>

namespace SelfPr
{

//================================================================
//=========== TProtIn ============================================
//================================================================
class TProtIn: public TProtocolIn
{
    public:
	TProtIn( string name );
	~TProtIn();

	bool mess( const string &request, string &answer, const string &sender );
	
    private:
	bool m_nofull;
	string req_buf;
};

//================================================================
//=========== TProt ==============================================
//================================================================
class TProt: public TProtocol
{
    public:
	//Data
	class SAuth
	{
	    public:
		//Methods
		SAuth(time_t itm, string inm, int ises) :
		    t_auth(itm), name(inm), id_ses(ises) { }
		//Attributes
		time_t t_auth;
		string name;
		int    id_ses;
	};					    
    
	//Methods
	TProt( string name );
	~TProt();
	
	int authTime()	{ return m_t_auth; }
	
	void modLoad( );
	void modSave( );
	
	int ses_open(const char *user,const char *pass);
	void ses_close(int id_ses);
	SAuth ses_get(int id_ses);

    private:
	//Methods
	void cntrCmdProc( XMLNode *opt );
	string optDescr( );
	TProtocolIn *in_open( const string &name );
	
	//Attributes
	int		ses_res;
	vector<SAuth>	auth_lst;
	int		m_t_auth;
};

extern TProt *mod;
} //End namespace SelfPr
#endif //SELF_H

