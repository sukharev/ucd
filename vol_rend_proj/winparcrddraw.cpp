/***************************************************************************
                          winparcrddraw.cpp  -  description
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

#include "winparcrddraw.h"

#define SAMPLESIZE 10

WinParCrdDraw::WinParCrdDraw(int X,int Y,int W,int H) : Fl_Widget(X,Y,W,H) 
{	
	m_pParent =NULL;
	m_pcoord = new PCoord(X,Y,W,H);
	//m_data = new TestDataset;

	// simple test data
/*
	m_data->m_dim = 2;
	m_data->names = new char*[2];
	m_data->names[0] = new char[20];
	strcpy(m_data->names[0], "axis 1");
	m_data->names[1] = new char[20];
	strcpy(m_data->names[1], "axis 2");
	m_data->dim_max = new float[2];
	m_data->dim_min = new float[2];
	m_data->dim_max[0] = 30;
	m_data->dim_max[1] = 20;
	m_data->dim_min[0] = 15;
	m_data->dim_min[1] = 10;
	m_pcoord->InitData(m_data);
	*/
	m_pcoord->InitParentWindow(this);

	reset();
}

void WinParCrdDraw::draw()
{
	gl_start();
	fl_clip(x(),y(),w(),h());
	fl_color(FL_DARK3);
	fl_push_matrix();
	
	
	m_pcoord->SetDisplaySize(w(), h());
	m_pcoord->Display();

	
	fl_line_style(FL_SOLID, 1);
	
	fl_pop_matrix();
	fl_pop_clip();
	
	gl_finish();
}

int WinParCrdDraw::handle(int event)
{
/*
    switch(event) {
  case FL_DRAG:
        if (Fl::event_button1()) {
          newline();
        } else if (Fl::event_button3()) {
          eraseline();
        }     
          
        redraw();
        return 1;
    case FL_PUSH:
        //m_mousex = Fl::event_x();
        //m_mousey = Fl::event_y();
        if (Fl::event_button1()) {
          initline();
        } else if (Fl::event_button3()) {
          initline(true);
        }
              
        redraw();
    case FL_RELEASE:
        updateview();       
        return 1;
  default:
        return Fl_Widget::handle(event);
    }
*/
	return 0;
}

void WinParCrdDraw::reset()
{
	redraw();
}
