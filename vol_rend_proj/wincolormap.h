/***************************************************************************
                          wincolormap.h  -  description
                             -------------------
    begin                : Thu Jan 23 2003
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

#ifndef WINCOLORMAP_H
#define WINCOLORMAP_H

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Pack.H>
#include <FL/fl_draw.H>
#include <vector>
#include "wintinydraw.h"
#include "mydragbutton.h"
#include "cxcolor.h"
#include "winhistogram.h"

using namespace std;

/**
  *@author Hongfeng YU
  */

#define TF_SIZE 1024

class WinTinyDraw;
class MyDragButton;
class WinColorMap;
class cxVolume;

class MyRuler:public Fl_Widget
{
public:
	MyRuler(int X,int Y,int W,int H) : Fl_Widget(X,Y,W,H) {m_pColorMap = NULL;}
	void AddColor(int xpos);

private:
	int handle(int event);
	void draw();
public:
	WinColorMap * m_pColorMap;
};

class MyColorButton:public Fl_Button
{
public:
	MyColorButton(int x, int y, int w, int h) : Fl_Button(x,y,w,h){};
	vector<cxColor> m_colors;
	vector<int> m_colorpos;
private:
	void draw();
	cxColor getcolor(int pos);
};

typedef vector<MyDragButton *> colormap;

class WinColorMap{
public: 
	WinColorMap(int, int, int, int, cxVolume * pVolume);
	~WinColorMap();
	
public:
	void GenWindow(int x, int y, int w, int h);
	void show(int argc, char **argv);
	void reset();
	void SetMinMax(float min, float max);

	cxColor GetColor(float value);
    
	void SaveColor();
    void LoadColorFromFile();   
    void LoadColorFromVolume();   
	

	inline void cb_bReset_i(Fl_Button*, void*);
	static void cb_bReset(Fl_Button*, void*);

	inline void cb_bAdd_i(Fl_Button*, void*);
	static void cb_bAdd(Fl_Button*, void*);

	inline void cb_bDel_i(Fl_Button*, void*);
	static void cb_bDel(Fl_Button*, void*);
	
	cxColor * GetColorMap(){return m_Colors;}
	float * GetTransparence(){ return m_Values;}
    
    void SetVolume(cxVolume * pVolume){
        m_pVolume = pVolume;
    }

    void GenTexColorMap();  
    
public:
	Fl_Window *m_wMainWin;
    WinHistogram *m_wHistogram;   
	WinTinyDraw *m_wTinyDraw;
	Fl_Box * m_bMin;
	Fl_Box * m_bMax;
	MyRuler * m_bRuler;
	Fl_Button * m_bReset;
	Fl_Button * m_bAdd;
	Fl_Button  * m_bDel;
	Fl_Group * m_gFrm;
	Fl_Color_Chooser *m_cColorChooser;
	Fl_Scroll * m_sScroll;
	Fl_Pack * m_pPack;
	
	colormap m_colors;
	
	float m_min;
	float m_max;
	
	float m_curisovalue;
	
	cxColor m_Colors[TF_SIZE];
	float m_Values[TF_SIZE];
    
    unsigned int m_nTexname_Colormap;
    
    /* pointer to volume*/
    cxVolume * m_pVolume;
};

#endif
