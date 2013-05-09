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

#ifndef WINPARCRDDRAW_H
#define WINPARCRDDRAW_H

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
#include "parcrd.h"

using namespace std;

/**
  *@author Hongfeng YU
  */
class WinColorMap;



class WinParCrdDraw : public Fl_Widget  
{
public:
	//WinParCrdDraw(int X,int Y,int W,int H) : Fl_Widget(X,Y,W,H) {m_pParent =NULL;}	
	WinParCrdDraw(int X,int Y,int W,int H);// : Fl_Widget(X,Y,W,H) {m_pParent =NULL; reset();}	
	
	WinColorMap * m_pParent;
	void setparent(WinColorMap * w){m_pParent = w;}
	
	void reset();
	
	PCoord* getPCoord() { return m_pcoord; }
public:
	
/*
	map<int,float> m_line;	
	int m_mousex;
	int m_mousey;
*/
private:
	void draw();

	int handle(int event);
	
/////////////////////////////////////////
// parallel coordinates implementation
private:
	PCoord* m_pcoord;

};

#endif
