/*
 *  transferFunc1D.h
 *  amrview
 *
 *  Created by Ryan Armstrong on 2/15/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef TRANSFERFUNC1D_H
#define TRANSFERFUNC1D_H


#include <vector>

#include "OpacityDisp.h"
#include "Histogram1D.h"

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <FL/Fl_Gl_Window.H>
#include <FL/Fl.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Button.H>

using namespace std;
#include <fstream>


class cxVolume;
class ColorChooser;
class ColorNode;

class TransferFunc1D : public Fl_Gl_Window {
	void draw();
	int handle(int);
	bool drawingLine;
	bool movingNode;
	
	cxVolume *m_pVolume;
	
	
	float mousex, mousey;
	float prevMousex, prevMousey;
	int resolution;
	float * colorTable;
	OpacityDisp * opacityDisp;
	Histogram1D * histogram1D;
public:
	TransferFunc1D(int x, int y, int w, int h, int resolution, cxVolume * pVolume);
	virtual ~TransferFunc1D();
	
	// sets
	void setOpacityDisp(OpacityDisp ** inOpacityDisp);
	void setHistogram1D(Histogram1D ** inHistogram1D);
	void setDefaults();
	void setCorrDefaults();
	
	
	// gets
	float * getColorTablePtr() { return colorTable; }
	int getResolution() { return resolution; }
	
	// file i/o for state saving and loading
	void loadTransFuncFromFile(std::ifstream &inFile);
	void saveTransFuncToFile(std::ofstream &outFile);
	
	void SaveColor();
	void LoadColorFromFile();
	
private:
	void reinterpolate(int newWidth);
	void updateColorTable();
	void drawBackground();
		
	void fillGap(int mode);
	int Max(int a, int b);
	int Min(int a, int b);	
	
	friend class ColorChooser;
	ColorChooser * colorChooser;

	Fl_Button * buttonA;
	
	std::vector< ColorNode > colorNodes; // vector of user defined color nodes
	std::vector< float > alphaNodes;
	
	int selectedColorNode;
	
	// UI CALLBACKS
	friend void buttonA_cb(Fl_Button *, void *);
	friend void colorChooser_cb(ColorChooser *, void *);

	
};




class ColorChooser : public Fl_Color_Chooser {
public:
	ColorChooser(int x, int y, int w, int h, TransferFunc1D **inTransFunc);
private:
	TransferFunc1D *transFunc;
	
	friend void colorChooser_cb(ColorChooser *, void *);

	
	
	
};

class ColorNode {
public:
	float r, g, b;
	int index;
	float xpos;
	//bool selected;
	bool leftClicked, rightClicked;
	ColorNode(float inXpos, float r, float g, float b){ 
		this->xpos = inXpos;
		this->r = r; 
		this->g = g; 
		this->b = b; 
	//	selected = false;
	}
	void draw(float width, float height, bool selected);
};

#endif
