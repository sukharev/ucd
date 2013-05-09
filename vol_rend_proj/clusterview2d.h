/*
 *  ClusterView2D.h
 *  amrview
 *
 *  Created by Ryan Armstrong on 2/15/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef CLUSTERVIEW2D_H
#define CLUSTERVIEW2D_H


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
#include <FL/Fl_File_Chooser.H>

using namespace std;
#include <fstream>


class cxVolume;
class ColorChooser;
class ColorNode;

class ClusterView2D : public Fl_Gl_Window {
public:
	virtual void draw();
	//int handle(int);
private:
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
	ClusterView2D(int x, int y, int w, int h, int resolution, cxVolume * pVolume);
	virtual ~ClusterView2D();
	bool read(const char* filename);
	void clean();
	
	inline float YMap(float yc);
	inline float XMap(float i); 
	void ColorCluster(int clusterid, float a);
	// sets

	//void setDefaults();


	// UI CALLBACKS
	//call back function for button
	inline void cb_ButtonA_i(Fl_Button*, void*);
	static void cb_ButtonA(Fl_Button*, void*);
	
	inline void cb_ButtonChooser_i(Fl_Button*, void*);
	static void cb_ButtonChooser(Fl_Button*, void*);
	
	inline void cb_ButtonLoad_i(Fl_Button*, void*);
	static void cb_ButtonLoad(Fl_Button*, void*);

	inline void cb_ButtonSave_i(Fl_Button*, void*);
	static void cb_ButtonSave(Fl_Button*, void*);

	void cb_LB_i(Fl_Light_Button*, void*);
    static void cb_LB(Fl_Light_Button*, void*);

	inline void cb_ButtonNext_i(Fl_Button*, void*);
	static void cb_ButtonNext(Fl_Button*, void*);

#ifdef CHANGES
	inline void cb_ButtonClear_i(Fl_Light_Button*, void*);
	static void cb_ButtonClear(Fl_Light_Button*, void*);
#endif

	bool isInit() {return m_bInit;}
	int getDim() {return m_dim;}
	int currCluster();

	int** getClusterVol() { return m_cluster; }
private:
	void drawBackground();
		
	void fillGap(int mode);
	int Max(int a, int b);
	int Min(int a, int b);	

	std::vector< float > alphaNodes;
	
	int selectedColorNode;
	

	ifstream m_clusterStream;
	ifstream m_dataStream;

	//data
	float** m_dataPts;
	int* m_clusterPts;   
	int** m_cluster;
	float*** m_ctrs;
	int m_total_size;    // total number of data points
	int m_dim;           // number of timesteps

	float m_percentHeight;
	float m_percentWidth;

	float m_datamin;
	float m_datamax;
	
public:

	bool m_bshowall;
	bool m_bInit;
	int m_selcluster;
	int m_nClusters;

	int m_xblock;
	int m_yblock;
	int m_zblock;

	int m_ts_chunk;     // number of timesteps in a chunk

public:
	Fl_Button * m_buttonA;
	Fl_Button * m_pButtonNext;
	Fl_Button * m_pButtonChooser;
	Fl_File_Chooser * m_pFileChooser;
	Fl_Input *m_pInputA;

#ifdef CHANGES
	Fl_Light_Button * m_pButtonClear;
	bool m_bClear;
#endif
	Fl_Light_Button *m_pLB;
	Fl_Input* m_inp_tch;
	Fl_Input* m_inp_t;
	Fl_Input* m_inp_k;
	Fl_Input* m_inp_s;
	Fl_Input* m_inp_start_time;
	Fl_Input* m_inp_total_time;
};


#endif //CLUSTERVIEW2D_H
