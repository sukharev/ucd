//
// C++ Interface: winsliceview
//
// Description: 
//
//
// Author: Hongfeng Yu <hfyu@ucdavis.edu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WINSLICEVIEW_H
#define WINSLICEVIEW_H

/**
	@author Hongfeng Yu <hfyu@ucdavis.edu>
*/

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/gl.h>


#include <assert.h>
#include "cxvolume.h"


class WinSliceView : public Fl_Gl_Window
{
public:
    WinSliceView(int x,int y,int w,int h,const char *l=0);

    virtual ~WinSliceView();

private:

    void draw();

    int handle(int event1);

public:
    void initline(bool is_erase = false);
    void update();
    void moveupdate1(float diffx, float diffy);
    void moveupdate();
    void updateSelection(int axis);

	void setCorrMode(bool arg) { 
		m_bcorr = arg; 
		m_startx = m_starty = 0.0;
		m_crossx = m_crossy = 0.0;
		if(m_pVolume)
			m_pVolume->m_bcorr = m_bcorr;
		redraw();
	}
	bool getCorrMode() { return m_bcorr; }
    cxVolume * m_pVolume;
    int m_nAxis;
    bool m_bBlack;

private:
    map<int,float> m_line;	
    float m_startx;
    float m_starty;
    float m_finishx;
    float m_finishy;

	float m_crossx;
	float m_crossy;
    bool m_bRegion;
	bool m_bcorr;
    
};

class WinSliceMain
{
public:
    
    WinSliceMain(int x,int y,int w,int h,const char *l=0);
    
    ~WinSliceMain();

    void GenWindow(int x,int y,int w,int h,const char *l=0);

    void show(int argc = 0, char ** argv = NULL);

    void SetVolume(cxVolume * pVolume, int axis)
    {
        if (m_pSliceView != NULL) {
            m_pSliceView->m_pVolume = pVolume;   //? 
            m_pSliceView->m_nAxis = axis;
            int h = m_pSliceView->h();
            int w = m_pSliceView->w();
            pVolume->m_pSliceView[axis - SLICE_X_AXIS] = m_pSliceView;
        }
           
    }

    //call back function for slider
    inline void cb_SliderA_i(Fl_Value_Slider*, void*);
    static void cb_SliderA(Fl_Value_Slider*, void*);
	
    inline void cb_SliderB_i(Fl_Value_Slider*, void*);
    static void cb_SliderB(Fl_Value_Slider*, void*);

    inline void cb_SliderTAC_i(Fl_Value_Slider*, void*);
    static void cb_SliderTAC(Fl_Value_Slider*, void*);
    
public:
    inline void cb_RadioA_i(Fl_Round_Button*, void*);
    static void cb_RadioA(Fl_Round_Button*, void*);

    inline void cb_RadioB_i(Fl_Round_Button*, void*);
    static void cb_RadioB(Fl_Round_Button*, void*);

    inline void cb_RadioC_i(Fl_Round_Button*, void*);
    static void cb_RadioC(Fl_Round_Button*, void*);

    inline void cb_LB_i(Fl_Light_Button*, void*);
    static void cb_LB(Fl_Light_Button*, void*);

    inline void cb_LB_color_i(Fl_Light_Button*, void*);
    static void cb_LB_color(Fl_Light_Button*, void*);	

    inline void cb_SaveButton_i(Fl_Button*, void*);
    static void cb_SaveButton(Fl_Button*, void*);

    inline void cb_ClearButton_i(Fl_Button*, void*);
    static void cb_ClearButton(Fl_Button*, void*);
    
    inline void cb_SaveTAC_i(Fl_Button*, void*);
    static void cb_SaveTAC(Fl_Button*, void*);

	inline void cb_SaveGTAC_i(Fl_Button*, void*);
    static void cb_SaveGTAC(Fl_Button*, void*);

	void cb_LB_corr_i(Fl_Light_Button*, void*);
    static void cb_LB_corr(Fl_Light_Button*, void*);
public:

    Fl_Window * m_pMainWin;

    Fl_Value_Slider * m_pSlider;
    Fl_Value_Slider * m_pSliderCtr;
    Fl_Value_Slider * m_pSliderTAC;
    
    WinSliceView * m_pSliceView;

    //roundbutton
    Fl_Round_Button *m_pRoundA;
    Fl_Round_Button *m_pRoundB;
    Fl_Round_Button *m_pRoundC;
	Fl_Light_Button *m_pLB_corr;
    Fl_Light_Button *m_pLB;
    Fl_Light_Button *m_pLB_color;
    Fl_Button *m_pSaveButton;
    Fl_Button *m_pClearButton;
    Fl_Button *m_pSaveTAC;
	Fl_Button *m_pSaveGTAC;

    Fl_Select_Browser * m_pBrowser;

    float m_contour;
    bool m_bLBseleced;
    int m_TAC_finish;     // last timestamp for TAC
   
};

#endif
