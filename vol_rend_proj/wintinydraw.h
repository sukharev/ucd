/***************************************************************************
                          wintinydraw.h  -  description
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

#ifndef WINTINYDRAW_H
#define WINTINYDRAW_H

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Image.H>
#include <vector>
#include <set>
#include <map>
#include "wincolormap.h"
#include "mydragbutton.h"
#include "cxcolor.h"

using namespace std;

/**
  *@author Hongfeng YU
  */
class WinColorMap;

class WinTinyDraw : public Fl_Widget  
{
public:
	WinTinyDraw(int X,int Y,int W,int H) : Fl_Widget(X,Y,W,H) {m_pParent =NULL; reset();}	

	void reset();
	cxColor getcolor(int pos);
	cxColor getbackcolor(int pos);

	void setparent(WinColorMap * w){m_pParent = w;}
	void updateview();
	void updatecolor();
	void ReadColors();

	WinColorMap * m_pParent;
	
	vector<cxColor> m_colors;
	vector<int> m_colorpos;

public:

	map<int,float> m_line;	
	int m_mousex;
	int m_mousey;

private:
	int handle(int event);
	void draw();
	void newline();
	void initline(bool is_erase = false);
    void eraseline();   
	float getheight(int pos);
	unsigned int * getStatistic();
	void drawStatistic();
	cxColor FindColor(int pos);
	float FindPos(int pos);
	void drawIsoValue( );
	
};

#endif
