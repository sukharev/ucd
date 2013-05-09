/***************************************************************************
                          winctrlmain.h  -  description
                             -------------------
    begin                : Sat Feb 8 2003
    copyright            : (C) 2003 by Hongfeng YU
    email                : yuho@cs.ucdavis.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WINCTRLMAIN_H
#define WINCTRLMAIN_H


/**
  *@author Hongfeng YU
  */
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <stdlib.h>
#include "wincolormap.h"
#include "winctrlview.h"
#include "winctrllight.h"
#include "winctrlca.h"
#include "winctrlnetcdfview.h"
#include "clusterview2d.h"

class WinCtrlMain {
public: 
    WinCtrlMain(int x = 0, int y =0, int w = 200, int h = 200, cxVolume * pVolume = NULL);
    ~WinCtrlMain();
    
    void GenWindow(int, int ,int ,int, cxVolume * pVolume);
    void show(int argc = 0, char ** argv = NULL);    
    void DeActivate();
    void Activate();
    
public:
    Fl_Window *m_wMainWin;
    
    Fl_Tabs * m_tTab;
    
    WinColorMap * m_wColorMap;
    WinCtrlView * m_wCtrlView;
    WinCtrlLight * m_wCtrlLight;
	WinCtrlCA * m_wCtrlCA;

	WinCtrlNetCDFView * m_wCtrlNetCDFView;
    ClusterView2D* cluster_view;

	Fl_Group * m_gGroup1;
    Fl_Group * m_gGroupA;
    Fl_Group * m_gGroupB;
	Fl_Group * m_gGroupB1;
    Fl_Group * m_gGroupC;
    Fl_Group * m_gGroupD;
	Fl_Group * m_gGroupE;
};

#endif
