//
// C++ Interface: WinVisView
//
// Description: 
//
//
// Author: Hongfeng Yu <hfyu@ucdavis.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef WINVISVIEW_H
#define WINVISVIEW_H

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#include <assert.h>
#include "image.h"
#include <vector>

using namespace std;

class WinVisMain;
class cxVolume;

class WinVisView : public Fl_Gl_Window
{
public:
	WinVisView(int x,int y,int w,int h,const char *l=0);
	virtual ~WinVisView();
	
    void Reset();   
private:
	//FL_GL_Window
	void draw();
	
    int handle(int event);
	
	//Display
#ifdef MULTI_VARIABLES   
    void ShowMultiObject();  
    void InitProgram();   
    void DrawVolume(); 
    void CreateTfTex(int vol);
    void CreateDataTex(int vol);
#else
    void ShowObject();
#endif

	//Translate	
	void Rotate();
	void Scale();
	void Translate();
	
	//Call back function
	static void cb_wClose(Fl_Gl_Window*, void*);
    
    //Save viewing
    void SaveView();
    void LoadView();
    void SaveTga(char *filename, const unsigned char *image, int image_x, int image_y); 
    void SaveImage();  
    

public:
	WinVisMain * m_pVisMain;
    
    vector<cxVolume*> m_vVolume;
    vector<unsigned int> m_vTexTF;
    vector<unsigned int> m_vTexVol;

	cxVolume * m_pVolume;

	int m_mousex;
	int m_mousey;
	float m_curquat[4];
	float m_scale;
	float m_deltax;
	float m_deltay;
	
	bool m_bOperating;
    bool m_bDrawing;   
    unsigned char * m_pImage;
    int m_nImagewidth;
    int m_nImageheight;
    
    Image m_SaveImage;
    bool m_bSaveImage;
    int m_nImageCount;
    
    int m_nVolProgram; 
    bool m_bReload;
    
    bool m_bBlack;
    
    bool m_bRotation;
    
    bool m_bCut;
    
    bool m_bPlayOn;
    
    bool m_bEnd;
    
    unsigned int m_nFrameNum;
};

void cb_wRedraw(void * pData);

#endif 
