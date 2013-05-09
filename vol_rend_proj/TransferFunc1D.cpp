/*
 *  transferFunc1D.cpp
 *  amrview
 *
 *  Created by Ryan Armstrong on 2/15/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */


#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#endif


//#include "controlWin.h"
#include "TransferFunc1D.h"
#include "cxvolume.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include <assert.h>
#include <limits.h>

#define NODE_HEIGHT 15
#define NODE_WIDTH  20
#define COLOR_CHOOSER_WIDTH  190
#define COLOR_CHOOSER_HEIGHT 100
#define MARGIN 15
#define BUTTON_HEIGHT 25
#define BUTTON_WIDTH  100


	void buttonA_cb(Fl_Button *, void *);
	void colorChooser_cb(ColorChooser *, void *);

TransferFunc1D::TransferFunc1D(int x, int y, int w, int h, int inResolution, cxVolume * pVolume) : Fl_Gl_Window(x, y, w, h-COLOR_CHOOSER_HEIGHT){

	mode(FL_ALPHA | FL_DEPTH | FL_DOUBLE | FL_RGB8 );
	//	Fl::add_idle(&idle_cp, (void*)this);
	opacityDisp = 0;
	histogram1D = 0;
	
	m_pVolume = pVolume;
	resolution = inResolution;
	colorTable = new float[resolution * 4]; // RGBA
	memset(colorTable, 0, resolution*4);
	selectedColorNode = -1;
	alphaNodes.resize(w);
	//std::cout << alphaNodes.size() << std::endl;

	setDefaults();

	TransferFunc1D *ihateptrs = this; // this is necessary because &this doesnt work.
	colorChooser = new ColorChooser(x, y+(h-COLOR_CHOOSER_HEIGHT)+MARGIN, COLOR_CHOOSER_WIDTH, COLOR_CHOOSER_HEIGHT-1,&ihateptrs);
	colorChooser->resizable(NULL);
	
	buttonA = new Fl_Button(x+MARGIN+COLOR_CHOOSER_WIDTH, y+(h-COLOR_CHOOSER_HEIGHT)+MARGIN, BUTTON_WIDTH,BUTTON_HEIGHT, "Reset");
	buttonA->callback((Fl_Callback*)buttonA_cb, this);
	//buttonA->resizable(0);
		
	updateColorTable();
}


TransferFunc1D::~TransferFunc1D(){
	delete [] colorTable;
}


// file i/o for state saving and loading
void TransferFunc1D::loadTransFuncFromFile(std::ifstream &inFile) {
	cout << "TransferFunc1D::loadTransFuncFromFile" << endl;
	assert(inFile != 0);
	
	long colorNodesSize;
	long alphaNodesSize;
	float xpos;
	float r;
	float g;
	float b;
	float a;
	
	colorNodes.clear();
	alphaNodes.clear();
	
	inFile >> colorNodesSize;
	assert(colorNodesSize > 0);
	
	for (int i = 0; i < colorNodesSize; i++) {
		inFile >> xpos >> r >> g >> b;
		colorNodes.push_back(ColorNode(xpos,r,g,b));
	}

	inFile >> alphaNodesSize;

	for (int i = 0; i < alphaNodesSize; i++) {
		inFile >> a;
		alphaNodes.push_back(a);
	}
	
	updateColorTable();
}


void TransferFunc1D::saveTransFuncToFile(std::ofstream &outFile) {
	cout << "TransferFunc1D::saveTransFuncToFile" << endl;
	assert(outFile != 0);
	
	outFile << colorNodes.size() << " ";
	for (unsigned int i = 0; i < colorNodes.size(); i++) {
		outFile << colorNodes[i].xpos << " ";
		outFile << colorNodes[i].r << " ";
		outFile << colorNodes[i].g << " ";
		outFile << colorNodes[i].b << " ";
	}
	outFile << alphaNodes.size() << " ";
	for (unsigned int i = 0; i < alphaNodes.size(); i++) {
		outFile << alphaNodes[i] << " ";
	}
	
}


// reset the colormap
void TransferFunc1D::setDefaults() {
	
	colorNodes.clear();
	
	// these first two are the endpoints and are important.
	// they should not be deleted or moved, this is necessary for interpolation
	colorNodes.push_back(ColorNode(0.0, 0.0,0.0,1.0));
	colorNodes.push_back(ColorNode(1.0, 1.0,0.0,0.0));
	// end of important nodes
	
	colorNodes.push_back(ColorNode(1.0/4.0, 0.0,1.0,1.0));
	colorNodes.push_back(ColorNode(1.0/2.0, 0.0,1.0,0.0));
	colorNodes.push_back(ColorNode(3.0*1.0/4.0, 1.0,1.0,0.0));
	
	alphaNodes.clear();
	alphaNodes.resize(w());
	
	updateColorTable();
}


void TransferFunc1D::setCorrDefaults() {
	
	colorNodes.clear();
	float fact = 1.0;//0.02;
	// these first two are the endpoints and are important.
	// they should not be deleted or moved, this is necessary for interpolation
	colorNodes.push_back(ColorNode(0.0, 0.8* fact, 0.0,1.0* fact));
	colorNodes.push_back(ColorNode(1.0, 1.0* fact,0.0,0.0));
	// end of important nodes
	
	//colorNodes.push_back(ColorNode(0.0,0.0,0.75*fact,1.0));
	colorNodes.push_back(ColorNode(1.0/7.0,0.0,0.0,1.0*fact));
	colorNodes.push_back(ColorNode(2.0/7.0,0.0,1.0*fact,1.0*fact));
	colorNodes.push_back(ColorNode(3.0/7.0,1.0*fact,1.0*fact,1.0*fact));
	colorNodes.push_back(ColorNode(4.0/7.0,1.0*fact,1.0*fact,0.0));
	colorNodes.push_back(ColorNode(5.0/7.0,1.0*fact,0.75*fact,0.0));
	colorNodes.push_back(ColorNode(6.0/7.0,1.0*fact,0.5*fact,0.0));
	
	alphaNodes.clear();
	alphaNodes.resize(w());
	
	updateColorTable();
}

void TransferFunc1D::setOpacityDisp(OpacityDisp ** inOpacityDisp){
	opacityDisp = *inOpacityDisp;
}

void TransferFunc1D::setHistogram1D(Histogram1D ** inHistogram1D){
	histogram1D = *inHistogram1D;
}

bool cmp(const ColorNode *a, const ColorNode *b)
{
	std::cout << "distance comparison: " << a->xpos << " " << b->xpos << std::endl;
	return a->xpos > b->xpos;
}

void TransferFunc1D::drawBackground(){
	int nextNode = 0;
	int nextIndex = 0;
	float x1, x2;
	// draw color box

	
	for(unsigned int j = 0; j < colorNodes.size(); j++) {
		colorNodes[j].index = int( colorNodes[j].xpos*(resolution-1) );
	}
	
	for(unsigned int node = 0; node < colorNodes.size(); node++) {
		nextIndex = INT_MAX;
		for(unsigned int next = 0; next < colorNodes.size(); next++){
			if(colorNodes[next].index > colorNodes[node].index){
				if(colorNodes[next].index < nextIndex){
					nextIndex = colorNodes[next].index;
					nextNode = next;
				}
			}
		}
		
		if (nextIndex != INT_MAX){
			x1 = (float)colorNodes[node].xpos;
			if(colorNodes[nextNode].index == resolution-1){
				x2 = 1;
			} else {
				x2 = colorNodes[nextNode].xpos;
			}
			
			glBegin(GL_QUADS);
			{// solid non-alpha lower half
				glColor4f(colorNodes[node].r, colorNodes[node].g,
						  colorNodes[node].b, 1);
				glVertex2f(x1,0);
				glVertex2f(x1,1);
				glColor4f(colorNodes[nextNode].r, colorNodes[nextNode].g,
						  colorNodes[nextNode].b, 1);
				glVertex2f(x2,1);
				glVertex2f(x2,0);
			}
			glEnd();
		}
	}
}

void TransferFunc1D::draw() {

	if (!valid()) {

#ifndef __APPLE__
    GLenum err = glewInit();
    if (GLEW_OK != err) {
    	cerr << "Error: " << glewGetErrorString(err) << endl;
	}

    cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;
#endif


		if(alphaNodes.size() < unsigned(w()) ) {
			// reinterpolate into longer alpha array
			reinterpolate(w());
		}
		std::cout << "alphaNodes.size()" << alphaNodes.size() << std::endl;

		//		... set up projection, viewport, etc ...
		//		... window size is in w() and h().
		//		... valid() is turned on by FLTK after draw() returns
		glViewport(0, 0, w(), h());
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		
		gluOrtho2D(0.0, 1.0, 0.0-((float)NODE_HEIGHT/((float)h()-(float)NODE_HEIGHT)), 1.001);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		std::cout << "about to blend" << std::endl;

		// TODO: PUT THIS BACK after segfault is fixed
		glBlendEquation(GL_FUNC_ADD);


		std::cout << "blended!" << std::endl;		
		glEnable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	}
	
	if(	opacityDisp ) opacityDisp->redraw();
	if(	histogram1D ) histogram1D->redraw();
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0,0.0,0.0,1.0);
	
	glPushMatrix();
	{
		
		glTranslatef(0.0f, 0.0f, -1);
	
		glPushMatrix();
		{
			
			drawBackground();
			

			glEnable(GL_BLEND);
			glEnable(GL_LINE_SMOOTH);
			glLineWidth(4.0);
			glColor4f(0.0,0.0,0.0,0.5);
			glBegin(GL_LINE_STRIP);
			{
				for (unsigned int i = 0; i < alphaNodes.size(); i++){
					glVertex2f((float)i/(float)alphaNodes.size(), alphaNodes[i]-1.0/h());
				}
			}
			glEnd();
			
			glColor4f(1.0,1.0,1.0,1.0);
			glLineWidth(2.0);
			glBegin(GL_LINE_STRIP);
			{
				for (unsigned int i = 0; i < alphaNodes.size(); i++){
					glVertex2f((float)i/(float)alphaNodes.size(), alphaNodes[i]);
				}
			}
			glEnd();
			glDisable(GL_LINE_SMOOTH);

			// draw color node background
			glColor3f(.0,.0,.0);
			glBegin(GL_QUADS);
			glVertex2f(0,0);
			glVertex2f(1,0);
			glVertex2f(1,0-((float)NODE_HEIGHT/((float)h()-(float)NODE_HEIGHT)));
			glVertex2f(0,0-((float)NODE_HEIGHT/((float)h()-(float)NODE_HEIGHT)));
			glEnd();
			
			for(unsigned int node = 0; node < colorNodes.size(); node++){
				colorNodes[node].draw(  
					(float)w(), 
					0.0-((float)NODE_HEIGHT/((float)h()-(float)NODE_HEIGHT)),
					(int(node) == selectedColorNode));
			}
		}
		glPopMatrix(); // for exhibit class
	}
	glPopMatrix(); // for rotation
	
	glFlush();

	
}


int TransferFunc1D::handle(int event) {

	
	switch(event) {
		
		case FL_PUSH:
			//			... mouse down event ...
			//			... position in Fl::event_x() and Fl::event_y()
			// must shift coords to make room for color nodes.
			mousex = (float)Fl::event_x() / (float)w();
			mousey = ( (h()-Fl::event_y()) / ((float)h()-(float)NODE_HEIGHT) ) - 
				( (float)NODE_HEIGHT / ((float)h()-(float)NODE_HEIGHT) );
			
			prevMousex = mousex;
			prevMousey = mousey;
			
			// x: 0->.99999 y:.00001->1
			if(mousex >= 0 && mousex <= 1 && mousey >= 0 && mousey <= 1) {
				drawingLine = true;
			} else {
				movingNode = true;
			}
				
			if (drawingLine) { // MODIFY OPACITY LINE
				if( Fl::event_button1() ) // LEFT MOUSE
				{
					if (Fl::event_state(FL_SHIFT) ) { 
						if(alphaNodes[int((float)mousex*((float)alphaNodes.size()-1))] < mousey) {
							alphaNodes[int((float)mousex*((float)alphaNodes.size()-1))] = mousey;
						}
						
					} else if (Fl::event_state(FL_CTRL) ){ 
						if(alphaNodes[int((float)mousex*((float)alphaNodes.size()-1))] > mousey) {
							alphaNodes[int((float)mousex*((float)alphaNodes.size()-1))] = mousey;
						}						
						
					} else {
						
						alphaNodes[int((float)mousex*((float)alphaNodes.size()-1))] = mousey;
						
					}
					// modified the opacity line, so must update
					updateColorTable();
				}
				if( Fl::event_button2() ) // MIDDLE MOUSE
				{
				}
				if( Fl::event_button3() ) // RIGHT MOUSE
				{
					alphaNodes[int((float)mousex*((float)alphaNodes.size()-1))] = 0.0;
					// modified the opacity line, so must update
					updateColorTable();
				}


			} else if (movingNode) { // MOVE COLOR NODES  or CREATE
				std::cout << "Moving node" << std::endl;
				selectedColorNode = -1; // value for no node selected.
				
				// test for selection (is mouse inside a node?)
				for(unsigned int i = 0; i < colorNodes.size(); i++){
					// std::cout << "in loop: " << i << " " << colorNodes[i].xpos << " " << mousex << std::endl;
					if ( (mousex > colorNodes[i].xpos - (NODE_WIDTH/(w()*2.0))) &&
						 (mousex < colorNodes[i].xpos + (NODE_WIDTH/(w()*2.0))) ) 
					{
						selectedColorNode = i;
					}
				}
				
				
				if( Fl::event_button1() ) // LEFT MOUSE
				{
					//a node is selected, so update the colorchooser to this nodes color
					if(selectedColorNode > -1) {
						colorChooser->rgb(colorNodes[selectedColorNode].r, 
										  colorNodes[selectedColorNode].g, 
										  colorNodes[selectedColorNode].b);
					}
					
					// no node is selected, so insert a node at the click location 
					// and update the node and color chooser with the color at 
					// that location in the colortable
					if(selectedColorNode == -1) {
						//updateColorTable();
						colorNodes.push_back(ColorNode(mousex, 
													   colorTable[int(mousex*(resolution-1))*4+0], 
													   colorTable[int(mousex*(resolution-1))*4+1], 
													   colorTable[int(mousex*(resolution-1))*4+2]
													   ));
						selectedColorNode = colorNodes.size()-1;
						
						colorChooser->rgb(colorNodes[selectedColorNode].r, 
										  colorNodes[selectedColorNode].g, 
										  colorNodes[selectedColorNode].b);
					}
				}
				if( Fl::event_button2() ) // MIDDLE MOUSE
				{
				}
				if( Fl::event_button3() ) // RIGHT MOUSE
				{
					if(selectedColorNode > 1 ){ // 0 and 1 are the endpoints!! cant move/delete them
						std::vector<ColorNode>::iterator iter = colorNodes.begin();
						iter += selectedColorNode;
						colorNodes.erase(iter);
						selectedColorNode = -1;
						
						// deleted a node, so must update!
						updateColorTable();

					}
					
				}
			}
			redraw();
			
			return 1;
		case FL_DRAG:
			//			... mouse moved while down event ...
			mousex = (float)Fl::event_x()/(float)w();
			mousey = ((h()-Fl::event_y())/((float)h()-(float)NODE_HEIGHT))-((float)NODE_HEIGHT/((float)h()-(float)NODE_HEIGHT));
			
			//std::cout << Fl::event_x() << " " << Fl::event_y() << std::endl;
			//std::cout << mousex << " " << mousey << " " << h() <<  std::endl;
			
			
			if (drawingLine) { // MODIFY OPACITY LINE
				
				if(mousex < 0) mousex = 0;
				if(mousex > 1) mousex = 1;
				if(mousey < 0) mousey = 0;
				if(mousey > 1) mousey = 1;
				
				int mode = 0;
				if( Fl::event_button1() ) // LEFT MOUSE
				{
					if (Fl::event_state(FL_SHIFT) ) { //== (FL_SHIFT | FL_BUTTON1)) {
						mode = FL_SHIFT;
						if(alphaNodes[int((float)mousex*((float)alphaNodes.size()-1))] < mousey) {
							alphaNodes[int((float)mousex*((float)alphaNodes.size()-1))] = mousey;
						}
					} else if (Fl::event_state(FL_CTRL) ){ //== (FL_CTRL | FL_BUTTON1)) {
						mode = FL_CTRL;						
						if(alphaNodes[int((float)mousex*((float)alphaNodes.size()-1))] > mousey) {
							alphaNodes[int((float)mousex*((float)alphaNodes.size()-1))] = mousey;
						}						
					} else {
						mode = FL_BUTTON1;
						alphaNodes[int((float)mousex*((float)alphaNodes.size()-1))] = mousey;
					}
					
					if(mousex*(float)alphaNodes.size() > prevMousex*(float)alphaNodes.size()+1.0 || 
					   mousex*(float)alphaNodes.size() < prevMousex*(float)alphaNodes.size()-1.0) { // need to fill in array
						fillGap(mode);
					}
					// modified the opacity line, so must update
					updateColorTable();
				}
				
				if( Fl::event_button2() ) // MIDDLE MOUSE
				{
				}
				
				if( Fl::event_button3() ) // RIGHT MOUSE
				{

					alphaNodes[int((float)mousex*((float)alphaNodes.size()-1))] = 0.0;
					if(mousex*(float)alphaNodes.size() > prevMousex*(float)alphaNodes.size()+1.0 || mousex*(float)alphaNodes.size() < prevMousex*(float)alphaNodes.size()-1.0) { // need to fill in array
						fillGap(FL_BUTTON3);
					}
			
					// modified the opacity line, so must update
					updateColorTable();
					
				}
				
			} else if (movingNode) { // MOVE COLOR NODES
				
				if(mousex < 0) mousex = 0;
				if(mousex > 1) mousex = 1;
				if(mousey < 0-NODE_HEIGHT) mousey = 0-NODE_HEIGHT;
				if(mousey > 0) mousey = 0;
				
				if( Fl::event_button1() ) // LEFT MOUSE
				{
					if(selectedColorNode > 1){
						colorNodes[selectedColorNode].xpos = mousex;
						// modified the colornode, so must update
						updateColorTable();
					}
				}
				
				if( Fl::event_button2() ) // MIDDLE MOUSE
				{
				}
				
				if( Fl::event_button3() ) // RIGHT MOUSE
				{
					mousex = Fl::event_x();
					mousey = Fl::event_y();
				}
				
			}
				
			prevMousex = mousex;
			prevMousey = mousey;
			
			redraw();
			
			return 1;
		case FL_RELEASE:    
			//			... mouse up event ...
			drawingLine = false;
			movingNode = false;
			
			return 1;
		case FL_FOCUS :
		case FL_UNFOCUS :
			//			... Return 1 if you want keyboard events, 0 otherwise
			return 1;
		case FL_KEYBOARD:
			//			... keypress, key is in Fl::event_key(), ascii in Fl::event_text()
			//			... Return 1 if you understand/use the keyboard event, 0 otherwise...
			std::cout << "Keyboard@TransferFunc1D: " << Fl::event_key() << ", " << Fl::event_text() << std::endl;
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


void TransferFunc1D::updateColorTable() {
	
	//fprintf(stderr,"test");
	// interpolate alpha channel from alpha nodes
	for(int i = 0; i < resolution; i++) {
		float n = ((float)i/(float)resolution) * (alphaNodes.size()-1.0); // 0 -> w()-1 index into alpha nodes
		int nFloor = int(floor(n));
		int nCeil = int(ceil(n));
		float delta = n - (float)nFloor;
		colorTable[i*4 + 3] =
		    alphaNodes[nFloor] * (1-delta) + alphaNodes[nCeil] * (delta);		
	}
	
	int last_hit = 0;
	int dist = resolution;
	// interpolate rgb channels from color nodes
	
	for(unsigned int j = 0; j < colorNodes.size(); j++) {
		colorNodes[j].index = int( colorNodes[j].xpos*(resolution-1) );
	}
	
	
	for(int i = 0; i < resolution; i++) {
		for(unsigned int j = 0; j < colorNodes.size(); j++) {
			
			if (colorNodes[j].index == i) {
				dist = i-last_hit;
				
				colorTable[i*4+0] = colorNodes[j].r;
				colorTable[i*4+1] = colorNodes[j].g;
				colorTable[i*4+2] = colorNodes[j].b;
				
				for(int jj = last_hit; jj < i ; jj++ ){
					
					colorTable[jj*4+0] = ((float)(jj - last_hit)/((float)dist)) * colorTable[i*4+0] + 
					    (1.0 - ((float)(jj - last_hit)/(float)(dist))) * colorTable[last_hit*4+0];
					colorTable[jj*4+1] = ((float)(jj - last_hit)/((float)dist)) * colorTable[i*4+1] + 
						(1.0 - ((float)(jj - last_hit)/((float)dist))) * colorTable[last_hit*4+1];
					colorTable[jj*4+2] = ((float)(jj - last_hit)/((float)dist)) * colorTable[i*4+2] + 
						(1.0 - ((float)(jj - last_hit)/((float)dist))) * colorTable[last_hit*4+2];
				}
				last_hit = i;
			}
		}
	}

	
	if(m_pVolume){
		if(m_pVolume->m_pVolume && histogram1D)
			histogram1D->updateHistogram(m_pVolume->m_pVolume,m_pVolume->m_vSize[0]*m_pVolume->m_vSize[1]*m_pVolume->m_vSize[2]);
		SaveColor();
		//controlWin->needUpdate = true;
		m_pVolume->ReDraw();
	}
}


// fillGap(int)
// Pass the fltk keys for shift or control or mouse1 or mouse3 to change gap
// filling behavior

void TransferFunc1D::fillGap(int mode) {

	int x,prevX;
	x = int( mousex * ((float)alphaNodes.size()-1.0));
	prevX = int( prevMousex * ((float)alphaNodes.size()-1.0));
	
	int delta = abs(prevX-x);
	for(int i = (min(prevX,x)+1); i < max(x,prevX); i++){
		float n = (float)(i-min(prevX,x))/(float)delta;
		// set interpolated value based on drawing mode
	
		float smlY = 0;
		float bigY = 0;

		if (x > prevX) {
			smlY = prevMousey;
			bigY = mousey;
		} else {
			bigY = prevMousey;
			smlY = mousey;
		}
		
		if (mode == FL_SHIFT) {
			if ( (smlY * (1-n) + bigY * n) > alphaNodes[int(i)] ) {
				alphaNodes[int(i)] = smlY * (1-n) + bigY * n; 
			}
		} else if (mode == FL_CTRL) {
			if ( (smlY * (1-n) + bigY * n) < alphaNodes[int(i)] ) {
				alphaNodes[int(i)] = smlY * (1-n) + bigY * n;
			}
		} else if (mode == FL_BUTTON1) {
			alphaNodes[int(i)] = smlY * (1-n) + bigY * n; 	
		} else if (mode == FL_BUTTON3) {
			alphaNodes[int(i)] = 0;
		} else {
			assert(0);
		}
	}
	

}




/* 
when window is resized, causing the alphaNodes list to be resized to fill 
the largest window size, we need to reinterpolate the old smaller alpha list 
into the larger list so that the features are still in the same location 
in the transferfunction texture.
Note: This should only be called when current resized screen is larger than the
alpha array. The point here is to maintain the highest resolution ever demanded
by the user
*/
void TransferFunc1D::reinterpolate(int newWidth) {
	int oldWidth = alphaNodes.size();
	assert( oldWidth < newWidth );
	//alphaNodes.resize(w());
	
	std::vector< float > tempNodes(newWidth);
	
	for(int i = 0; i < newWidth; i++) {
		float n = ((float)i/((float)newWidth-1)) * (oldWidth-1.0); // 0 -> w()-1 index into alpha nodes
		// get interpolation locations for start and end
		int nFloor = int(floor(n));
		int nCeil = int(ceil(n));
		float delta = n - (float)nFloor;
		tempNodes[i] =
		    alphaNodes[nFloor] * (1-delta) + alphaNodes[nCeil] * (delta);		
	}
	
	// copy back into the resized alpha array
	alphaNodes.resize(newWidth);
	for( int i = 0; i < newWidth; i++) {
		alphaNodes[i] = tempNodes[i];
	}
}


int TransferFunc1D::Max(int a, int b) {
	if (b > a) return b;
	return a;
}


int TransferFunc1D::Min(int a, int b) {
	if (b < a) return b;
	return a;
}


void TransferFunc1D::SaveColor()
{
    assert(m_pVolume != NULL);    
    assert(m_pVolume->m_pVisStatus != NULL);
    
    m_pVolume->m_pVisStatus->m_vTFButtonSettings.clear();
    m_pVolume->m_pVisStatus->m_vTFLineSettings.clear();

    for(int i = 0; i < resolution; i++) {
 
        m_pVolume->m_pVisStatus->m_Colors[i] = cxColor(colorTable[i*4+0],
													   colorTable[i*4+1],
													   colorTable[i*4+2],
													   colorTable[i*4+3]);       
    }

	/* NEW
    for (unsigned int i = 0; i < colorNodes.size(); i++) {
		outFile << colorNodes[i].xpos << " ";
		outFile << colorNodes[i].r << " ";
		outFile << colorNodes[i].g << " ";
		outFile << colorNodes[i].b << " ";
	}
	outFile << alphaNodes.size() << " ";
	for (unsigned int i = 0; i < alphaNodes.size(); i++) {
		outFile << alphaNodes[i] << " ";
	NEW */
	/*
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
    */
    
    m_pVolume->ReDraw();
}

void TransferFunc1D::LoadColorFromFile()
{
	/*
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
	*/
}
/*
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
*/





// ColorChooser

ColorChooser::ColorChooser(int x, int y, int w, int h, TransferFunc1D **inTransFunc) : Fl_Color_Chooser(x, y, w, h){
	//	Fl::add_idle(&idle_cp, (void*)this);
	transFunc = *inTransFunc;
	rgb(1.0,1.0,1.0);
	
	callback((Fl_Callback*)colorChooser_cb, this);
	when(FL_WHEN_CHANGED);

	
}

void colorChooser_cb(ColorChooser *w, void *o)
{	
	ColorChooser* cc = reinterpret_cast<ColorChooser*>(o);
	if(cc->transFunc->selectedColorNode > -1){ // && event!=FL_MOVE) { //&& event==FL_RELEASE) {
		cc->transFunc->colorNodes[cc->transFunc->selectedColorNode].r = w->r();
		cc->transFunc->colorNodes[cc->transFunc->selectedColorNode].g = w->g();
		cc->transFunc->colorNodes[cc->transFunc->selectedColorNode].b = w->b();
		cc->transFunc->updateColorTable();
		cc->transFunc->redraw();
	}
	
}




// ColorNode

void ColorNode::draw(float width, float bottom, bool selected) {
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);	
	glColor3f(r,g,b);
	glBegin(GL_TRIANGLES);
	{
		glVertex2f(xpos+(NODE_WIDTH/(width*2.0)),bottom);
		glVertex2f(xpos-(NODE_WIDTH/(width*2.0)),bottom);
		glVertex2f(xpos,0.0);
	}
	glEnd();
	
	if(selected) {
		glColor3f(1.0,1.0,1.0);
	} else {
		glColor3f(0.5,0.5,0.5);
	}
	
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glBegin(GL_TRIANGLES);
	{
		glVertex2f(xpos+(NODE_WIDTH/(width*2.0)),bottom);
		glVertex2f(xpos-(NODE_WIDTH/(width*2.0)),bottom);
		glVertex2f(xpos,0.0);
	}
	glEnd();
	
	glPopAttrib();
			   
	
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
	

// UI CALLBACKS
void buttonA_cb(Fl_Button *w, void *o)
{
	TransferFunc1D* tf = reinterpret_cast<TransferFunc1D*>(o);
	tf->setDefaults();	
	tf->updateColorTable();
	tf->redraw();
}


