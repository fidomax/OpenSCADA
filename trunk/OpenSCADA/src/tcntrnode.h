
//OpenSCADA system file: tcntrnode.h
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

#ifndef TCNTRNODE_H
#define TCNTRNODE_H

#include <pthread.h>

#include <string>
#include <vector>

#include "xml.h"
#include "autohd.h"
#include "resalloc.h"

#ifndef OSC_HASHMAP
#include <map>
using std::map;
#else
#include <ext/hash_map>
using __gnu_cxx::hash_map;
//#include <unordered_map>
//using std::unordered_map;
#endif

//> Security standard permissions
#define R_R_R_  0444
#define R_R___  0440
#define R_____  0400
#define RWRWRW  0666
#define RWRWR_  0664
#define RWR_R_  0644
#define RWRW__  0660
#define RWR___  0640
#define RW____  0600

using std::string;
using std::vector;

namespace OSCADA
{

class TCntrNode;

#ifndef OSC_HASHMAP
/*struct StrPntLess
{
    bool operator()(const string *s1, const string *s2) const
    {
	return s1->compare(*s2) < 0;
    }
};*/

typedef map<string, TCntrNode*> TMap;
#else
namespace __gnu_cxx
{
template <> class hash<string>
{
    public:
	size_t operator()(const string& h) const
	{
	    return hash<const char*>()(h.c_str());
	}
};
}
typedef hash_map<string, TCntrNode*, __gnu_cxx::hash<string> > TMap;

//typedef unordered_map<string, TCntrNode* > TMap;

#endif

#define DEF_TIMEOUT 2

//***************************************************************
//* TCntrNode - Controll node					*
//***************************************************************
class TVariant;

//#pragma pack(push,1)
class TCntrNode
{
    //* Controll scenaries language section *
    public:
	//Methods
	TCntrNode( TCntrNode *prev = NULL );
	virtual ~TCntrNode( );

	virtual TCntrNode &operator=( TCntrNode &node );

	void cntrCmd( XMLNode *opt, int lev = 0, const string &path = "", int off = 0 );

	//> Static functions
	//>> Controll Field
	static XMLNode *ctrId( XMLNode *inf, const string &n_id, bool noex = false );      //get node for it full identifier
	static XMLNode *ctrMkNode( const char *n_nd, XMLNode *nd, int pos, const char *req, const string &dscr,
	    int perm=0777, const char *user="root", const char *grp="root", int n_attr=0, ... );
	static bool ctrRemoveNode( XMLNode *nd, const char *path );
	static bool ctrChkNode( XMLNode *nd, const char *cmd="get", int perm=0444, const char *user="root",
	    const char *grp="root", char mode=04, const char *warn = NULL );

    protected:
	//Methods
	virtual void cntrCmdProc( XMLNode *req );

    //* Resource section *
    public:
	//Data
	enum Flag
	{
	    //> Modes
	    MkDisable	= 0x00,		//Node make disable
	    Disable	= 0x01,		//Node disabled
	    MkEnable	= 0x02,		//Node make enable
	    Enable	= 0x03,		//Node enabled
	    //> Flags
	    SelfModify	= 0x04,		//Self modify
	};
	enum EnFlag
	{
	    NodeConnect	= 0x01,		//Connect node to control tree
	    NodeRestore	= 0x02,		//Restore node enabling after broken disabling.
	    NodeShiftDel= 0x04
	};
	enum ModifFlag	{ Self = 0x01, Child = 0x02, All = 0x03 };

	//Methods
	virtual Res &nodeRes( )		{ return hd_res; }
	virtual const string &nodeName( ) = 0;
	string nodePath( char sep = 0, bool from_root = false );

	void nodeList( vector<string> &list, const string& gid = "" );				//Full node list
	AutoHD<TCntrNode> nodeAt( const string &path, int lev = 0, char sep = 0, int off = 0 );	//Get node for full path
	void nodeDel( const string &path, char sep = 0, int flag = 0, bool shDel = false );	//Delete node at full path
	static void nodeCopy( const string &src, const string &dst, const string &user = "root" );

	TCntrNode *nodePrev( bool noex = false );
	char	 nodeFlg( )		{ return m_flg; }
	char	 nodeMode( )		{ return m_flg&0x3; }
	unsigned nodeUse( bool selfOnly = false );
	unsigned nodePos( )		{ return mOi; }

	//> Modify process methods
	int  isModify( int mflg = TCntrNode::All );		//Check for modify want
	void modif( )	{ m_flg |= SelfModify; }		//Set local modify
	void modifG( );						//Set group modify
	void modifClr( ){ m_flg &= ~(SelfModify); }		//Clear modify
	void modifGClr( );					//Modify group clear
	void load( bool force = false );			//Load node, if modified
	void save( );						//Save node, if modified

	//> Connections counter
	virtual void AHDConnect( );
	virtual void AHDDisConnect( );

	//> User object access
	virtual TVariant objPropGet( const string &id );
	virtual void objPropSet( const string &id, TVariant val );
	virtual TVariant objFuncCall( const string &id, vector<TVariant> &prms, const string &user );

    protected:
	//Data
	struct GrpEl
	{
	    string	id;
	    bool	ordered;
	    TMap	elem;
	};

	//Methods
	//> Commands
	void nodeEn( int flag = 0 );
	void nodeDis( long tm = 0, int flag = 0 );

	void nodeDelAll( );	//For hard link objects

	void setNodePrev( TCntrNode *node )	{ prev.node = node; }
	void setNodeMode( char mode );

	//> Childs
	virtual AutoHD<TCntrNode> chldAt( int8_t igr, const string &name, const string &user = "" );
	void chldList( int8_t igr, vector<string> &list );
	bool chldPresent( int8_t igr, const string &name );
	void chldAdd( int8_t igr, TCntrNode *node, int pos = -1, bool noExp = false );
	void chldDel( int8_t igr, const string &name, long tm = -1, int flag = 0, bool shDel = false );

	//> Conteiners
	int8_t	grpSize( );
	int8_t	grpId( const string &sid );
	GrpEl	&grpAt( int8_t id );
	unsigned grpAdd( const string &id, bool ordered = false );

	virtual void preEnable( int flag )	{ }
	virtual void postEnable( int flag )	{ }

	virtual void preDisable( int flag )	{ }
	virtual void postDisable( int flag )	{ }

	virtual void load_( )			{ }
	virtual void save_( )			{ }

    private:
	//Data
	struct
	{
	    TCntrNode	*node;
	    int8_t	grp;
	} prev;

	//Attributes
	//> Childs
	Res			hd_res;	//Resource HD
	static pthread_mutex_t	connM;	//Connection mutex

	vector<GrpEl>		*chGrp;	//Child groups

	//> Curent node
	unsigned short int	mUse;	//Use counter
	unsigned short int	mOi;	//Order index

	char	m_flg;
};
//#pragma pack(pop)

}

#endif //TCNTRNODE_H