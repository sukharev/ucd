/***************************************************************************
                          winctrlca.cpp  -  description
                             -------------------
    begin                : Mon Jan 20 2003
    copyright            : (C) 2003 by Hongfeng YU
    email                : hfyu@ucdavis.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 * Copyright: All rights reserved.  May not be used, modified, or copied   *
 * without permission.                                                     *
 *                                                                         *
 ***************************************************************************/


#include "winctrlca.h"
#include "winvisview.h"
#include "cxvolume.h"
#include <iostream>
#include <fstream>

using namespace std;

#define TEXTSIZE 12



WinCtrlCA::WinCtrlCA(int x = 0, int y =0, int w = 200, int h = 200, cxVolume * pVolume = NULL)
{
	m_pVolume =  pVolume;
	GenWindow(x,y, w,h);
	m_h = h;
	m_w = w;
	m_x = x;
	m_y = y;
	
	reset();
}

WinCtrlCA::~WinCtrlCA(){    
}

void WinCtrlCA::AddUI(int n)
{
	/*
	for (int i = 0; i < n; i++)
	{
		int x = m_pcoord->XMap(i);
		m_pCheckA[i] = new Fl_Check_Button(x, m_h-50, 25, 20, "variable");
		m_pCheckA[i]->labelsize(TEXTSIZE);
	}
	*/
	reset();
}


#define TEXTSIZE 12
void WinCtrlCA::GenWindow(int x, int y, int w, int h)
{
	//m_wMainWin = new Fl_Window(x, y,w,h, "Control Center");
	//m_wMainWin = new Fl_Window(x, y+100,w,h-100, "Control Center");
	//m_wMainWin->user_data((void*)(this));

	/*
	m_pCtrlWin  = new Fl_Window(x,y,w-5,h-240, "");
	m_pCtrlWin->user_data((void*)(this));
 	m_pGroupA = new Fl_Group(x,y,w-10,h-300, "");
 	m_pGroupA->box(FL_ENGRAVED_BOX);
 	m_pGroupA->align(FL_ALIGN_TOP_LEFT);
 	m_pGroupA->user_data((void*)(this));
	*/

	int delta = 0;
	//m_pcoord = new PCoord(x+10,y,w-20,280, m_pVolume, INIT_SAMPLE);//h-380);//-100);
 	//m_pcoord = new PCoord(m_pGroupA->x()+3,y,w-20,m_pGroupA->h()-4, m_pVolume, INIT_SAMPLE);//h-380);//-100);
	m_pcoord = new PCoord(0,40,w,h/2, m_pVolume, INIT_SAMPLE);//h-380);//-100);
	m_pcoord->mode(FL_ALPHA | FL_RGB | FL_DOUBLE | FL_DEPTH);
	m_pVolume->SetParCoord(m_pcoord);
	m_pcoord->resizable(NULL);
	//m_pcoord = new PCoord(x,y+100,w,h);//-100);
	
	//m_pGroupA->end();
	//m_pCtrlWin->resizable(m_pCtrlWin);
	//m_pCtrlWin->end();
	
	//m_pCheckA ->down_box(FL_DOWN_BOX);

	//m_wMainWin->end();
	//m_wMainWin->resizable(m_wMainWin);
	//m_wMainWin->size_range(200,200);

	//m_pCheck1 = new Fl_Check_Button(10, h-50, 25, 20, "variable 1");
	//m_pCheck1->labelsize(TEXTSIZE);
	//m_pCtrlWin = new Fl_Window(x, h-60,w,60, "Control");
	//m_pCtrlWin->user_data((void*)(this));

	m_pGroupB = new Fl_Group(	2,
								y+h/2-COLOR_CHOOSER_HEIGHT+MARGIN,
								w-2*2,
								COLOR_CHOOSER_HEIGHT*4,"Controls");

	//m_pGroupB = new Fl_Group(x,h-200,w-10,150, "Controls");
 	m_pGroupB->box(FL_ENGRAVED_BOX);
 	m_pGroupB->align(FL_ALIGN_TOP_LEFT);
 	m_pGroupB->user_data((void*)(this));

	int yy = m_pGroupB->y();

	int step = 17;
	m_pSliderA = new Fl_Value_Slider(5,yy+1*step, (w - 10)/2, 20, "X start");
	m_pSliderA->align(FL_ALIGN_TOP_LEFT);
	m_pSliderA->labelsize(TEXTSIZE);
	m_pSliderA->type(FL_HOR_NICE_SLIDER);
	m_pSliderA->range(0, 99);
	m_pSliderA->value(0);
	m_pSliderA->step(1);    
	m_pSliderA->callback((Fl_Callback *)cb_SliderA);
	
	
	m_pSliderD = new Fl_Value_Slider(5+(w - 10)/2,yy+1*step, (w - 10)/2-7, 20, "X size");
	m_pSliderD->align(FL_ALIGN_TOP_LEFT);
	m_pSliderD->labelsize(TEXTSIZE);
	m_pSliderD->type(FL_HOR_NICE_SLIDER);
	m_pSliderD->range(1, 100);
	m_pSliderD->value(100);
	m_pSliderD->step(1);    
	m_pSliderD->callback((Fl_Callback *)cb_SliderD);
	
	
	m_pSliderB = new Fl_Value_Slider(5,yy+3*step, (w - 10)/2, 20, "Y start");
	m_pSliderB->align(FL_ALIGN_TOP_LEFT);
	m_pSliderB->labelsize(TEXTSIZE);
	m_pSliderB->type(FL_HOR_NICE_SLIDER);
	m_pSliderB->range(0, 99);
	m_pSliderB->value(0);
	m_pSliderB->step(1); 
	m_pSliderB->callback((Fl_Callback *)cb_SliderB);
	

	m_pSliderE = new Fl_Value_Slider(5+(w - 10)/2,yy+3*step, (w - 10)/2-7, 20, "Y size");
	m_pSliderE->align(FL_ALIGN_TOP_LEFT);
	m_pSliderE->labelsize(TEXTSIZE);
	m_pSliderE->type(FL_HOR_NICE_SLIDER);
	m_pSliderE->range(1, 100);
	m_pSliderE->value(100);
	m_pSliderE->step(1); 
	m_pSliderE->callback((Fl_Callback *)cb_SliderE);
	
	
	//m_pSliderC = new Fl_Value_Slider(x, h-100, w-10, 20, "test");
	m_pSliderC = new Fl_Value_Slider(5,yy+5*step, w-17, 20, "sampling");
	m_pSliderC->align(FL_ALIGN_TOP_LEFT);
	//m_pSliderC->align(FL_ALIGN_CENTER);
	m_pSliderC->labelsize(TEXTSIZE);
	m_pSliderC->type(FL_HOR_NICE_SLIDER);
	m_pSliderC->range(1, 50000);
	m_pSliderC->value(INIT_SAMPLE);
	m_pSliderC->step(1);   
	//m_pSliderC->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
	m_pSliderC->callback((Fl_Callback *)cb_SliderC);
	
	m_pLB = new Fl_Light_Button(5,
			 	    m_pSliderC->y() + 23, 
				    200, 30, "View Parallel Coordinates");
	m_pLB->labelsize(TEXTSIZE);
    	m_pLB->callback((Fl_Callback *)cb_LB);;
				    
	m_pGroupB->end();

	//m_wMainWin->end();
	//m_wMainWin->resizable(m_wMainWin);
	//m_pCtrlWin->end();
	//m_pCtrlWin->resizable(m_pCtrlWin);
}


void WinCtrlCA::show(int argc, char **argv){
	//m_wMainWin->show(argc,argv);
	m_pSliderA->show();
	m_pSliderB->show();
	m_pSliderC->show();
	//m_pCheckA = new Fl_Check_Button(10, m_h-50, 25, 20, "variable 3");
	//m_pCheckA->labelsize(TEXTSIZE);
}





void WinCtrlCA::reset()
{
	//m_wParCrdDraw->updatecolor();
	//m_wParCrdDraw->reset();
	//m_wParCrdDraw->updateview();

}


void WinCtrlCA::SaveConfig()
{
    char filename[1024];

#ifdef MULTI_VARIABLES
    sprintf(filename, "ctrllight%d.cfg", m_pVolume->m_nVolumeID);
#else
    sprintf(filename, "ctrllight.cfg");
#endif
    
    ofstream outf(filename);
    
    float value;

    outf.close();

}

void WinCtrlCA::cb_SliderA(Fl_Value_Slider* o, void* v)
{
  ((WinCtrlCA*)(o->parent()->user_data()))->cb_SliderA_i(o,v);
}

// X start
void WinCtrlCA::cb_SliderA_i(Fl_Value_Slider* o, void* v)
{
	int new_start = o->value();
	int val = 0;
	bool bUpdate = ((WinCtrlCA*)(o->parent()->user_data()))->m_pcoord->ChangeDataSize(new_start, -1, 'X', &val);
	if(!bUpdate){
		new_start = val;
		m_pSliderA->value(val);
	}
	((WinCtrlCA*)(o->parent()->user_data()))->m_pcoord->redraw();

	//m_pVolume->SetXstart(new_start);
	//m_pVolume->Read(m_pVolume->m_curnetcdf_var);
	m_pVolume->ReDraw();
}

void WinCtrlCA::cb_SliderB(Fl_Value_Slider* o, void* v)
{
  ((WinCtrlCA*)(o->parent()->user_data()))->cb_SliderB_i(o,v);
}

// Y start
void WinCtrlCA::cb_SliderB_i(Fl_Value_Slider* o, void* v)
{
	int new_start = o->value();
	int val = 0;
	bool bUpdate = ((WinCtrlCA*)(o->parent()->user_data()))->m_pcoord->ChangeDataSize(new_start, -1, 'Y', &val);
	if(!bUpdate){
		new_start = val;
		m_pSliderB->value(val);
	}
	((WinCtrlCA*)(o->parent()->user_data()))->m_pcoord->redraw();
	//m_pVolume->SetYstart(new_start);
	//m_pVolume->Read(m_pVolume->m_curnetcdf_var);
    	m_pVolume->ReDraw();
}

void WinCtrlCA::cb_SliderC(Fl_Value_Slider* o, void* v)
{
  ((WinCtrlCA*)(o->parent()->user_data()))->cb_SliderC_i(o,v);
}


void WinCtrlCA::cb_SliderC_i(Fl_Value_Slider* o, void* v)
{
	int sample = INIT_SAMPLE;
	sample = o->value();

	((WinCtrlCA*)(o->parent()->user_data()))->m_pcoord->UpdateSampleSize(sample);
	((WinCtrlCA*)(o->parent()->user_data()))->m_pcoord->redraw();
}


void WinCtrlCA::cb_SliderD(Fl_Value_Slider* o, void* v)
{
  ((WinCtrlCA*)(o->parent()->user_data()))->cb_SliderD_i(o,v);
}

// X size
void WinCtrlCA::cb_SliderD_i(Fl_Value_Slider* o, void* v)
{
	int new_count = o->value();
	int val = 0;
	bool bUpdate = ((WinCtrlCA*)(o->parent()->user_data()))->m_pcoord->ChangeDataSize(-1, new_count, 'X', &val);
	if(!bUpdate){
		new_count = val;
		m_pSliderD->value(val);
	}
	((WinCtrlCA*)(o->parent()->user_data()))->m_pcoord->redraw();
	//m_pVolume->SetXstop(new_count);
	//m_pVolume->Read(m_pVolume->m_curnetcdf_var);
    	m_pVolume->ReDraw();
}

void WinCtrlCA::cb_SliderE(Fl_Value_Slider* o, void* v)
{
  ((WinCtrlCA*)(o->parent()->user_data()))->cb_SliderE_i(o,v);
}

// Y size
void WinCtrlCA::cb_SliderE_i(Fl_Value_Slider* o, void* v)
{
	int new_count = o->value();
	int val = 0;
	bool bUpdate = ((WinCtrlCA*)(o->parent()->user_data()))->m_pcoord->ChangeDataSize(-1, new_count, 'Y', &val);
	if(!bUpdate){
		new_count = val;
		m_pSliderE->value(val);
	}
	((WinCtrlCA*)(o->parent()->user_data()))->m_pcoord->redraw();
	//m_pVolume->SetYstop(new_count);
	//m_pVolume->Read(m_pVolume->m_curnetcdf_var);
    	m_pVolume->ReDraw();
}


void WinCtrlCA::cb_LB(Fl_Light_Button* o, void* v) {
  ((WinCtrlCA*)(o->parent()->user_data()))->cb_LB_i(o,v);
}

void WinCtrlCA::cb_LB_i(Fl_Light_Button* o, void*)
{

	bool bSelected = o->value();
	
	((WinCtrlCA*)(o->parent()->user_data()))->m_pcoord->EnablePCoord(bSelected); 
	((WinCtrlCA*)(o->parent()->user_data()))->m_pcoord->redraw();
	//SaveConfig();	
}