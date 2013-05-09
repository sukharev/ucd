/***************************************************************************
                          winctrllight.h  -  description
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

#ifndef WINCTRLLIGHT_H
#define WINCTRLLIGHT_H


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
#include "cxvolume.h"

class WinVisMain;

class WinCtrlLight {
public: 
	WinCtrlLight(int, int ,int, int, cxVolume *);
	~WinCtrlLight();

    void SetVolume(cxVolume * pVolume){
        m_pVolume = pVolume;
    }
    
	void show(int argc, char **argv);
	
	void reset();

private:
	void GenWindow(int x, int y, int w, int h);

	//call back function for checkbox
	inline void cb_CheckA_i(Fl_Check_Button*, void*);
	static void cb_CheckA(Fl_Check_Button*, void*);

	inline void cb_CheckB_i(Fl_Check_Button*, void*);
	static void cb_CheckB(Fl_Check_Button*, void*);
	
	inline void cb_CheckC_i(Fl_Check_Button*, void*);
	static void cb_CheckC(Fl_Check_Button*, void*);
	
	inline void cb_CheckD_i(Fl_Check_Button*, void*);
	static void cb_CheckD(Fl_Check_Button*, void*);

	inline void cb_CheckE_i(Fl_Check_Button*, void*);
	static void cb_CheckE(Fl_Check_Button*, void*);

	inline void cb_CheckF_i(Fl_Check_Button*, void*);
	static void cb_CheckF(Fl_Check_Button*, void*);

	inline void cb_CheckG_i(Fl_Check_Button*, void*);
	static void cb_CheckG(Fl_Check_Button*, void*);
	
	inline void cb_CheckH_i(Fl_Check_Button*, void*);
	static void cb_CheckH(Fl_Check_Button*, void*);

	inline void cb_CheckI_i(Fl_Check_Button*, void*);
	static void cb_CheckI(Fl_Check_Button*, void*);

	//call back function for slider
	inline void cb_SliderA_i(Fl_Value_Slider*, void*);
	static void cb_SliderA(Fl_Value_Slider*, void*);
	
	inline void cb_SliderB_i(Fl_Value_Slider*, void*);
	static void cb_SliderB(Fl_Value_Slider*, void*);

    inline void cb_SliderC_i(Fl_Value_Slider*, void*);
    static void cb_SliderC(Fl_Value_Slider*, void*);
    
    inline void cb_SliderD_i(Fl_Value_Slider*, void*);
    static void cb_SliderD(Fl_Value_Slider*, void*);

    inline void cb_SliderE_i(Fl_Value_Slider*, void*);
    static void cb_SliderE(Fl_Value_Slider*, void*);

    inline void cb_SliderF_i(Fl_Value_Slider*, void*);
    static void cb_SliderF(Fl_Value_Slider*, void*);

    inline void cb_SliderG_i(Fl_Value_Slider*, void*);
    static void cb_SliderG(Fl_Value_Slider*, void*);

	//call back function for button
	inline void cb_ButtonA_i(Fl_Button*, void*);
	static void cb_ButtonA(Fl_Button*, void*);
	
	inline void cb_ButtonB_i(Fl_Button*, void*);
	static void cb_ButtonB(Fl_Button*, void*);

	inline void cb_ButtonC_i(Fl_Button*, void*);
	static void cb_ButtonC(Fl_Button*, void*);
	
	inline void cb_ButtonD_i(Fl_Button*, void*);
	static void cb_ButtonD(Fl_Button*, void*);
	
	//call back function for browser
	inline void cb_BrowserA_i(Fl_Hold_Browser*, void*);
	static void cb_BrowserA(Fl_Hold_Browser*, void*);

    void SaveConfig();
	
public:
	//main window for ctrshow
	Fl_Window *m_wMainWin;
	
	//checkbox
	Fl_Check_Button *m_pCheckA;
	Fl_Check_Button *m_pCheckB;
	Fl_Check_Button *m_pCheckC;
	Fl_Check_Button *m_pCheckD;
	Fl_Check_Button *m_pCheckE;
	Fl_Check_Button *m_pCheckG;
	Fl_Check_Button *m_pCheckF;
	Fl_Check_Button *m_pCheckH;
	Fl_Check_Button *m_pCheckI;

    //roundbutton
    Fl_Round_Button *m_pRoundA;
    Fl_Round_Button *m_pRoundB;
    Fl_Round_Button *m_pRoundC;
	
	//slider
	Fl_Value_Slider * m_pSliderA;
	Fl_Value_Slider * m_pSliderB;	
    Fl_Value_Slider * m_pSliderC;
    Fl_Value_Slider * m_pSliderD;
    Fl_Value_Slider * m_pSliderE;
    Fl_Value_Slider * m_pSliderF;
    Fl_Value_Slider * m_pSliderG;


	//button
	Fl_Button * m_pButtonA;
	Fl_Button * m_pButtonB;
	Fl_Button * m_pButtonC;
	Fl_Button * m_pButtonD;
		
	//list
	Fl_Hold_Browser * m_pBrowserA;
	
	//group
	Fl_Group *m_pGroupA;
	Fl_Group *m_pGroupB;
	Fl_Group *m_pGroupC;
    Fl_Group *m_pGroupD;
	
    //pointer to the volume
    cxVolume * m_pVolume;
};

#endif
