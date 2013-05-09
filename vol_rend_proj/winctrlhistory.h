//
// C++ Interface: winctrlhistory
//
// Description: 
//
//
// Author: Hongfeng Yu <hfyu@ucdavis.edu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WINCTRLHISTORY_H
#define WINCTRLHISTORY_H

/**
	@author Hongfeng Yu <hfyu@ucdavis.edu>
*/


#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Slider.H>
#include "cxvolume.h"
#include "winctrlmain.h"
#include <list>

using namespace std;


class cxSnapshot : public Fl_Button
{
public:
    cxSnapshot(int, int, int ,int);
    ~cxSnapshot();    
    
public:
    cxVisStatus * m_pVisStatus;
    Fl_RGB_Image  * m_pActiveImage;
    Fl_Image * m_pDeactiveImage;
    list<cxSnapshot *>::iterator m_pIter;
};

class WinCtrlHistory
{
public:
    WinCtrlHistory(int, int, int, int);
    ~WinCtrlHistory();
    
    void SetVolume(cxVolume * pVolume){
        m_pVolume = pVolume;
    }
    
    void SetCtrlMain(WinCtrlMain * pCtrlMain){
        m_pCtrlMain = pCtrlMain;
    }    
    
    void UpdateView(cxVisStatus * pVisStatus);
    void UpdateCtrl(cxVisStatus * pVisStatus);
    
    void show(int argc, char **argv);
    
private:
    void GenWindow(int x, int y, int w, int h);
    
    inline void cb_ButtonA_i(Fl_Button*, void*);
    static void cb_ButtonA(Fl_Button*, void*);
    
    inline void cb_ButtonB_i(Fl_Button*, void*);    
    static void cb_ButtonB(Fl_Button*, void*);
    
    inline void cb_ButtonC_i(Fl_Button*, void*);    
    static void cb_ButtonC(Fl_Button*, void*);

    inline void cb_Snapshot_i(cxSnapshot*, void*);    
    static void cb_Snapshot(cxSnapshot*, void*);
    
    inline void cb_SliderA_i(Fl_Slider*, void*);    
    static void cb_SliderA(Fl_Slider*, void*);

    
public:    
    cxVolume * m_pVolume;    
    WinCtrlMain * m_pCtrlMain;
    
    //main window for ctrshow
    Fl_Window * m_wMainWin;
    
    Fl_Scroll * m_pScrollA;
    Fl_Pack * m_pPack; 
    Fl_Button * m_pButtonA;
    Fl_Button * m_pButtonB;
    Fl_Button * m_pButtonC;
    Fl_Slider * m_pSliderA;
    
    list<cxSnapshot *> m_lSnapshot;
    list<cxSnapshot *>::iterator m_iCur;
    
    cxSnapshot * m_pActiveSnapshot;
    
    unsigned int m_nFrameCount;
    
    unsigned int m_nInterSteps;
};

void cb_wPlayOn(void * pData);

#endif
