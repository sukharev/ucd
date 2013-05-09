//
// C++ Interface: WinVisMain
//
// Description: 
//
//
// Author: Hongfeng Yu <hfyu@ucdavis.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef WINVISMAIN_H
#define WINVISMAIN_H

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Value_Slider.H>

class WinVisView;
class cxVolume;

class WinVisMain  
{
public:
	WinVisMain(int x =0, int y = 0, int w=600, int h = 600);
	virtual ~WinVisMain();
	
	void Show(int argc, char **argv);    
	void SetVolume(cxVolume * pVolume);
	void AddVolume(cxVolume * pVolume);
	
	//call back function for slider
    	inline void cb_SliderA_i(Fl_Value_Slider*, void*);
    	static void cb_SliderA(Fl_Value_Slider*, void*);

private:
	void MyCreateWindow(int x, int y, int w, int h);	

public:
	Fl_Window * m_pMainWin;
	Fl_Value_Slider * m_pSlider;
	WinVisView * m_wView;
private:
	bool m_update_contour;
};

#endif
