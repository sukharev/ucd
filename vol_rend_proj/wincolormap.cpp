/***************************************************************************
                          wincolormap.cpp  -  description
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

#include "wincolormap.h"
#include "Fl_Color_Chooser.cxx"
#include <fstream>
#include "winvisview.h"
#include "cxvolume.h"
#include <FL/gl.h>
#include <FL/glut.H>
#include <GL/glu.h>

#define TEXTSIZE 12

using namespace std;

WinColorMap::WinColorMap(int x, int y, int w, int h, cxVolume * pVolume){
	m_pVolume =  pVolume;
	m_colors.clear();
    m_nTexname_Colormap = 0;   
	GenWindow(x,y, w,h);
}

WinColorMap::~WinColorMap(){
}

void WinColorMap::show(int argc, char **argv){
	  m_wMainWin->show(argc,argv);
}

void WinColorMap::GenWindow(int x, int y, int w, int h)
{
	m_wMainWin = new Fl_Window(x, y,w,h, "Color Map");
 	m_wMainWin->user_data((void*)(this));
  
    int delta = 0;
#ifdef HISTOGRAM
    m_wHistogram = new WinHistogram(10, 5, w - 25, 100);
    m_wHistogram->box(FL_UP_BOX);
    delta = 105;
#endif
	
    m_wTinyDraw = new WinTinyDraw(10, 5 + delta, w-25, 100);
	m_wTinyDraw->setparent(this);
	m_wTinyDraw->box(FL_UP_BOX);
    

	Fl_Box * box = new Fl_Box(1, 5 + delta, 10, 10, "1");
	box = new Fl_Box(1, 90 + delta, 10, 10, "0");

	m_bMin = new Fl_Box(5, 105 + delta, 40,10, "min");
	m_bMax = new Fl_Box(w-50, 105 + delta, 40,10,"max");

  
	m_gFrm = new Fl_Group (1, m_wTinyDraw->y()+m_wTinyDraw->h()+10, w-1, 15);
	m_gFrm->user_data((void*)(this));
	m_gFrm->box(FL_FLAT_BOX);
  
	m_bRuler = new MyRuler (m_wTinyDraw->x(), m_wTinyDraw->y()+m_wTinyDraw->h()+ 10, m_wTinyDraw->w(),10);
	m_bRuler->m_pColorMap = this;
  

    m_gFrm->end();

    m_cColorChooser = new Fl_Color_Chooser(10, m_bRuler->y()+m_bRuler->h()+5, w-25, 100);
    m_cColorChooser->rgb(1,1,1);
    
    m_wMainWin->end();
    m_wMainWin->resizable(m_wMainWin);
    m_wMainWin->size_range(200,200);
    
    LoadColorFromFile();
}

void WinColorMap::SetMinMax(float min, float max)
{
	m_min = min;
	m_max = max;
	
	static char tmin[10];
	sprintf(tmin, "%.2f", m_min);
	m_bMin->label(tmin);

	static char tmax[10];
	sprintf(tmax,"%.2f", m_max);
	m_bMax->label(tmax);
}

void WinColorMap::reset()
{
	vector<MyDragButton *>::iterator pv;
	for(pv=m_colors.begin(); pv!=m_colors.end();pv++)
	{
			(*pv)->hide();
	}
	m_gFrm->redraw();
	m_colors.clear();

	m_wTinyDraw->updatecolor();
	m_wTinyDraw->reset();
	m_wTinyDraw->updateview();

	m_curisovalue = m_min - 1;
}


void WinColorMap::GenTexColorMap()
{
    float *image = new float[256 * 4];

    for(int i=0; i< 256; i++)
    {
        int factor=(int)(((float)i) / 256.0 * m_wTinyDraw->w());
        cxColor color = m_wTinyDraw->getcolor(factor);
        
        image[i*4+0] = color.r();
        image[i*4+1] = color.g();
        image[i*4+2] = color.b();
        image[i*4+3] = color.a();
    }
    
    glDeleteTextures(1, &m_nTexname_Colormap);
    glGenTextures(1, &m_nTexname_Colormap);
        
    glBindTexture(GL_TEXTURE_2D,m_nTexname_Colormap);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, 256, 1, GL_RGBA, GL_FLOAT, image);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1,0, GL_RGBA, GL_FLOAT, image);
}

cxColor WinColorMap::GetColor(float value)
{
	int factor =(int)((value - m_min) / (m_max - m_min) * m_wTinyDraw->w());
	return m_wTinyDraw->getcolor(factor);
}


void WinColorMap::SaveColor()
{
    assert(m_pVolume != NULL);    
    assert(m_pVolume->m_pVisStatus != NULL);
    
    m_pVolume->m_pVisStatus->m_vTFButtonSettings.clear();
    m_pVolume->m_pVisStatus->m_vTFLineSettings.clear();
    
    for(int i=0; i< TF_SIZE; i++)
    {
        int factor=(int)(((float)i) / TF_SIZE * m_wTinyDraw->w());
        cxColor color = m_wTinyDraw->getcolor(factor);
        m_Colors[i] = color;
        m_Values[i] = color.a();
    
        m_pVolume->m_pVisStatus->m_Colors[i] = color;        
    }

    char filename[1024];

#ifdef MULTI_VARIABLES
    sprintf(filename, "ctrlTF%d.cfg", m_pVolume->m_nVolumeID);
#else
    sprintf(filename, "ctrlTF.cfg");
#endif

    ofstream outf;
    
    outf.open(filename, ios::binary);
    
    int size = m_colors.size();
    outf.write(reinterpret_cast<const char*>(&size), sizeof(int));
    float value[4];   
    for ( int i = 0; i < size; i++)
    {
        value[0] = m_colors[i]->x();
        value[1] = m_colors[i]->m_r;
        value[2] = m_colors[i]->m_g;
        value[3] = m_colors[i]->m_b;
        
        outf.write(reinterpret_cast<const char*>(value), sizeof(float) * 4);
        
        m_pVolume->m_pVisStatus->m_vTFButtonSettings.push_back(value[0]);
        m_pVolume->m_pVisStatus->m_vTFButtonSettings.push_back(value[1]);
        m_pVolume->m_pVisStatus->m_vTFButtonSettings.push_back(value[2]);
        m_pVolume->m_pVisStatus->m_vTFButtonSettings.push_back(value[3]);
    }
    
    float pos[2];
    size = m_wTinyDraw->m_line.size();
    outf.write(reinterpret_cast<const char*>(&size), sizeof(int));
    map<int, float>::iterator iter;
    for ( iter = m_wTinyDraw->m_line.begin(); iter != m_wTinyDraw->m_line.end(); iter++)
    {
        outf.write(reinterpret_cast<const char*>(&(iter->first)), sizeof(int));
        outf.write(reinterpret_cast<const char*>(&(iter->second)), sizeof(float));
        
        pos[0] = iter->first;
        pos[1] = iter->second;
        
        m_pVolume->m_pVisStatus->m_vTFLineSettings.push_back(pos[0]);
        m_pVolume->m_pVisStatus->m_vTFLineSettings.push_back(pos[1]);
    }
    outf.close();
    
    
    m_pVolume->ReDraw();

}

void WinColorMap::LoadColorFromFile()
{
	m_colors.clear();
	
     char filename[1024];

#ifdef MULTI_VARIABLES
    sprintf(filename, "ctrlTF%d.cfg", m_pVolume->m_nVolumeID);
#else
    sprintf(filename, "ctrlTF.cfg");
#endif
   
	ifstream inf(filename, ios::binary);
	
	if (!inf)
		return;
		
	int size;
	inf.read(reinterpret_cast<char *>(&size), sizeof(int));
	
	m_gFrm->begin();
	
	for ( int i = 0; i < size; i++)
	{
		float x, r, g, b;
		
		inf.read(reinterpret_cast<char*>(&x), sizeof(float));
		inf.read(reinterpret_cast<char*>(&r), sizeof(float));
		inf.read(reinterpret_cast<char*>(&g), sizeof(float));
		inf.read(reinterpret_cast<char*>(&b), sizeof(float));
		
		MyDragButton * bButton = new MyDragButton (x, m_bRuler->y(), m_bRuler->h(), m_bRuler->h());
		bButton->m_r = (unsigned char)(r);
		bButton->m_g = (unsigned char)(g);
		bButton->m_b = (unsigned char)(b);
		bButton->color(fl_rgb_color(bButton->m_r, bButton->m_g, bButton->m_b));
		bButton->set(m_bRuler->x(), m_bRuler->x() + m_bRuler->w());
		bButton->setdraw(m_wTinyDraw);
		
		m_colors.push_back(bButton);
	}
	inf.read(reinterpret_cast<char *>(&size), sizeof(int));
	for ( int i = 0; i < size; i++)
	{
		int id;
		float h;
		inf.read(reinterpret_cast<char*>(&id), sizeof(int));
		inf.read(reinterpret_cast<char*>(&h), sizeof(float));
		m_wTinyDraw->m_line[id] = h;
	}
	inf.close();
	
	//m_gFrm->redraw();

	m_gFrm->end();

	m_wTinyDraw->ReadColors();
	//m_wTinyDraw->redraw();
    
    for(int i=0; i< TF_SIZE; i++)
    {
        int factor=(int)(((float)i) / TF_SIZE * m_wTinyDraw->w());
        cxColor color = m_wTinyDraw->getcolor(factor);
        m_Colors[i] = color;
        m_Values[i] = color.a();
    }
    
    SaveColor();
}


void WinColorMap::LoadColorFromVolume()
{
    assert(m_pVolume != NULL);    
    assert(m_pVolume->m_pVisStatus != NULL);
    
    int size = m_colors.size();
    
    for ( int i = 0; i < size; i++)
        m_colors[i]->hide();
        
    m_colors.clear();
            
    size = m_pVolume->m_pVisStatus->m_vTFButtonSettings.size();
    
    m_gFrm->begin();
    
    for ( int i = 0; i < size; i+=4)
    {
        float x, r, g, b;
        
        x = m_pVolume->m_pVisStatus->m_vTFButtonSettings[i];
        r = m_pVolume->m_pVisStatus->m_vTFButtonSettings[i + 1];
        g = m_pVolume->m_pVisStatus->m_vTFButtonSettings[i + 2];
        b = m_pVolume->m_pVisStatus->m_vTFButtonSettings[i + 3];
        
        MyDragButton * bButton = new MyDragButton (x, m_bRuler->y(), m_bRuler->h(), m_bRuler->h());
        bButton->m_r = (unsigned char)(r);
        bButton->m_g = (unsigned char)(g);
        bButton->m_b = (unsigned char)(b);
        bButton->color(fl_rgb_color(bButton->m_r, bButton->m_g, bButton->m_b));
        bButton->set(m_bRuler->x(), m_bRuler->x() + m_bRuler->w());
        bButton->setdraw(m_wTinyDraw);
        
        m_colors.push_back(bButton);
    }
    
    size = m_pVolume->m_pVisStatus->m_vTFLineSettings.size();
    m_wTinyDraw->m_line.clear();
    for ( int i = 0; i < size; i+=2)
    {
        int id = (int)(m_pVolume->m_pVisStatus->m_vTFLineSettings[i]);
        float h = m_pVolume->m_pVisStatus->m_vTFLineSettings[i+1];
        
        m_wTinyDraw->m_line[id] = h;
    }
    
    //m_gFrm->redraw();

    m_gFrm->end();

    m_wTinyDraw->ReadColors();
        
    for(int i=0; i< TF_SIZE; i++)
    {
        int factor=(int)(((float)i) / TF_SIZE * m_wTinyDraw->w());
        cxColor color = m_wTinyDraw->getcolor(factor);
        m_Colors[i] = color;
        m_Values[i] = color.a();
    }
    
    m_gFrm->redraw();
    m_wTinyDraw->redraw();
}


inline void WinColorMap::cb_bReset_i(Fl_Button* o, void* v)
{
	reset();
}

void WinColorMap::cb_bReset(Fl_Button* o, void* v) {
  ((WinColorMap*)(o->parent()->user_data()))->cb_bReset_i(o,v);
}

inline void WinColorMap::cb_bAdd_i(Fl_Button* o, void* v)
{
  MyColorButton * pButton = new MyColorButton(0,0,10,20);
  int count = m_wTinyDraw->m_colorpos.size();
  for(int i=0; i< count; i++)
  {
     pButton->m_colorpos.push_back(m_wTinyDraw->m_colorpos[i]);
     pButton->m_colors.push_back(m_wTinyDraw->m_colors[i]);
  }

  m_pPack->insert(*pButton, m_pPack->children());
  m_pPack->redraw();
  m_sScroll->redraw();    
}

void WinColorMap::cb_bAdd(Fl_Button* o, void* v) {
  ((WinColorMap*)(o->parent()->user_data()))->cb_bAdd_i(o,v);
}

inline void WinColorMap::cb_bDel_i(Fl_Button* o, void* v)
{

}

void WinColorMap::cb_bDel(Fl_Button* o, void* v) {
  ((WinColorMap*)(o->parent()->user_data()))->cb_bDel_i(o,v);
}


void MyRuler::draw()
{
  fl_clip(x(),y(),w(),h());
  fl_color(FL_BLACK);
  fl_push_matrix();

  fl_line_style(FL_SOLID, 1);
  fl_begin_line();
  
  fl_vertex(x(),y()+h()/2);
  fl_vertex(x()+w(),y()+h()/2);
  
  fl_end_line();
  
  fl_pop_matrix();
  fl_pop_clip();
}

int MyRuler::handle(int event)
{
  switch(event){
  case FL_PUSH:
      if(Fl::event_button1())
        AddColor(Fl::event_x());
      return Fl_Widget::handle(FL_PUSH);
  default:
      return Fl_Widget::handle(event);
  }
}

void MyRuler::AddColor(int xpos)
{
	if(m_pColorMap == NULL)
		return;

	m_pColorMap->m_gFrm->begin();

	MyDragButton * bButton = new MyDragButton (xpos, y(), h(), h());
	bButton->m_r = (unsigned char)(m_pColorMap->m_cColorChooser->r()*255);
	bButton->m_g = (unsigned char)(m_pColorMap->m_cColorChooser->g()*255);
	bButton->m_b = (unsigned char)(m_pColorMap->m_cColorChooser->b()*255);
	
	bButton->color(fl_rgb_color(bButton->m_r, bButton->m_g, bButton->m_b));
	
	bButton->set(x(), x() + w());
	bButton->setdraw(m_pColorMap->m_wTinyDraw);

	m_pColorMap->m_colors.push_back(bButton);
	m_pColorMap->m_gFrm->redraw();

	m_pColorMap->m_gFrm->end();

	m_pColorMap->m_wTinyDraw->ReadColors();
    m_pColorMap->m_wTinyDraw->updatecolor();      
	m_pColorMap->m_wTinyDraw->redraw();
 
	redraw();
}

void MyColorButton::draw()
{
	fl_clip(x(),y(),w(),h());
	fl_color(FL_DARK3);
	fl_push_matrix();
	
	for(int i = 0; i< w(); i++)
	{
		cxColor color = getcolor(i);
		fl_color(((int)(color.r() * 255)), ((int) (color.g() * 255)), ((int)( color.b() * 255)));
		fl_line(x()+i, y(), x()+ i , y()+ h());
	}
	
	fl_pop_matrix();
	fl_pop_clip();
  
//    Fl_Button::draw();  
}

cxColor MyColorButton::getcolor(int pos)
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
				float r = start.r()*(1-tv) + end.r()*tv;
				float g = start.g()*(1-tv) + end.g()*tv;
				float b = start.b()*(1-tv) + end.b()*tv;
		
				return cxColor::cxColor(r,g,b,1);
			}
		}
	}
	return cxColor::cxColor(0,0,0,0);
}
