//
// C++ Implementation: WinVisMain
//
// Description: 
//
//
// Author: Hongfeng Yu <hfyu@ucdavis.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef LINUX
#include <windows.h>
#endif

#include "winvismain.h"
#include "winvisview.h"
#include "cxvolume.h"
#include <math.h>

#define TEXTSIZE 12
WinVisMain::WinVisMain(int x, int y, int w, int h)
{
	m_wView = NULL;
	MyCreateWindow(x,y,w,h);
	m_update_contour = false;
}

WinVisMain::~WinVisMain()
{

}

void WinVisMain::MyCreateWindow(int x, int y,int w , int h )
{
	m_pMainWin = new Fl_Window(x, y, w, h);
	m_pMainWin->user_data((void*)(this));

	m_wView = new WinVisView(0, 0, w, h-25/*h*/, "cxvolume");
	//m_wView = new WinVisView(x,y,w,h-40, "cxvolume");
	
	assert(m_wView != NULL);
	
	//m_wView->resizable(NULL);
	//m_wView->box(FL_NO_BOX);
	m_wView->box(FL_FLAT_BOX);
	m_wView->color(49);
	m_wView->m_pVisMain = this;
	//m_wView->size_range(600,600);
   	m_wView->mode(FL_ALPHA | FL_RGB | FL_DOUBLE | FL_DEPTH);   
	m_wView->resizable(m_wView);
	m_wView->end();


	//m_pSlider = new Fl_Value_Slider(x, h-40, w, 20);
	m_pSlider = new Fl_Value_Slider(5, h - 22, w - 10, 20);
   	m_pSlider->align(FL_ALIGN_TOP_LEFT);
   	m_pSlider->labelsize(TEXTSIZE);
   	m_pSlider->type(FL_HOR_NICE_SLIDER);
    	m_pSlider->range(0, 1.0);
    	m_pSlider->value(0.0);
    	m_pSlider->step(0.005);   
    	m_pSlider->when( FL_WHEN_CHANGED | FL_WHEN_RELEASE);
    	m_pSlider->callback((Fl_Callback *)cb_SliderA);

	m_pMainWin->resizable(m_pMainWin);
	m_pMainWin->end();

}



void WinVisMain::cb_SliderA(Fl_Value_Slider* o, void* v)
{
  ((WinVisMain*)(o->parent()->user_data()))->cb_SliderA_i(o,v);
}


inline void WinVisMain::cb_SliderA_i(Fl_Value_Slider* o, void* v)
{
     if(m_wView == NULL || m_wView->m_pVolume == NULL || m_wView->m_pVolume->m_netcdf == NULL)
         return;

    //TODO: useless test
    if (!m_wView->m_pVolume->m_curnetcdf_var)
	return;
    int time_in = 0;
    int time_out = 0;
    int time_steps = 0;

    float pos = o->value();

    m_wView->m_pVolume->m_netcdf->get_varinfo(m_wView->m_pVolume->m_curnetcdf_var);
    time_steps = m_wView->m_pVolume->m_netcdf->get_num_timesteps(m_wView->m_pVolume->m_curnetcdf_var);
    time_steps = time_steps-1;
    time_in = abs(time_steps*pos);
    if(time_in<0) time_in = 0;

    time_out = 1;


	m_wView->m_pVolume->SetTimePeriod(time_in, time_out); 
    if (Fl::event_button1()) {
        //cout <<  "push" << endl;
		if(m_wView->m_pVolume->IsEnabledLineContours()){
			m_wView->m_pVolume->EnableLineContours(false);
			m_update_contour = true;
		} 
	}else{
		//cout << "release" << endl;
		if(m_update_contour){
			m_wView->m_pVolume->EnableLineContours(true); 
			m_update_contour = false;
		}
    }    
	
	//m_wView->m_pVolume->SetTimePeriod(time_in, time_out);
    m_wView->m_pVolume->ReDraw();
}


void WinVisMain::Show(int argc, char **argv)
{
	m_pMainWin->show(argc,argv);
	m_wView->show(argc,argv);
}

void WinVisMain::SetVolume(cxVolume * pVolume)
{
	if(m_wView) {
		m_wView->m_pVolume = pVolume;
        pVolume->SetVisView(m_wView);
    	}
}

void WinVisMain::AddVolume(cxVolume * pVolume)
{
    if(m_wView) {
        m_wView->m_vVolume.push_back(pVolume);
        m_wView->m_vTexTF.push_back(0);
        m_wView->m_vTexVol.push_back(0);
        pVolume->SetVisView(m_wView);
    }
}

