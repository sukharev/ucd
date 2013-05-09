/***************************************************************************
                          winctrllight.cpp  -  description
                             -------------------
    begin                : Mon Jan 20 2003
    copyright            : (C) 2003 by Hongfeng YU
    email                : hfyu@ucdavis.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 * Copyright: All rights reserved.  May not be used, modified, or copied   *
 * without permission.                                                     *
 *                                                                         *
 ***************************************************************************/


#include "winctrllight.h"
#include "winvisview.h"
#include "cxvolume.h"
#include <iostream>
#include <fstream>

using namespace std;

#define TEXTSIZE 12



WinCtrlLight::WinCtrlLight(int x = 0, int y =0, int w = 200, int h = 200, cxVolume * pVolume = NULL){
	GenWindow(x,y, w,h);
	m_pVolume =  pVolume;
	reset();
}

WinCtrlLight::~WinCtrlLight(){    
}

void WinCtrlLight::GenWindow(int x, int y, int w, int h)
{
	m_wMainWin = new Fl_Window(x, y,w,h, "Control Center");
	m_wMainWin->user_data((void*)(this));
	
    int step = 17;
    
// 	//m_pGroupA = new Fl_Group(3,20,w-6,h*15/100, "General Settings");
// 	m_pGroupA = new Fl_Group(3,20,w-6, 60, "General Settings");
// 	m_pGroupA->box(FL_ENGRAVED_BOX);
// 	m_pGroupA->align(FL_ALIGN_TOP_LEFT);
// 	m_pGroupA->user_data((void*)(this));
// 	
// 	int yy = m_pGroupA->y();
// 	
// 	
// 	m_pCheckA = new Fl_Check_Button(7, yy+0*step, 25, 20, "Show axes");
// 	m_pCheckA->labelsize(TEXTSIZE);
// 	m_pCheckA ->down_box(FL_DOWN_BOX);
// 	m_pCheckA->callback((Fl_Callback *)cb_CheckA);
// 		
// 	m_pCheckB = new Fl_Check_Button(7, yy+1*step, 25, 20, "Show boundary");
// 	m_pCheckB->labelsize(TEXTSIZE);
// 	m_pCheckB->down_box(FL_DOWN_BOX);
// 	m_pCheckB->callback((Fl_Callback *)cb_CheckB);
// 		
// 		
// 	m_pCheckC = new Fl_Check_Button(7, yy+2*step, 25, 20, "Show Volume");
// 	m_pCheckC->labelsize(TEXTSIZE);
// 	m_pCheckC->down_box(FL_DOWN_BOX);
// 	m_pCheckC->callback((Fl_Callback *)cb_CheckC);
// 	
// 	m_pGroupA->end();
	
	m_pGroupB = new Fl_Group(3,20,w-6, 140, "Lighting Settings");
	m_pGroupB ->box(FL_ENGRAVED_BOX);
	m_pGroupB ->align(FL_ALIGN_TOP_LEFT);
	m_pGroupB->user_data((void*)(this));
	
	int yy = m_pGroupB->y();
	
    m_pSliderA = new Fl_Value_Slider(5,yy+1*step, w - 10, 20, "ka");
    m_pSliderA->align(FL_ALIGN_TOP_LEFT);
    m_pSliderA->labelsize(TEXTSIZE);
    m_pSliderA->type(FL_HOR_NICE_SLIDER);
    m_pSliderA->range(0.0, 1.0);
    m_pSliderA->value(0.1);
    m_pSliderA->step(0.01);    
    m_pSliderA->callback((Fl_Callback *)cb_SliderA);
    
    m_pSliderB = new Fl_Value_Slider(5,yy+3*step, w - 10, 20, "kd");
    m_pSliderB->align(FL_ALIGN_TOP_LEFT);
    m_pSliderB->labelsize(TEXTSIZE);
    m_pSliderB->type(FL_HOR_NICE_SLIDER);
    m_pSliderB->range(0.0, 1.0);
    m_pSliderB->value(0.9);
    m_pSliderB->step(0.01); 
    m_pSliderB->callback((Fl_Callback *)cb_SliderB);
   
    m_pSliderC = new Fl_Value_Slider(5,yy+5*step, w - 10, 20, "ks");
    m_pSliderC->align(FL_ALIGN_TOP_LEFT);
    m_pSliderC->labelsize(TEXTSIZE);
    m_pSliderC->type(FL_HOR_NICE_SLIDER);
    m_pSliderC->range(0.0, 1.0);
    m_pSliderC->value(0.6);
    m_pSliderC->step(0.01); 
    m_pSliderC->callback((Fl_Callback *)cb_SliderC);
    
    m_pSliderD = new Fl_Value_Slider(5,yy+7*step, w - 10, 20, "shininess");
    m_pSliderD->align(FL_ALIGN_TOP_LEFT);
    m_pSliderD->labelsize(TEXTSIZE);
    m_pSliderD->type(FL_HOR_NICE_SLIDER);
    m_pSliderD->range(1, 128);
    m_pSliderD->value(20);
    m_pSliderD->step(1); 
    m_pSliderD->callback((Fl_Callback *)cb_SliderD);
    
    m_pGroupB->end();	
 	
 	m_pGroupC = new Fl_Group(3, m_pGroupB->y()+m_pGroupB->h()+step, w-6, 50, "Image Quality");    
 	m_pGroupC ->box(FL_ENGRAVED_BOX);
 	m_pGroupC ->align(FL_ALIGN_TOP_LEFT);
 	m_pGroupC->user_data((void*)(this));
 	
    yy = m_pGroupC->y();	
 	
    m_pSliderE = new Fl_Value_Slider(5,yy+1*step, w - 10, 20, "sampling spacing");
    m_pSliderE->align(FL_ALIGN_TOP_LEFT);
    m_pSliderE->labelsize(TEXTSIZE);
    m_pSliderE->type(FL_HOR_NICE_SLIDER);
    
#ifdef DRAW_LARGEIMAGE
    m_pSliderE->range(0.0001, 1.0/128.0*pow(1.2,4.0));
    m_pSliderE->value(0.0001);
    m_pSliderE->step(0.000001); 
#else    
    m_pSliderE->range(1.0/128.0/pow(1.2,20.0), 1.0/128.0*pow(1.2,4.0));
    m_pSliderE->value(1.0/128.0);
    m_pSliderE->step(0.0001); 
#endif

    
    
    m_pSliderE->callback((Fl_Callback *)cb_SliderE);

 	m_pGroupC->end();

/*    m_pGroupD = new Fl_Group(3,m_pGroupC->y()+ m_pGroupC->h()+step, w-6, 50, "Cutting Planes");
 	m_pGroupD->box(FL_ENGRAVED_BOX);
 	m_pGroupD->align(FL_ALIGN_TOP_LEFT);
 	m_pGroupD->user_data((void*)(this));
 	
    yy = m_pGroupD->y();	
 	
    m_pRoundA = new Fl_Round_Button(5,yy+0*step, 15, 20, "x");
    m_pRoundA->type(102);
    m_pRoundA->labelsize(TEXTSIZE);
    m_pRoundA->down_box(FL_ROUND_DOWN_BOX);

    m_pRoundB = new Fl_Round_Button(5,yy+1*step, 15, 20, "y");
    m_pRoundB->type(102);
    m_pRoundB->labelsize(TEXTSIZE);
    m_pRoundB->down_box(FL_ROUND_DOWN_BOX);

    m_pRoundC = new Fl_Round_Button(5,yy+2*step, 15, 20, "z");
    m_pRoundC->type(102);
    m_pRoundC->labelsize(TEXTSIZE);
    m_pRoundC->down_box(FL_ROUND_DOWN_BOX);

    m_pSliderF = new Fl_Value_Slider(60, yy+0*step, w-70, 20, "min");
    m_pSliderF->align(FL_ALIGN_LEFT);
    m_pSliderF->labelsize(TEXTSIZE);
    m_pSliderF->type(FL_HOR_NICE_SLIDER);
    m_pSliderF->range(0, 1);
    m_pSliderF->value(0);
    m_pSliderF->step(0.01); 
    m_pSliderF->callback((Fl_Callback *)cb_SliderF);

    m_pSliderG = new Fl_Value_Slider(60, yy+1*step, w-70, 20, "max");
    m_pSliderG->align(FL_ALIGN_LEFT);
    m_pSliderG->labelsize(TEXTSIZE);
    m_pSliderG->type(FL_HOR_NICE_SLIDER);
    m_pSliderG->range(0, 1);
    m_pSliderG->value(1);
    m_pSliderG->step(0.01); 
    m_pSliderG->callback((Fl_Callback *)cb_SliderG);

 	m_pGroupD->end();*/
        
//	m_pCheckD = new Fl_Check_Button(7, yy+3*step, 25, 20, "Option");
//	m_pCheckD->labelsize(TEXTSIZE);
//	m_pCheckD ->down_box(FL_DOWN_BOX);
//	m_pCheckD->callback((Fl_Callback *)cb_CheckD);

// 	m_pCheckE = new Fl_Check_Button(7,yy+0*step,25,20,"Option");
// 	m_pCheckE->labelsize(TEXTSIZE);
// 	m_pCheckE->down_box(FL_DOWN_BOX);
// 	m_pCheckE->callback((Fl_Callback *)cb_CheckE);
// 	
// 	m_pButtonB = new Fl_Button(7,yy+2*step, 100, 20, "Button");
// 	m_pButtonB -> labelsize(TEXTSIZE);
// 	m_pButtonB->callback((Fl_Callback *)cb_ButtonB);
// 
// 	m_pCheckF = new Fl_Check_Button(7,yy+0*step,25,20,"Option");
// 	m_pCheckF->labelsize(TEXTSIZE);
// 	m_pCheckF ->down_box(FL_DOWN_BOX);
// 	m_pCheckF ->callback((Fl_Callback *)cb_CheckF);
// 	
// 	m_pCheckG = new Fl_Check_Button(7,yy+1*step,25,20,"Option");
// 	m_pCheckG->labelsize(TEXTSIZE);
// 	m_pCheckG ->down_box(FL_DOWN_BOX);
// 	m_pCheckG->callback((Fl_Callback *) cb_CheckG);
// 	
// 	m_pCheckH = new Fl_Check_Button(7,yy+2*step,25,20,"Option");
// 	m_pCheckH->labelsize(TEXTSIZE);
// 	m_pCheckH->down_box(FL_DOWN_BOX);
// 	m_pCheckH->callback((Fl_Callback *)cb_CheckH);
// 
// 	m_pSliderA = new Fl_Value_Slider(5,yy+3*step, w - 10, 20);
// 	m_pSliderA->labelsize(TEXTSIZE);
// 	m_pSliderA->type(FL_HOR_NICE_SLIDER);
// 	m_pSliderA->range(0, 600);
// 	m_pSliderA->value(80);
// 	m_pSliderA->precision(0);
// 	m_pSliderA->callback((Fl_Callback *)cb_SliderA);
// 	
// 	m_pSliderB = new Fl_Value_Slider(5,yy+4*step, w - 10, 20);
// 	m_pSliderB->labelsize(TEXTSIZE);
// 	m_pSliderB->type(FL_HOR_NICE_SLIDER);
// 	m_pSliderB->range(0, 0.03);
// 	m_pSliderB->value(0.003);
// 	m_pSliderB->step(0.0005);
// 	//m_pSliderB->precision(0.0001);
// 	m_pSliderB->callback((Fl_Callback *)cb_SliderB);
// 	
// 	
// 	m_pBrowserA = new Fl_Hold_Browser(5,yy+5.5*step, w - 60, 150);
// 	m_pBrowserA->align(FL_ALIGN_LEFT_TOP);
// 	m_pBrowserA->labelsize(TEXTSIZE);
// 	m_pBrowserA->callback((Fl_Callback*)cb_BrowserA);
// 	
// 	m_pButtonC = new Fl_Button(w - 50,yy+5.5*step, 40, 20, "Button");
// 	m_pButtonC -> labelsize(TEXTSIZE);
// 	m_pButtonC->callback((Fl_Callback *)cb_ButtonC);
// 	
// 	m_pButtonD = new Fl_Button(w - 50,yy+5.5*step + 20, 40, 20, "Button");
// 	m_pButtonD -> labelsize(TEXTSIZE);
// 	m_pButtonD->callback((Fl_Callback *)cb_ButtonD);
 	
// 	m_pButtonA = new Fl_Button(5,h-25, 100, 20, "Default Position");
// 	m_pButtonA -> labelsize(TEXTSIZE);
// 	m_pButtonA->callback((Fl_Callback *)cb_ButtonA);
	
	m_wMainWin->end();
	m_wMainWin->resizable(m_wMainWin);
	m_wMainWin->size_range(200,200);
}


void WinCtrlLight::show(int argc, char **argv){
	m_wMainWin->show(argc,argv);
}


inline void WinCtrlLight::cb_CheckA_i(Fl_Check_Button* o, void*)
{
	if(m_pVolume == NULL)
		return;
	m_pVolume->m_pVisStatus->m_bDrawAxes = (o->value() == 1)? true:false;
	m_pVolume->ReDraw();
	SaveConfig();
}

void WinCtrlLight::cb_CheckA(Fl_Check_Button* o, void* v) {
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_CheckA_i(o,v);
}

inline void WinCtrlLight::cb_CheckB_i(Fl_Check_Button* o, void*)
{
	if(m_pVolume == NULL)
		return;
	m_pVolume->m_pVisStatus->m_bDrawFrame = (o->value() == 1)? true:false;
	m_pVolume->ReDraw();
	SaveConfig();
}

void WinCtrlLight::cb_CheckB(Fl_Check_Button* o, void* v) {
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_CheckB_i(o,v);
}

inline void WinCtrlLight::cb_CheckC_i(Fl_Check_Button* o, void*)
{
	if(m_pVolume == NULL)
		return;
	m_pVolume->m_pVisStatus->m_bDrawVolume = (o->value() == 1)? true:false;
	m_pVolume->ReDraw();
    SaveConfig();
}

void WinCtrlLight::cb_CheckC(Fl_Check_Button* o, void* v) {
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_CheckC_i(o,v);
}

inline void WinCtrlLight::cb_CheckD_i(Fl_Check_Button* o, void*)
{
	if(m_pVolume == NULL)
		return;
//	m_pVolume->m_pVisStatus->m_bDrawTorus = (o->value() == 1)? true:false;
	m_pVolume->ReDraw();
	SaveConfig();
}

void WinCtrlLight::cb_CheckD(Fl_Check_Button* o, void* v) {
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_CheckD_i(o,v);
}


inline void WinCtrlLight::cb_CheckE_i(Fl_Check_Button* o, void*)
{
	if(m_pVolume == NULL)
		return;
	
//	m_pVolume->m_pVisStatus->m_bDrawSlice = (o->value() == 1)? true:false;
	m_pVolume->ReDraw();
    SaveConfig();
}


void WinCtrlLight::cb_CheckE(Fl_Check_Button* o, void* v) {
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_CheckE_i(o,v);
}


inline void WinCtrlLight::cb_CheckF_i(Fl_Check_Button* o, void*)
{
	if(m_pVolume == NULL)
		return;
	
    SaveConfig();
}


void WinCtrlLight::cb_CheckF(Fl_Check_Button* o, void* v) {
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_CheckF_i(o,v);
}


inline void WinCtrlLight::cb_CheckG_i(Fl_Check_Button* o, void*)
{
	if(m_pVolume == NULL)
		return;
    SaveConfig();
}


void WinCtrlLight::cb_CheckG(Fl_Check_Button* o, void* v) {
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_CheckG_i(o,v);
}

inline void WinCtrlLight::cb_CheckH_i(Fl_Check_Button* o, void*)
{
	if(m_pVolume == NULL)
		return;
		
    SaveConfig();
}

void WinCtrlLight::cb_CheckH(Fl_Check_Button* o, void* v) {
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_CheckH_i(o,v);
}


inline void WinCtrlLight::cb_SliderA_i(Fl_Value_Slider* o, void* v)
{
	if(m_pVolume == NULL || !m_pVolume->m_pVisStatus)
		return;
    

    m_pVolume->m_pVisStatus->m_fLightPar[0] = o->value();
    
    m_pVolume->ReDraw();
    SaveConfig();
}  


void WinCtrlLight::cb_SliderA(Fl_Value_Slider* o, void* v)
{
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_SliderA_i(o,v);
}


inline void WinCtrlLight::cb_SliderB_i(Fl_Value_Slider* o, void* v)
{
	if(m_pVolume == NULL)
		return;
    
    m_pVolume->m_pVisStatus->m_fLightPar[1] = o->value();
	
    m_pVolume->ReDraw();
    SaveConfig();
}  


void WinCtrlLight::cb_SliderB(Fl_Value_Slider* o, void* v)
{
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_SliderB_i(o,v);
}

inline void WinCtrlLight::cb_SliderC_i(Fl_Value_Slider* o, void* v)
{
	if(m_pVolume == NULL)
		return;
    
    m_pVolume->m_pVisStatus->m_fLightPar[2] = o->value();
	
    m_pVolume->ReDraw();
    SaveConfig();
}  


void WinCtrlLight::cb_SliderC(Fl_Value_Slider* o, void* v)
{
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_SliderC_i(o,v);
}

inline void WinCtrlLight::cb_SliderD_i(Fl_Value_Slider* o, void* v)
{
	if(m_pVolume == NULL)
		return;
    
    m_pVolume->m_pVisStatus->m_fLightPar[3] = o->value();
	
    m_pVolume->ReDraw();
    SaveConfig();
}  


void WinCtrlLight::cb_SliderD(Fl_Value_Slider* o, void* v)
{
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_SliderD_i(o,v);
}



inline void WinCtrlLight::cb_SliderE_i(Fl_Value_Slider* o, void* v)
{
	if(m_pVolume == NULL)
		return;
    
    m_pVolume->m_pVisStatus->m_fSampleSpacing = o->value();
	
    m_pVolume->ReDraw();
    SaveConfig();
}  


void WinCtrlLight::cb_SliderE(Fl_Value_Slider* o, void* v)
{
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_SliderE_i(o,v);
}


inline void WinCtrlLight::cb_SliderF_i(Fl_Value_Slider* o, void* v)
{
	if(m_pVolume == NULL)
		return;
    
    float min = o->value();
    float max = m_pSliderG->value();

    if (min > max)
        min = max;

    if (m_pRoundA->value()==1)
        m_pVolume->m_pVisStatus->m_vRangeMin[0] = min;
    else if (m_pRoundB->value()==1)
        m_pVolume->m_pVisStatus->m_vRangeMin[1] = min;
    else if (m_pRoundC->value()==1)
        m_pVolume->m_pVisStatus->m_vRangeMin[2] = min;

	
    m_pVolume->ReDraw();
    SaveConfig();
}  


void WinCtrlLight::cb_SliderF(Fl_Value_Slider* o, void* v)
{
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_SliderF_i(o,v);
}


inline void WinCtrlLight::cb_SliderG_i(Fl_Value_Slider* o, void* v)
{
	if(m_pVolume == NULL)
		return;

    float max = o->value();
    float min = m_pSliderE->value();

    if (min > max)
        max = min;

    if (m_pRoundA->value()==1)
        m_pVolume->m_pVisStatus->m_vRangeMax[0] = max;
    else if (m_pRoundB->value()==1)
        m_pVolume->m_pVisStatus->m_vRangeMax[1] = max;
    else if (m_pRoundC->value()==1)
        m_pVolume->m_pVisStatus->m_vRangeMax[2] = max;
	
    m_pVolume->ReDraw();
    SaveConfig();
}  


void WinCtrlLight::cb_SliderG(Fl_Value_Slider* o, void* v)
{
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_SliderG_i(o,v);
}


inline void WinCtrlLight::cb_ButtonA_i(Fl_Button* o, void*)
{
	if(m_pVolume == NULL)
		return;
//	m_pVisMain->m_wView->Reset();
	m_pVolume->ReDraw();
    SaveConfig();
}

void WinCtrlLight::cb_ButtonA(Fl_Button* o, void* v) {
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_ButtonA_i(o,v);
}

inline void WinCtrlLight::cb_ButtonB_i(Fl_Button* o, void*)
{
	if(m_pVolume == NULL)
		return;
    SaveConfig();
}

void WinCtrlLight::cb_ButtonB(Fl_Button* o, void* v) {
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_ButtonB_i(o,v);
}

inline void WinCtrlLight::cb_ButtonC_i(Fl_Button* o, void*)
{
	if(m_pVolume == NULL)
		return;
	

	
	m_pVolume->ReDraw();
    SaveConfig();	
}

void WinCtrlLight::cb_ButtonC(Fl_Button* o, void* v) {
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_ButtonC_i(o,v);
}

inline void WinCtrlLight::cb_ButtonD_i(Fl_Button* o, void*)
{
	if(m_pVolume == NULL)
		return;
		
	//delete
	int size = m_pBrowserA->size();
	if (size <= 0)
		return;
		

	m_pVolume->ReDraw();
	SaveConfig();
}

void WinCtrlLight::cb_ButtonD(Fl_Button* o, void* v) {
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_ButtonD_i(o,v);
}

inline void WinCtrlLight::cb_BrowserA_i(Fl_Hold_Browser* o, void*)
{
	if(m_pVolume == NULL)
		return;
    SaveConfig();
}

void WinCtrlLight::cb_BrowserA(Fl_Hold_Browser* o, void* v) {
  ((WinCtrlLight*)(o->parent()->user_data()))->cb_BrowserA_i(o,v);
}



void WinCtrlLight::reset()
{
    char filename[1024];

#ifdef MULTI_VARIABLES
    sprintf(filename, "ctrllight%d.cfg", m_pVolume->m_nVolumeID);
#else
    sprintf(filename, "ctrllight.cfg");
#endif
    
    ifstream inf(filename);

    if (inf) {
        float value;

//        inf.read(reinterpret_cast<char *>(&value), sizeof(float));
//         m_pCheckA->value((int)value);
//         m_pCheckA->do_callback(); 
// 
//         inf.read(reinterpret_cast<char *>(&value), sizeof(float));
//         m_pCheckB->value((int)value);
//         m_pCheckB->do_callback();
// 
//         inf.read(reinterpret_cast<char *>(&value), sizeof(float));
//         m_pCheckC->value((int)value);
//         m_pCheckC->do_callback();
        
        inf.read(reinterpret_cast<char *>(&value), sizeof(float));
        m_pSliderA->value(value);
        m_pSliderA->do_callback();

        inf.read(reinterpret_cast<char *>(&value), sizeof(float));
        m_pSliderB->value(value);
        m_pSliderB->do_callback();

        inf.read(reinterpret_cast<char *>(&value), sizeof(float));
        m_pSliderC->value(value);
        m_pSliderC->do_callback();

        inf.read(reinterpret_cast<char *>(&value), sizeof(float));
        m_pSliderD->value(value);
        m_pSliderD->do_callback();

//         inf.read(reinterpret_cast<char *>(&value), sizeof(float));
//         m_pSliderE->value(value);
//         m_pSliderE->do_callback();

//         inf.read(reinterpret_cast<char *>(&value), sizeof(float));
//         m_pSliderF->value(value);
//         m_pSliderF->do_callback();
// 
//         inf.read(reinterpret_cast<char *>(&value), sizeof(float));
//         m_pSliderG->value(value);
//         m_pSliderG->do_callback();
// 
//         inf.read(reinterpret_cast<char *>(&value), sizeof(float));
//         m_pRoundA->value((int)value);
//         m_pRoundA->do_callback();
// 
//         inf.read(reinterpret_cast<char *>(&value), sizeof(float));
//         m_pRoundB->value((int)value);
//         m_pRoundB->do_callback();
// 
//         inf.read(reinterpret_cast<char *>(&value), sizeof(float));
//         m_pRoundC->value((int)value);
//         m_pRoundC->do_callback();

        inf.close();

    } else {
// 	    m_pCheckA->value(1);
// 	    m_pCheckB->value(1);
// 	    m_pCheckC->value(1);
//         m_pRoundA->value(1);
    //	m_pCheckD->value(0);
    // 	m_pCheckE->value(1);
    // 	m_pCheckF->value(1);
    // 	m_pCheckG->value(1);
    // 	m_pCheckH->value(0);
	    //m_bAdvanceRender->set();
    }
}


void WinCtrlLight::SaveConfig()
{
    char filename[1024];

#ifdef MULTI_VARIABLES
    sprintf(filename, "ctrllight%d.cfg", m_pVolume->m_nVolumeID);
#else
    sprintf(filename, "ctrllight.cfg");
#endif
    
    ofstream outf(filename);
    
    float value;

//     value = m_pCheckA->value();
//     outf.write(reinterpret_cast<char *>(&value), sizeof(float));
// 
//     value = m_pCheckB->value();
//     outf.write(reinterpret_cast<char *>(&value), sizeof(float));
// 
//     value = m_pCheckB->value();
//     outf.write(reinterpret_cast<char *>(&value), sizeof(float));

    value = m_pSliderA->value();
    outf.write(reinterpret_cast<char *>(&value), sizeof(float));

    value = m_pSliderB->value();
    outf.write(reinterpret_cast<char *>(&value), sizeof(float));

    value = m_pSliderC->value();
    outf.write(reinterpret_cast<char *>(&value), sizeof(float));

    value = m_pSliderD->value();
    outf.write(reinterpret_cast<char *>(&value), sizeof(float));
    
//     value = m_pSliderE->value();
//     outf.write(reinterpret_cast<char *>(&value), sizeof(float));

//     value = m_pSliderF->value();
//     outf.write(reinterpret_cast<char *>(&value), sizeof(float));
// 
//     value = m_pSliderG->value();
//     outf.write(reinterpret_cast<char *>(&value), sizeof(float));
// 
//     value = m_pRoundA->value();
//     outf.write(reinterpret_cast<char *>(&value), sizeof(float));
// 
//     value = m_pRoundB->value();
//     outf.write(reinterpret_cast<char *>(&value), sizeof(float));
// 
//     value = m_pRoundC->value();
//     outf.write(reinterpret_cast<char *>(&value), sizeof(float));

    outf.close();
}
