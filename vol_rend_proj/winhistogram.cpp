//
// C++ Implementation: winhistogram
//
// Description: 
//
//
// Author: Hongfeng Yu <hfyu@ucdavis.edu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "winhistogram.h"

void WinHistogram::draw()
{
    if (!m_pImage)
        return;
        
    Fl::visual(FL_RGB);        
    
    fl_draw_image(m_pImage, x(), y(), w(), h());
}

