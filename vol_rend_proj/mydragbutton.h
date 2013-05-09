/***************************************************************************
                          dragonbutton.h  -  description
                             -------------------
    begin                : Fri Jan 24 2003
    copyright            : (C) 2003 by Hongfeng Yu
    email                : root@localhost.localdomain
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DRAGONBUTTON_H
#define DRAGONBUTTON_H

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include "wintinydraw.h"

class WinTinyDraw;

class MyDragButton : public Fl_Button  {
public:
	MyDragButton(int x, int y, int w, int h):Fl_Button(x,y,w,h){m_maxx=m_minx=0; m_pDraw = NULL;m_isDrag = false;}
	~MyDragButton();
	
	void set(int minx, int maxx){ m_minx = minx; m_maxx = maxx; }
	void setdraw(WinTinyDraw * w) { m_pDraw =w;}
	float pos();

	unsigned char m_r;
	unsigned char m_g;
	unsigned char m_b;
private:
	int handle (int event);
	
	void move();
	
	void remove();
	
	int m_maxx;
	int m_minx;
	int m_mousex;
	int m_mousey;
	bool m_isDrag;

	WinTinyDraw * m_pDraw;
};


#endif
