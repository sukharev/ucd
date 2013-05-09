/***************************************************************************
                          winctrlca.h  -  description
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

#ifndef WINCTRLCA_H
#define WINCTRLCA_H


/**
  *@author Hongfeng YU
  */

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Value_Slider.H>
#include "cxvolume.h"
#include "parcrd.h"
#include "abstract.h"

class WinVisMain;

#define INIT_SAMPLE  1
//#define INIT_SAMPLE  500

class WinCtrlCA 
{
public: 
	WinCtrlCA(int, int ,int, int, cxVolume *);
	~WinCtrlCA();

    void SetVolume(cxVolume * pVolume){
        m_pVolume = pVolume;
    }
    
	void AddUI(int n);

	void show(int argc, char **argv);
	
	void reset();
	PCoord* getPCoord() { return m_pcoord; }

public:
	static void cb_SliderA(Fl_Value_Slider* o, void* v);
	void cb_SliderA_i(Fl_Value_Slider* o, void* v);

	static void cb_SliderB(Fl_Value_Slider* o, void* v);
	void cb_SliderB_i(Fl_Value_Slider* o, void* v);

	static void cb_SliderC(Fl_Value_Slider* o, void* v);
	void cb_SliderC_i(Fl_Value_Slider* o, void* v);

	static void cb_SliderD(Fl_Value_Slider* o, void* v);
	void cb_SliderD_i(Fl_Value_Slider* o, void* v);

	static void cb_SliderE(Fl_Value_Slider* o, void* v);
	void cb_SliderE_i(Fl_Value_Slider* o, void* v);
	
        static void cb_LB(Fl_Light_Button*, void*);
	void cb_LB_i(Fl_Light_Button*, void*);
	
private:
	void GenWindow(int x, int y, int w, int h);
    void SaveConfig();
	
public:
	//main window for ctrshow
	Fl_Window *m_wMainWin;
	Fl_Window *m_pCtrlWin;
	
			
	//group
	Fl_Group *m_pGroupA;
	Fl_Group *m_pGroupB;
	Fl_Group *m_pGroupC;
    Fl_Group *m_pGroupD;

	Fl_Check_Button *m_pCheckA;
	
	//WinParCrdDraw *m_wParCrdDraw;
	PCoord *m_pcoord;

    //pointer to the volume
    cxVolume * m_pVolume;
	int m_h;
	int m_w;
	int m_x;
	int m_y;

	Fl_Value_Slider *m_pSliderA;
	Fl_Value_Slider *m_pSliderB;
	Fl_Value_Slider *m_pSliderC;

	Fl_Value_Slider *m_pSliderD;
	Fl_Value_Slider *m_pSliderE;
	
	Fl_Light_Button *m_pLB;
};

#endif
