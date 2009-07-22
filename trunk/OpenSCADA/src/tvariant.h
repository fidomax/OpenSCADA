
//OpenSCADA system file: tvariant.h
/***************************************************************************
 *   Copyright (C) 2009 by Roman Savochenko                                *
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

#ifndef TVARIANT_H
#define TVARIANT_H

#include <string>

using std::string;

//Error values
#define EVAL_BOOL 2
#define EVAL_INT  -2147483647
#define EVAL_REAL -3.3E308
#define EVAL_STR  "<EVAL>"

//*************************************************
//* TVariant                                      *
//*************************************************
class TVarObj;

class TVariant
{
    public:
	//Data
	enum Type
	{
	    Null	= 0,
	    Boolean	= 1,
	    Integer	= 2,
	    Real	= 3,
	    String	= 4,
	    Object	= 5
	};

	//Methods
	TVariant( );
	TVariant( char ivl );
	TVariant( int ivl );
	TVariant( double ivl );
	TVariant( const string &ivl );
	TVariant( TVarObj *ivl );
	TVariant( const TVariant &var );

	~TVariant( );

	bool operator==( TVariant &vr );
	TVariant &operator=( const TVariant &vr );

	bool isNull( ) const	{ return (vl[0]==TVariant::Null); }
	Type type( ) const	{ return (Type)vl[0]; }
	void setType( Type tp );

	char	getB( ) const;
	int	getI( ) const;
	double	getR( ) const;
	string	getS( ) const;
	TVarObj	*getO( ) const;

	void setB( char val );
	void setI( int val );
	void setR( double val );
	void setS( const string &val );
	void setO( TVarObj *val );

    private:
	//Attributes
	string vl;
};

//***********************************************************
//* TVarObj                                                 *
//*   Variable object, by default included properties       *
//***********************************************************
class TVarObj
{
    public:
	//Methods
	TVarObj( );
	virtual ~TVarObj( );

	int connect( );
	int disconnect( );

	virtual TVariant propGet( const string &id );
	virtual void propSet( const string &id, TVariant val );

	virtual string getStrXML( const string &oid = "" );

	virtual TVariant funcCall( const string &id, vector<TVariant> &prms );

    protected:
	//Attributes
	map<string,TVariant> mProps;
	unsigned int mUseCnt;
};

//***********************************************************
//* TAreaObj                                                *
//*   Area object included indexed properties               *
//***********************************************************
class TAreaObj : public TVarObj
{
    public:
	//Methods
	TAreaObj( )	{ };

	int size( )	{ return mEls.size(); }

	TVariant propGet( const string &id );
	void propSet( const string &id, TVariant val );

	string getStrXML( const string &oid = "" );

	TVariant funcCall( const string &id, vector<TVariant> &prms );

    private:
	//Attributes
	vector<TVariant> mEls;

	//Methods
	static bool compareLess( const TVariant &v1, const TVariant &v2 );
};

#endif // TVARIANT_H
