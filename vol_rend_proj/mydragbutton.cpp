/***************************************************************************
                          dragonbutton.cpp  -  description
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

#include "mydragbutton.h"
#include "wincolormap.h"

MyDragButton::~MyDragButton()
{

}

int MyDragButton::handle(int event)
{
	switch (event)
	{
		case FL_DRAG:
			move();
			parent()->redraw();
			return Fl_Button::handle(FL_DRAG);

		case FL_RELEASE:
			if (m_pDraw != NULL)
			m_pDraw->updateview();
			return Fl_Button::handle(FL_RELEASE);

		case FL_PUSH:
			if(Fl::event_button3())
				remove();
			m_mousex = Fl::event_x();
			m_mousey = Fl::event_y();
			return Fl_Button::handle(FL_PUSH);

		default:	
			return Fl_Button::handle(event);
	}
}

void MyDragButton::move()	
{
	int tx = Fl::event_x();
	int ty = Fl::event_y();
	if( tx != m_mousex)
	{
		int deltax = tx-m_mousex;
		//int deltay = ty-m_mousey;
		int curx = x();
		if( curx + deltax < m_maxx && curx + deltax > m_minx)
		{
			position(x()+deltax,y());
			if(m_pDraw!=NULL)
			{
				m_pDraw->ReadColors();
				m_pDraw->redraw();
			}
		}
		m_mousex = tx;
		m_mousey = ty;
	}
}

float MyDragButton::pos()
{
	if ( (m_maxx- m_minx) == 0)
		return 0;

	float curx = x();
	float total = m_maxx - m_minx;
	return (curx - m_minx) / total;
}

void MyDragButton::remove()
{
	WinColorMap *pMap = m_pDraw->m_pParent;
	vector<MyDragButton *>::iterator pb;
	for(pb = pMap->m_colors.begin(); pb != pMap->m_colors.end(); pb++)
	{
		if( this == * pb )
		{
			this->hide();
			pMap->m_colors.erase(pb);
			pMap->m_gFrm->redraw();
			m_pDraw->updatecolor();
			break;
		}
	}
  
}
