/***************************************************************************
                          winctrlnetcdf.cpp  -  description
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

#include "winctrlnetcdf.h"
#include "winvisview.h"
#include "winvismain.h"
#include "winsliceview.h"
#include <FL/Fl_File_Chooser.H>

WinSliceMain * CreateSliceViewWindow(Fl_Group *wPrev, int axis)
{
    int flw = wPrev->w();
    int flh = wPrev->h();
    int flx = wPrev->x();
    int fly = wPrev->y();

/*
    int flw = mainw - 8;
    int flh = mainh - 30; //*3 / 2;
    int flx = mainx + 3;
    int fly = mainh + 30;
*/    
    string buf;
/*
    flw -= 8;
    flh -= 60; //*3 / 2;
    flx += 3;
    fly += 50;
*/
    flw -= 8;
    flh -= 10; //*3 / 2;
    flx += 3;
    fly += 3;

    
    switch(axis){
        case SLICE_X_AXIS : buf = "slice - x axis"; 
			    /*
			    flw -= 8;
			    flh = flh / 2 - 10; //*3 / 2;
			    flx += 3;
			    fly += 10;
			    */
			    //sprintf(buf, "%s", "slice - x axis"); 
                            break;
        case SLICE_Y_AXIS : buf = "slice - y axis"; 
			    /*
			    flw -= 8;
			    flh = flh / 2 - 10;
			    flx += 3;
			    fly += flh * 2 / 3  + 110;
			    */
			    //sprintf(buf, "%s", "slice - y axis"); 
                            break;
        case SLICE_Z_AXIS : buf = "slice - z axis"; 
			    //sprintf(buf, "%s", "slice - z axis"); 
                            break;
    }
    
    WinSliceMain * pSliceMain = new WinSliceMain(flx,fly,flw,flh, (char *)buf.c_str()); 

    return pSliceMain;
}

WinCtrlNetCDF::WinCtrlNetCDF(int x, int y, int w, int h, cxVolume * pVolume){
	GenWindow(x,y,w,h,pVolume);
}

WinCtrlNetCDF::~WinCtrlNetCDF(){
}

void WinCtrlNetCDF::show(int argc, char **argv){    
    m_wMainWin->show(argc,argv);
}

/*
void _open(Fl_Widget *f, void *)
{
  FILE *fptr = NULL;  // (new FILE) is wrong
  char buf[1024];     // allocate memory on stack for tmp buffers
  char buf2[1024];
  fb = new Fl_File_Chooser("HTML", NULL, 0, NULL);
  fb->show();
  while(fb->visible())
   Fl::wait();
  fptr = fopen(fb->value(), "rb");
  while( fgets(buf, sizeof(buf), fptr) ) // read up to sizeof(buf)-1 bytes
  {
  // use snprintf instead of sprintf
  snprintf(buf2, sizeof(buf2), "%s\n%s", stktextbuffer->text(),buf);
  stktextbuffer->text(buf);
  }
  fclose(fptr);
}
*/

void WinCtrlNetCDF::GenWindow(int x, int y, int w, int h, cxVolume * pVolume)
{
	m_wMainWin = new Fl_Window(x, y,w,h, "VIDI-Climate");
	m_wMainWin->user_data((void*)(this));
	m_pVolume = pVolume;
    
	int left  = 0;
	int top = 30;
	int width = w ;
	int height = h - 10;

	Fl_Window* volrendwin = new Fl_Window(1,1,width - 1, (height-20)/2 - 15, "");
  	volrendwin->user_data((void*)(this));
	m_gGroupC = new Fl_Group(2, 2, width - 4, (height - 20)/2 - 25 , "");    
    	m_gGroupC->box(FL_ENGRAVED_BOX);
	m_gGroupC->align(FL_ALIGN_CENTER);
	//Fl_Group::current()->resizable(m_gGroupC);
	
	m_pVisMain = new WinVisMain(5, 6, width - 10/*768*/, (height-20)/2 - 30/*384*/);
	m_pVisMain->SetVolume(pVolume);
	m_gGroupC->resizable(m_gGroupC);
	m_gGroupC->end();
	//m_gGroupC->resizable(NULL);
	//volrendwin->resizable(volrendwin);
	volrendwin->resizable(volrendwin);
	volrendwin->end();

#ifdef WIN_SLICE
	

	//m_gGroupA = new Fl_Group(2, (height-20)/2, width - 4, (height)/2 + 18, "Slices"); 
	m_gGroupA = new Fl_Group(2, (height-20)/2, width - 4, (height)/2 + 18);
	m_gGroupA->box(FL_ENGRAVED_BOX);
	m_gGroupA->align(FL_ALIGN_TOP_LEFT);
	
	m_wSliceMain = NULL;
	m_wSliceMain = CreateSliceViewWindow(m_gGroupA, SLICE_X_AXIS);
	//wSliceMain->show(argc,argv);
	m_wSliceMain->SetVolume(pVolume, SLICE_Z_AXIS);
	//m_wSliceMain = CreateSliceViewWindow(m_gGroupA, SLICE_Y_AXIS);
	//wSliceMain->show(argc,argv);
	//m_wSliceMain->SetVolume(pVolume, SLICE_Y_AXIS);
	
	//m_wSliceMain = CreateSliceViewWindow(wVisMain, SLICE_Z_AXIS);
	////wSliceMain->show(argc,argv);
	//m_wSliceMain->SetVolume(pVolume, SLICE_Z_AXIS);
	m_gGroupA->resizable(m_gGroupA);
	m_gGroupA->end();
#endif

/*
	m_tTab = new Fl_Tabs(left, top-20, width, height-20);
	
	//View and Cutting
	m_gGroupA = new Fl_Group(left, top, width, height, "NetCFD file");    
	Fl_Group::current()->resizable(m_gGroupA);
		m_wCtrlNetCDFView = new WinCtrlNetCDFView(left+2, 40, width-10, height-60, pVolume); 
		m_gGroupA->end();
	
	
	
	//Color Map
	m_gGroupB = new Fl_Group(left, top, width, height, "Information Analysis");        
	
	
	m_tTab->end();
	
	m_wMainWin->end();
		//m_wMainWin->resizable(m_wMainWin);     
		//m_wMainWin->size_range(200,200);
*/
	m_wMainWin->resizable(m_wMainWin);
	m_wMainWin->end();
	//m_wMainWin->resizable(NULL);   
}

void WinCtrlNetCDF::DeActivate()
{
    m_wMainWin->deactivate();
    m_tTab->deactivate();
    m_gGroupA->deactivate();
    //m_gGroupB->deactivate();
    m_gGroupC->deactivate();    
}

void WinCtrlNetCDF::Activate()
{
    m_wMainWin->activate();
    m_tTab->activate();
    m_gGroupA->activate();
    //m_gGroupB->activate();
    m_gGroupC->activate();    
}

