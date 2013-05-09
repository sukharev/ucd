/*
 *  OpacityDisp.cpp
 *  amrview
 *
 *  Created by Ryan Armstrong on 2/16/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "OpacityDisp.h"

#include <iostream>

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else 
#include <GL/glew.h>
#include <GL/glu.h>
#include <assert.h>
#endif

OpacityDisp::OpacityDisp(int x, int y, int w, int h, int inResolution, float ** inColorTable)
: Fl_Gl_Window(x, y, w, h){
	//	Fl::add_idle(&idle_cp, (void*)this);
	colorTable = *inColorTable;
	assert(colorTable);
	resolution = inResolution;
	assert(resolution > 0);
}


void OpacityDisp::draw() {
	if (!valid()) {

#ifndef __APPLE__
	    GLenum err = glewInit();
	    if (GLEW_OK != err) {
	    	std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
		}
	
	    std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
#endif
		//		... set up projection, viewport, etc ...
		//		... window size is in w() and h().
		//		... valid() is turned on by FLTK after draw() returns
		glViewport(0, 0, w(), h());
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		
		gluOrtho2D(0.0, w(), 0.0, 1.0);
		
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
//		glEnable(GL_LINE_SMOOTH);
	}
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0,0.0,0.0,1.0);
	
	glPushMatrix();
	{
		
		glPushMatrix();
		{
			glLineWidth(1.0);
			for(int i = 0; i < w(); i++) {

				glBegin(GL_LINE_STRIP);
				
				int index = int( ((float)i/w()) * resolution ); // 0->colorTableSize-1
				if(index >= resolution) index = resolution-1;
				
				glColor4f(colorTable[int(index*4)+0], colorTable[int(index*4)+1],
						  colorTable[int(index*4)+2], colorTable[int(index*4)+3]);
				glVertex2f(i,0);
				glVertex2f(i,1);
				//				std::cout << "i:" << i << " " << ((double)histogram[i]) << " / " << (max) << " = " << ((double)histogram[i])/log2(max) << std::endl;
				glEnd();
				
				
				
//				glVertex2f(i, 0);
//				glVertex2f(i, 1);
//			
//			for (int i = 0; i <= (resolution-1); i++){
//				glBegin(GL_LINE_STRIP);
//				glColor4f(colorTable[int(i*4)+0],colorTable[int(i*4)+1],
//						  colorTable[int(i*4)+2], colorTable[int(i*4)+3]); // TODO colors from colorTable
//				glVertex2f((float)i, 0);
//				glVertex2f((float)i, 1);
//				glEnd();
			}

		}
		glPopMatrix(); // for exhibit class
	}
	glPopMatrix(); // for rotation
	
	glFlush();
	
}


int OpacityDisp::handle(int event) {
//	std::cout << "OpacityDisp handle: ";
//	redraw();
	return Fl_Gl_Window::handle(event);
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
	

