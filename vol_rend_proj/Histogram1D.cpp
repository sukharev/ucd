/*
 *  histogram1D.cpp
 *  amrview
 *
 *  Created by Ryan Armstrong on 2/15/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "Histogram1D.h"
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <string.h>

#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <assert.h>
#endif

#include <iostream>

using namespace std;

#include <math.h>





Histogram1D::Histogram1D(int x, int y, int w, int h )
: Fl_Gl_Window(x, y, w, h){
	//	Fl::add_idle(&idle_cp, (void*)this);
	resolution = w;
	//histogram = new long[resolution]; 
	histogram = 0;
	max = 0;
	resolution = w;
	colorTable = 0;
	scalarLength = 0;
}


Histogram1D::Histogram1D(int x, int y, int w, int h, float * inScalars, long inScalarLength)
: Fl_Gl_Window(x, y, w, h){
	//	Fl::add_idle(&idle_cp, (void*)this);
	resolution = w;
	//histogram = new long[resolution]; 
	histogram = 0;
	max = 0;
	resolution = w;
	scalarLength = inScalarLength;
	scalars = inScalars;
	colorTable = 0;
	buildHistogram();
}


Histogram1D::Histogram1D(int x, int y, int w, int h, int resolution, long * histArray)
: Fl_Gl_Window(x, y, w, h){
	//	Fl::add_idle(&idle_cp, (void*)this);
	colorTable = 0;

	
	//TODO: implement this for ease of use, although it may not be practical
}



// call this if you used this constructor: Histogram1D(int x, int y, int w, int h )
void Histogram1D::updateHistogram(float * inScalars, long inScalarLength)
{
	scalarLength = inScalarLength;
	scalars = inScalars;
	buildHistogram();
}



void Histogram1D::buildHistogram() 
{
	if(histogram) {
		delete []histogram;
	}
	
	
	histogram = new long[resolution];
	assert(histogram);
	
	memset(histogram,0,resolution*sizeof(long));
	std::cout << scalarLength << std::endl;
	//scalarLength=resolution; //JS
	for(int i = 0; i < scalarLength; i++) {
		//if(scalars[i] <= 1)
		//float value = scalars[i];
		// assumes scalars are between 0 and 1;
		if(scalars[i] >= 1.0){
			scalars[i] = 1.0;
		}
		if(scalars[i] <= 0.0){
			scalars[i] = 0.0;
		}
		assert(scalars[i] <= 1.0 && scalars[i] >= 0.0);
		histogram[(int) ( (float)(scalars[i]) * (float)(resolution-1.0))]++;
	}
	
	max = 0;
		
	for(int i = 0; i < resolution; i++) {
		if(histogram[i] > max) max = histogram[i];
	}
	redraw();
}


void Histogram1D::setColorTable(float * inColorTable, long size)
{
	colorTable = inColorTable;
	colorTableSize = size;
	
}



void Histogram1D::draw() {
	if (!valid()) {
#ifndef __APPLE__
	  	GLenum err = glewInit();
	    	if (GLEW_OK != err) {
	    	cerr << "Error: " << glewGetErrorString(err) << endl;
		}

	    	cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;
#endif
		//		... set up projection, viewport, etc ...
		//		... window size is in w() and h().
		//		... valid() is turned on by FLTK after draw() returns
		glViewport(0, 0, w(), h());
		std::cout << "Deleting histogram1D and rebuilding..." << std::endl;
		resolution = w();
		buildHistogram();
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		
		gluOrtho2D(0.0, w(), 0.0, 1.0);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);


	}
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0,0.0,0.0,1.0);
	if(histogram) {
		glPushMatrix();
		{
			
			glTranslatef(0.0f, 0.0f, -1.0);
			glPushMatrix();
			{
				glDisable(GL_LINE_SMOOTH);
				glLineWidth(1.0);
				
				// using log scale
				//			std::cout << std::endl;
				for(int i = 0; i < w(); i++) {
					glBegin(GL_LINE_STRIP);
					
					int index = (int)(((float)i/w()) * colorTableSize); // 0->colorTableSize-1
					if(index >= colorTableSize) index = colorTableSize-1;
					
					glColor4f(colorTable[int(index*4)+0], colorTable[int(index*4)+1],
							  colorTable[int(index*4)+2], colorTable[int(index*4)+3]);
					glVertex2f(i,0);
					glVertex2f(i, log((float)histogram[i])/log((float)max));
					//				std::cout << "i:" << i << " " << ((double)histogram[i]) << " / " << (max) << " = " << ((double)histogram[i])/log2(max) << std::endl;
					glEnd();
					
				}
				
				glColor4f(0,0,0,.6);
				glLineWidth(3);
				glBegin(GL_LINE_STRIP);
				for(int i = 0; i < w(); i++) {
					glVertex2f(i, log((float)histogram[i])/log((float)max)-.01);
				}
				glEnd();
				
				glColor4f(1,1,1,1);
				glLineWidth(1.5);
				glEnable(GL_LINE_SMOOTH);
				glBegin(GL_LINE_STRIP);
				for(int i = 0; i < w(); i++) {
					glVertex2f(i, log((float)histogram[i])/log((float)max));
				}
				glEnd();
				
			}
			glPopMatrix(); // for exhibit class
		}
		glPopMatrix(); // for rotation
	}
//	glFlush();
	
}


int Histogram1D::handle(int event) {
	//	std::cout << "Histogram1D handle: ";
	switch(event) {
		case FL_PUSH:
			//			... mouse down event ...
			//			... position in Fl::event_x() and Fl::event_y()
			
			if( Fl::event_button1() ) // LEFT MOUSE
			{
			}
			if( Fl::event_button2() ) // MIDDLE MOUSE
			{
			}
			if( Fl::event_button3() ) // RIGHT MOUSE
			{
				mousex = Fl::event_x();
				mousey = Fl::event_y();
			}
			
			return 1;
		case FL_DRAG:
			//			... mouse moved while down event ...
			
			if( Fl::event_button1() ) // LEFT MOUSE
			{
			}
			
			if( Fl::event_button2() ) // MIDDLE MOUSE
			{
			}
			
			if( Fl::event_button3() ) // RIGHT MOUSE
			{
				mousex = Fl::event_x();
				mousey = Fl::event_y();
			}
			
			redraw();
			return 1;
		case FL_RELEASE:    
			//			... mouse up event ...
			return 1;
		case FL_FOCUS :
		case FL_UNFOCUS :
			//			... Return 1 if you want keyboard events, 0 otherwise
			return 1;
		case FL_KEYBOARD:
			//			... keypress, key is in Fl::event_key(), ascii in Fl::event_text()
			//			... Return 1 if you understand/use the keyboard event, 0 otherwise...
			std::cout << "Keyboard@Histogram1D: " << Fl::event_key() << ", " << Fl::event_text() << std::endl;
			switch (Fl::event_key())
			{
			}
				return 0;
		case FL_SHORTCUT:
			//			... shortcut, key is in Fl::event_key(), ascii in Fl::event_text()
			//			... Return 1 if you understand/use the shortcut event, 0 otherwise...
			return 0;
		default:
			// pass other events to the base class...
			return Fl_Gl_Window::handle(event);
	}
}


// Idle function callback
//void idle_cp(void* pThis)
//{
//	if (pThis != NULL)
//	{
//		TransferFunc1D* win = reinterpret_cast<TransferFunc1D*>(pThis);
//
//	}
//}
	

