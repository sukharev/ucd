//
// C++ Interface: winhistogram
//
// Description: 
//
//
// Author: Hongfeng Yu <hfyu@ucdavis.edu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WINHISTOGRAM_H
#define WINHISTOGRAM_H

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Image.H>

/**
	@author Hongfeng Yu <hfyu@ucdavis.edu>
*/
class WinHistogram : public Fl_Widget  
{
public:
    WinHistogram(int X,int Y,int W,int H) : Fl_Widget(X,Y,W,H) {m_pImage = new uchar[W * H * 3];}
    
public:
    uchar *m_pImage;   
     
private:
    void draw();
};

#endif
