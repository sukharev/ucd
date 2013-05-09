/*
 *  OpacityDisp.h
 *  amrview
 *
 *  Created by Ryan Armstrong on 2/16/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */


#include <FL/Fl_Gl_Window.H>
#include <FL/Fl.H>

#ifndef OpacityDisp_H
#define OpacityDisp_H



class OpacityDisp : public Fl_Gl_Window {
	void draw();
	int handle(int);
	
	float zoom;
	int mousex, mousey;
	float * colorTable;	
	int resolution;
	
public:
		// resolution should be (#rgba entries) and is used as length of rgbaArray
		OpacityDisp(int x, int y, int w, int h, int resolution, 
					float ** colorTable); 
	
private:
		
};


#endif
