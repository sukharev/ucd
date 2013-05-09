/***************************************************************************
                          wintinydraw.cpp  -  description
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

#include "wintinydraw.h"

#define SAMPLESIZE 10

void WinTinyDraw::draw()
{
//     Fl::visual(FL_RGB);
//     
//     int width = w();
//     int height = h();
//     
//     static uchar *image = NULL;
//     
//     if (image != NULL)
//         delete image;
//     
//     image = new uchar[3*width*height];
//     
//     for (int x = 0; x < width; x++) {
//         cxColor color = getcolor(x);
//         
//         for (int y = 0; y < height; y++) {
//             int id = x + y * width;
//             image[id * 3 + 0] = (uchar)(color.r() * 255);   
//             image[id * 3 + 1] = (uchar)(color.g() * 255);   
//             image[id * 3 + 2] = (uchar)(color.b() * 255);   
//         }
//     }
//     
//      fl_draw_image(image, x(), y(), width, height);
     

	fl_clip(x(),y(),w(),h());
	fl_color(FL_DARK3);
	fl_push_matrix();
	

	for(int i = 0; i< w(); i++)
	{
		cxColor color = getcolor(i);
		fl_color(((int)(color.r() * 255)), ((int) (color.g() * 255)), ((int)( color.b() * 255)));
		fl_line(x()+i, y(), x()+ i , y()+ h());	
	}
	
	map<int,float>::iterator pv;
	//fl_color(FL_YELLOW);
    fl_color(FL_GRAY);   
	fl_line_style(FL_SOLID, 2);
	fl_begin_line();
	for(pv = m_line.begin(); pv != m_line.end(); pv++)
	{
			fl_vertex(pv->first+x(), pv->second+y());
	}
	fl_end_line();	
    
    
	//drawIsoValue();
	
	//drawStatistic();
	
	fl_line_style(FL_SOLID, 1);
	
	fl_pop_matrix();
	fl_pop_clip();
	
}

int WinTinyDraw::handle(int event)
{
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
}

void WinTinyDraw::reset()
{
	m_line.clear();
	m_mousex = 0;
	m_mousey =0;

	m_colors.clear();
	m_colorpos.clear();

	m_colors.push_back(cxColor::cxColor(0,0,0));
	m_colorpos.push_back(0);

	m_colors.push_back(cxColor::cxColor(1,1,1));
	m_colorpos.push_back(w());

	for(int i = 0; i< w(); i ++)
		m_line[i] = h()*0.1;

	redraw();
}

void WinTinyDraw::newline()
{
	int tx = Fl::event_x();
	int ty = Fl::event_y();
	
	if(tx!=m_mousex || ty!=m_mousey)
	{
		int min = tx > m_mousex ? m_mousex:tx;
		int max = tx > m_mousex ? tx:m_mousex;
		for(int i = min; i<= max; i++)
		{
			int index = i - x();
			int height = ty - y();
			if(height < 0)
				height = 0;
			if(height > h())
				height = h();
	  	m_line[index] = (float)height;
	 	}
	 	m_mousex = tx;
		m_mousey =ty;
	}
}

void WinTinyDraw::eraseline()
{
    int tx = Fl::event_x();
    int ty = Fl::event_y();
    
    if(tx!=m_mousex || ty!=m_mousey)
    {
        int min = tx > m_mousex ? m_mousex:tx;
        int max = tx > m_mousex ? tx:m_mousex;
        for(int i = min; i<= max; i++)
        {
            int index = i - x();
            m_line[index] = (float)h();
        }
        m_mousex = tx;
        m_mousey =ty;
    }
}


cxColor WinTinyDraw::getcolor(int pos)
{
	vector<int>::iterator pf;
	int i =0;
	for(pf = m_colorpos.begin();pf!=m_colorpos.end(); pf++, i++)
	{
    	if((pf+1)!=m_colorpos.end())
		{
       	if ( pos >= * pf && pos <= *(pf+1))
			{
           	float tv = ((float)(pos - * pf)) /(*(pf+1) -*pf);
				cxColor start = m_colors[i];
				cxColor end = m_colors[i+1];
//				float tv =( h() - height ) / h();
				float r = start.r()*(1-tv) + end.r()*tv;
				float g = start.g()*(1-tv)+end.g()*tv;
				float b = start.b()*(1-tv) + end.b()*tv;
  				
				float height = getheight(pos);
				if(height == -1)
					return cxColor::cxColor(0,0,0,0);	
/*				float r = start.r*(1-tv) + end.r*tv;
				float g = start.g*(1-tv)+end.g*tv;
				float b = start.b*(1-tv) + end.b*tv;
*/
				float factor =( h() - height ) /((float) h());
				//return cxColor::cxColor(r*factor,g*factor,b*factor,1);
				return cxColor::cxColor(r,g,b,factor);
			}
		}
	}
	return cxColor::cxColor(0,0,0,0);
}

float WinTinyDraw::getheight(int pos)
{
	return m_line[pos];
}

cxColor WinTinyDraw::getbackcolor(int pos)
{
	vector<int>::iterator pf;
	int i =0;
	for(pf = m_colorpos.begin();pf!=m_colorpos.end(); pf++, i++)
	{
    	if((pf+1)!=m_colorpos.end())
		{
       	if ( pos >= * pf && pos <= *(pf+1))
			{
           	float tv = (((float)pos) - * pf) /(*(pf+1) -*pf);
				cxColor start = m_colors[i];
				cxColor end = m_colors[i+1];

				float r = start.r()*(1-tv) + end.r()*tv;
				float g = start.g()*(1-tv)+end.g()*tv;
				float b = start.b()*(1-tv) + end.b()*tv;
				return cxColor::cxColor(r,g,b);				
			}
		}
	}
	return cxColor::cxColor(0,0,0);	
}

void WinTinyDraw::updateview()
{
	if(m_pParent != NULL)
	{
// 		if(m_pParent->m_pVisMain!=NULL)
// 		{
// 			WinVisView * pView = m_pParent->m_pVisMain->m_wVisView;
// 			CSlice * pCurSlice = pView->getcurslice();
// 			pView->CreateSlicePolygons(pCurSlice);
// 			pView->redraw();
// 		}
		m_pParent->SaveColor();
	}
}

unsigned int * WinTinyDraw::getStatistic()
{
// 	if(m_pParent != NULL)
// 	{
// 		if(m_pParent->m_wMainWin!=NULL)
// 		{
// 			WinVisView * pView = m_pParent->m_pVisMain->m_wVisView;
// 			CSlice * pCurSlice = pView->getcurslice();
// 			if(pCurSlice!=NULL)
// 				return pCurSlice->m_pCount;
// 		}
// 	}
	return NULL;	
}

void WinTinyDraw::drawStatistic()
{
	unsigned int * pCount = NULL;
	pCount = getStatistic();
	if(pCount==NULL)
		return;

	if(m_pParent == NULL)
		return;

	float totalcount = 0;
	for(int i =0; i< SAMPLESIZE; i++)
	{
    	totalcount += pCount[i];
	}

	fl_color(FL_RED);
	fl_line_style(FL_SOLID, 2);
	fl_begin_line();
	
	float width = w();
	float step = width / ( SAMPLESIZE-1);
	for(int i = 0; i < SAMPLESIZE; i++)
	{
    	float count = pCount[i];
		float height = h();
		float th = height *(count / totalcount) ;
		fl_vertex(i*step+x(),y()+h()-th);		
	}
	fl_end_line();
}

void WinTinyDraw::ReadColors()
{
	m_colors.clear();
	m_colorpos.clear();

	vector<MyDragButton *>::iterator pb;
	
	typedef std::set<int, std::less<int> > int_set;
	int_set pos;

	for(pb=m_pParent->m_colors.begin(); pb!=m_pParent->m_colors.end(); pb ++)
	{
		int colorpos = (*pb)->x(); // between 0 and 1
		pos.insert(colorpos);
	}

	int_set::iterator iter;
	for(iter  = pos.begin(); iter != pos.end(); iter++)
	{
    	int colorpos = *iter;
		m_colorpos.push_back ( (int)( (FindPos(colorpos))*w()) ); // 10 is the m_bRuller->x();
		m_colors.push_back(FindColor(colorpos));
	}
}

float WinTinyDraw::FindPos(int pos)
{
	vector<MyDragButton *>::iterator pb;
	for(pb=m_pParent->m_colors.begin(); pb!=m_pParent->m_colors.end(); pb ++)
	{
		if((*pb)->x() == pos)
		{
			return (*pb)->pos();
		}	    			
	}
 	return 0;
}

cxColor WinTinyDraw::FindColor(int pos)
{
	vector<MyDragButton *>::iterator pb;
	for(pb=m_pParent->m_colors.begin(); pb!=m_pParent->m_colors.end(); pb ++)
	{
		if((*pb)->x() == pos)
		{
			  uchar r,g,b;
  			  Fl::get_color((*pb)->color(),r,g,b);	
			  return cxColor::cxColor(((float) r)/255.0,((float) g)/255.0, ((float)b)/255.0);
		}	    			
	}

	return cxColor::cxColor(0,0, 0);		
}

void WinTinyDraw::updatecolor()
{
	ReadColors();
	redraw();
	updateview();
}

void WinTinyDraw::drawIsoValue( )
{
	if(m_pParent == NULL)
		return;
	float min = m_pParent -> m_min;
	float max = m_pParent -> m_max;
	float isovalue = m_pParent->m_curisovalue;

	if (isovalue < min)
		return;
	if ( isovalue > max)
		return;

	float width = w();
	float start = width * isovalue /(max - min);

	fl_color(FL_GREEN);
	fl_line_style(FL_SOLID, 1);
 	fl_begin_line();
	fl_line((int)start+x(),0+y(),(int) start+x(), h()+y());
	fl_end_line();	

}

void WinTinyDraw::initline(bool is_erase)
{
    int tx = Fl::event_x();
    int ty = Fl::event_y();
    
    int index = tx - x();
    
    if (is_erase) {
        m_line[index] = (float) h();
    } else {
        int height = ty - y();
        if(height < 0) height = 0;
        if(height > h()) height = h();
        m_line[index] = (float)height;   
    }
    
    m_mousex = tx;
    m_mousey =ty;
    
}
