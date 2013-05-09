/*
 *  histogram1D.h
 *  amrview
 *
 *  Created by Ryan Armstrong on 2/15/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */



#include <FL/Fl_Gl_Window.H>
#include <FL/Fl.H>

#ifndef Histogram1D_H
#define Histogram1D_H


class Histogram1D : public Fl_Gl_Window {
	void draw();
	int handle(int);
	
	float zoom;
	int mousex, mousey;
	int resolution;
	long int scalarLength;
	long int * histogram;	
	long int max;
	float * scalars;
	bool okToDraw;
	
	float * colorTable;
	int colorTableSize;
public:
	Histogram1D(int x, int y, int w, int h); 
	
	Histogram1D(int x, int y, int w, int h, 
				float * scalars, long scalarLength); 
	
	Histogram1D(int x, int y, int w, int h, int resolution, 
				long * histArray);
	
	void updateHistogram(float * scalars, long scalarLength);
	void buildHistogram();
	void setColorTable(float * colorTable, long size);	
private:
	
};


#endif
