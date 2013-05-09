/***************************************************************************
                          winctrlmain.cpp  -  description
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

#include "winctrlmain.h"
#include "winvisview.h"
#include "winctrlca.h"
#include "TransferFunc1D.h"
#include "OpacityDisp.h"
#include "Histogram1D.h"
#include "clusterview2d.h"

WinCtrlMain::WinCtrlMain(int x, int y, int w, int h, cxVolume * pVolume){
	GenWindow(x,y,w,h,pVolume);
}

WinCtrlMain::~WinCtrlMain(){
}

void WinCtrlMain::show(int argc, char **argv){    
    m_wMainWin->show(argc,argv);
}

void WinCtrlMain::GenWindow(int x, int y, int w, int h, cxVolume * pVolume)
{
	m_wMainWin = new Fl_Window(x, y,w,h, "Settings");
	m_wMainWin->user_data((void*)(this));    
    
    int left  = 1;
    int top = 30;
    int width = w - 2 * left;
    int height = h - 10;
    
    m_tTab = new Fl_Tabs(left, top - 20, width, height);
    
	//View and Cutting
    m_gGroup1 = new Fl_Group(left, top, width, height, "NetCDF");    
    Fl_Group::current()->resizable(m_gGroup1);
    m_wCtrlNetCDFView = new WinCtrlNetCDFView(2, 40, width, height, pVolume); 
    m_gGroup1->end();

    //View and Cutting
    m_gGroupA = new Fl_Group(left, top, width, height, "View");    
    Fl_Group::current()->resizable(m_gGroupA);
    m_wCtrlView = new WinCtrlView(2, 40, width, height, pVolume);    
    m_gGroupA->end();
    
	//Color Map
    /*
	m_gGroupB = new Fl_Group(left, top, width, height, "Old Color Map");       
    Fl_Group::current()->resizable(m_gGroupA);
    m_wColorMap = new WinColorMap(2, 40, width, height, pVolume);
    pVolume->m_pHistogram = m_wColorMap->m_wHistogram;           
    m_gGroupB->end();  
    */

    //Color Map
    
	m_gGroupB = new Fl_Group(left, top, width, height, "Color Map");   
	
    int vertOffset = int(2);
    int transFuncVerticalOffset = 4;
    int opacityDispVerticalOffset = 30;
   
    //ControlWin *ihateptrs = this;
    
	TransferFunc1D* trans1D = new TransferFunc1D(2, 40 + height/9 /*height/5+2*/, width, height/3, TF_SIZE, pVolume);                   
	m_wCtrlNetCDFView->setTfn(trans1D);
	float * colorTable = trans1D->getColorTablePtr();
    std::string filename = string("test") + "_" + string("test") + ".hist";
	
    Histogram1D* hist1D = new Histogram1D(2, trans1D->y()+trans1D->h() + 120/*height/5 + height/3 + 40*/, width, 200);
		//(width/3*2, height/2+10,width/2,60)



    OpacityDisp* opacityDisp = new OpacityDisp(2, 40, width,
                        height/9,trans1D->getResolution(),&colorTable);
   

    trans1D->setOpacityDisp(&opacityDisp);
    trans1D->setHistogram1D(&hist1D);
    hist1D->setColorTable(colorTable,trans1D->getResolution());
	//volume->setTransferFuncTex(&colorTable, (long) trans1D->getResolution());
    
	m_gGroupB->end(); 
	

    //Lighting
    m_gGroupC = new Fl_Group(left, top, width, height, "Effects");    
    Fl_Group::current()->resizable(m_gGroupC);    
    m_wCtrlLight = new WinCtrlLight(2, 40, width, height, pVolume);
    m_gGroupC->end();
    
	//Correlation Analysis
	m_gGroupD = new Fl_Group(left, top, width, height, "Correlation analysis");    
    Fl_Group::current()->resizable(m_gGroupD);    
    m_wCtrlCA = new WinCtrlCA(0, 40, width, height, pVolume);
    m_gGroupD->end();
  
#ifdef SHOW_CLUSTER
	//Temporal cluster viewer
	m_gGroupE = new Fl_Group(left, top, width, height, "Cluster viewer");    
    Fl_Group::current()->resizable(m_gGroupE);    
    cluster_view = new ClusterView2D(0, 40, width, height/2, TF_SIZE, pVolume/*&ihateptrs*/);
	cluster_view->mode(FL_ALPHA | FL_RGB | FL_DOUBLE | FL_DEPTH);
	//cluster_view->resizable(cluster_view);
	m_gGroupE->end();
#endif
    m_tTab->end();

	//m_wCtrlNetCDFView->SetPCoord(m_wCtrlCA->m_wParCrdDraw->getPCoord());
	m_wCtrlNetCDFView->SetPCoord(m_wCtrlCA->getPCoord());
	m_wMainWin->end();
	//m_wMainWin->resizable(m_wMainWin);     
	//m_wMainWin->size_range(200,200);
    m_wMainWin->resizable(m_wMainWin);   
}

void WinCtrlMain::DeActivate()
{
    m_wMainWin->deactivate();
    m_tTab->deactivate();
    m_gGroupA->deactivate();
    m_gGroupB->deactivate();
    m_gGroupC->deactivate();    
}

void WinCtrlMain::Activate()
{
    m_wMainWin->activate();
    m_tTab->activate();
    m_gGroupA->activate();
    m_gGroupB->activate();
    m_gGroupC->activate();    
}
