
//OpenSCADA system module UI.Vision file: vis_devel_widgs.h
/***************************************************************************
 *   Copyright (C) 2004-2006 by Roman Savochenko                           *
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

#ifndef VIS_DEVEL_WIDGS_H
#define VIS_DEVEL_WIDGS_H

#include <QDockWidget>

class QTreeWidgetItem;
class QTreeWidget;

namespace VISION
{
 
//****************************************
//* Inspector of attributes              *
//**************************************** 
class InspAttr: public QDockWidget
{
    public:
	InspAttr( QWidget * parent = 0 );
	~InspAttr();
};
 
//****************************************
//* Inspector of links                   *
//**************************************** 
class InspLnk: public QDockWidget
{
    public:
	InspLnk( QWidget * parent = 0 );
	~InspLnk();
};
 
//****************************************
//* Widget's libraries tree              *
//****************************************
class VisDevelop;

class WdgTree: public QDockWidget
{
    Q_OBJECT

    public:
	WdgTree( VisDevelop *parent = 0 );
	~WdgTree();
	
	void updateLibs();	

	VisDevelop *owner();
	
    protected:
	bool eventFilter( QObject *target, QEvent *event );

    private slots:
	void ctrTreePopup( );
	void selectItem( );
	
    private:
	QTreeWidget *treeW;
};
 
//****************************************
//* Project's tree                       *
//**************************************** 
class ProjTree: public QDockWidget
{
    public:
	ProjTree( VisDevelop * parent = 0 );
	~ProjTree();
};
    
}

#endif //VIS_DEVEL_WIDGS
