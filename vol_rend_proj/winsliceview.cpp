//
// C++ Implementation: winsliceview
//
// Description: 
//
//
// Author: Hongfeng Yu <hfyu@ucdavis.edu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "GL/glew.h"
#include "winsliceview.h"
#include <GL/glu.h>
#include <FL/glut.H>

#define TEXTSIZE 12

WinSliceMain::WinSliceMain(int x,int y,int w,int h,const char *l)
{
    m_bLBseleced = false;
    m_contour = 50.0;
    GenWindow(x, y, w, h, l);

}

WinSliceMain::~WinSliceMain()
{

}

void WinSliceMain::GenWindow(int x,int y,int w,int h,const char *l)
{
    m_pMainWin = new Fl_Window(x, y, w, h, l);
    m_pMainWin->user_data((void*)(this));

    //m_pSliceView = new WinSliceView(0, 0, w, h-30);
    //m_pSliceView = new WinSliceView(0, 47, w, h-80);
    m_pSliceView = new WinSliceView(0,  0, w, h-100);
    m_pSliceView->mode(FL_ALPHA | FL_RGB | FL_DOUBLE | FL_DEPTH);
    //Fl::belowmouse(m_pSliceView);
    m_pSliceView->setCorrMode(false);
	//m_pSliceView->resizable(m_pSliceView);
	m_pSliceView->end();

    //m_pSlider = new Fl_Value_Slider(5, h - 25, w - 10, 20);
    m_pSlider = new Fl_Value_Slider(5, m_pSliceView->y()+h-97, w - 10, 20);
    m_pSlider->align(FL_ALIGN_TOP_LEFT);
    m_pSlider->labelsize(TEXTSIZE);
    m_pSlider->type(FL_HOR_NICE_SLIDER);
    m_pSlider->range(-1.0, 1.0);
    m_pSlider->value(0.0);    
    m_pSlider->step(0.001); 
    m_pSlider->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
    m_pSlider->callback((Fl_Callback *)cb_SliderA);
    
    Fl_Group* m_gGroupB = new Fl_Group(0, m_pSlider->y()+23, w*4/9 - 90, 74);
    m_gGroupB->user_data((void*)(this));
    m_gGroupB->box(FL_THIN_UP_FRAME);
    m_pRoundA = new Fl_Round_Button(m_gGroupB->x() + 3, 
				    m_gGroupB->y() + 3, 
				    70, 20, "Slice X");
    m_pRoundA->type(102);
    m_pRoundA->down_box(FL_ROUND_DOWN_BOX);
    m_pRoundA->labelsize(TEXTSIZE);
    m_pRoundA->callback((Fl_Callback *)cb_RadioA);
      		
    m_pRoundB = new Fl_Round_Button(m_gGroupB->x() + 3, 
	        		    m_gGroupB->y() + 3 + m_pRoundA->h(), 
				    70, 20, "Slice Y");
    m_pRoundB->type(102);
    m_pRoundB->down_box(FL_ROUND_DOWN_BOX);
    m_pRoundB->labelsize(TEXTSIZE);
    m_pRoundB->callback((Fl_Callback *)cb_RadioB);
      	
    m_pRoundC = new Fl_Round_Button(m_gGroupB->x() + 3, 
			 	    m_gGroupB->y() + 6 + m_pRoundA->h()*2, 
				    70, 20, "Slice Z");
    m_pRoundC->type(102);
    m_pRoundC->down_box(FL_ROUND_DOWN_BOX);
    m_pRoundC->labelsize(TEXTSIZE);
    m_pRoundC->callback((Fl_Callback *)cb_RadioC);
    /*
	m_pLB_corr = new Fl_Light_Button(m_pRoundA->w() + 3,
			 	    m_gGroupB->y() + 3, 
				    130, 30, "Activate Correlation");
    m_pLB_corr->labelsize(TEXTSIZE);
    m_pLB_corr->callback((Fl_Callback *)cb_LB_corr, this);
	*/
/*   
 m_pSaveTAC = new Fl_Button(m_pRoundA->w() + 3,
			 	    m_gGroupB->y() + 3, 
				    110, 30, "Save selected TAC");
    m_pSaveTAC->labelsize(TEXTSIZE);
    m_pSaveTAC->callback((Fl_Callback *)cb_SaveTAC);
	
	m_pSaveGTAC = new Fl_Button(m_pSaveTAC->x() + m_pSaveTAC->w() + 3,
			 	    m_gGroupB->y() + 3, 
				    m_pSaveTAC->w(), 30, "Save volume TAC");
    m_pSaveGTAC->labelsize(TEXTSIZE);
    m_pSaveGTAC->callback((Fl_Callback *)cb_SaveGTAC);

    
    m_pSliderTAC = new Fl_Value_Slider(m_pRoundA->w() + 3, 
				      m_pSaveTAC->y() + 33, m_pSaveTAC->w()*2+3, 25, "");
    m_pSliderTAC->align(FL_ALIGN_RIGHT);
    m_pSliderTAC->labelsize(TEXTSIZE);
    m_pSliderTAC->type(FL_HOR_NICE_SLIDER);
    m_pSliderTAC->range(0, 1);
    m_pSliderTAC->value(0);
    m_pSliderTAC->step(0.001);   
    m_pSliderTAC->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
    m_pSliderTAC->callback((Fl_Callback *)cb_SliderTAC); 
*/
    
    m_gGroupB->end();

    Fl_Group* m_gGroupC = new Fl_Group(m_gGroupB->w()+5, m_pSlider->y()+23, 
									  m_pMainWin->w()-m_gGroupB->w() -6/*w*5/9*/, 74);
    m_gGroupC->user_data((void*)(this));
    m_gGroupC->box(FL_THIN_UP_FRAME);
    
    m_pLB = new Fl_Light_Button(m_gGroupC->x() + 3,
			 	    m_gGroupC->y() + 3, 
				    120, 30, "Line Contours");
    //m_pRoundD->type(102);
    //m_pLB->down_box(FL_ROUND_DOWN_BOX);
    m_pLB->labelsize(TEXTSIZE);
    m_pLB->callback((Fl_Callback *)cb_LB);

    m_pLB_color = new Fl_Light_Button(m_pLB->x() + 123 + 3,
			 	    m_gGroupC->y() + 3, 
				    120, 30, "Color Contours");
    //m_pRoundD->type(102);
    //m_pLB->down_box(FL_ROUND_DOWN_BOX);
    m_pLB_color->labelsize(TEXTSIZE);
    m_pLB_color->callback((Fl_Callback *)cb_LB_color);

    m_pSaveButton = new Fl_Button(m_pLB_color->x() + 123,
			 	    m_pLB->y() + 33, /*m_gGroupC->y() + 3,*/ 
				    120, 30, "Save Contour");
    //m_pRoundD->type(102);
    //m_pLB->down_box(FL_ROUND_DOWN_BOX);
    m_pSaveButton->labelsize(TEXTSIZE);
    m_pSaveButton->callback((Fl_Callback *)cb_SaveButton);

    m_pClearButton = new Fl_Button(m_pSaveButton->x(),
			 	    m_gGroupC->y() + 3, 
				    120, 30, "Clear All Contours");
    //m_pRoundD->type(102);
    //m_pLB->down_box(FL_ROUND_DOWN_BOX);
    m_pClearButton->labelsize(TEXTSIZE);
    m_pClearButton->callback((Fl_Callback *)cb_ClearButton);

    m_pBrowser = new Fl_Select_Browser(	m_pClearButton->x() + m_pSaveButton->w() + 3,
							m_gGroupC->y() + 3,160, 60);
    //int br_widths[] = {200};               // widths for each column
    //m_pBrowser->column_widths(br_widths);	
    //m_pBrowser->callback((Fl_Callback *)b_cb);

    m_pSliderCtr = new Fl_Value_Slider(m_gGroupC->x() + 3, 
				      m_pLB->y() + 33, m_pLB->w()*2+6, 25, "");
    m_pSliderCtr->align(FL_ALIGN_RIGHT);
    m_pSliderCtr->labelsize(TEXTSIZE);
    m_pSliderCtr->type(FL_HOR_NICE_SLIDER);
    m_pSliderCtr->range(0.0, 100.0);
    m_pSliderCtr->value(50.0);
    m_pSliderCtr->step(0.01);   
    m_pSliderCtr->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
    m_pSliderCtr->callback((Fl_Callback *)cb_SliderB); 

	m_gGroupC->resizable(m_gGroupC);
    m_gGroupC->end();
	m_pMainWin->resizable(m_pMainWin);
	m_pMainWin->end();
    //m_pMainWin->resizable(NULL);
}


void WinSliceMain::show(int argc, char **argv)
{    
    m_pMainWin->show(argc,argv);
    m_pSliceView->show(argc,argv);
}


inline void WinSliceMain::cb_SliderA_i(Fl_Value_Slider* o, void* v)
{
    if(m_pSliceView == NULL || m_pSliceView->m_pVolume == NULL)
        return;
    
    float pos = o->value();
    
    switch(m_pSliceView->m_nAxis){
        case SLICE_X_AXIS : m_pSliceView->m_pVolume->m_pVisStatus->m_fSlicePosX = pos;
                            break;
        case SLICE_Y_AXIS : m_pSliceView->m_pVolume->m_pVisStatus->m_fSlicePosY = pos;
                            break;
        case SLICE_Z_AXIS : m_pSliceView->m_pVolume->m_pVisStatus->m_fSlicePosZ = pos;
                            break;
    }
        
    
    if (Fl::event_button1()) {
        m_pSliceView->m_pVolume->m_bDraw2DSliceBoundary = true;
        m_pSliceView->m_pVolume->SetSliceAxis(m_pSliceView->m_nAxis);
    }else
        m_pSliceView->m_pVolume->m_bDraw2DSliceBoundary = false;
    m_pSliceView->updateSelection(m_pSliceView->m_nAxis);
	m_pSliceView->m_pVolume->ReDraw();
}  


void WinSliceMain::cb_SliderA(Fl_Value_Slider* o, void* v)
{
  ((WinSliceMain*)(o->parent()->user_data()))->cb_SliderA_i(o,v);
}

inline void WinSliceMain::cb_SliderB_i(Fl_Value_Slider* o, void* v)
{
    if(m_pSliceView == NULL || m_pSliceView->m_pVolume == NULL)
        return;
    m_contour = o->value();

    if (!Fl::event_button1()) {
	m_pSliceView->m_pVolume->EnableLineContours(m_bLBseleced); 
    	m_pSliceView->m_pVolume->CalculateContourLines(m_contour, m_pBrowser->size()); 
    	m_pSliceView->m_pVolume->ReDraw();
    }
}  


void WinSliceMain::cb_SliderB(Fl_Value_Slider* o, void* v)
{
  ((WinSliceMain*)(o->parent()->user_data()))->cb_SliderB_i(o,v);
   
}

inline void WinSliceMain::cb_SliderTAC_i(Fl_Value_Slider* o, void* v)
{
    if(m_pSliceView == NULL || m_pSliceView->m_pVolume == NULL)
        return;
    //m_TAC_finish = o->value();
    m_TAC_finish = (int)((float)o->value() *  (float)m_pSliceView->m_pVolume->m_netcdf->get_num_timesteps(m_pSliceView->m_pVolume->m_curnetcdf_var));
    if (!Fl::event_button1()) {
	m_pSliceView->m_pVolume->SetTAC_finish(m_TAC_finish);
    }
}  


void WinSliceMain::cb_SliderTAC(Fl_Value_Slider* o, void* v)
{
  ((WinSliceMain*)(o->parent()->user_data()))->cb_SliderTAC_i(o,v);
}


void WinSliceMain::cb_RadioA_i(Fl_Round_Button* o, void*)
{
	if(m_pSliceView == NULL || m_pSliceView->m_pVolume == NULL)
		return;
	//m_pSlider->step(2/m_pSliceView->m_pVolume->m_vSize[0]);
	SetVolume(m_pSliceView->m_pVolume, SLICE_X_AXIS);
	m_pSliceView->m_pVolume->CalculateContourLines(m_contour, m_pBrowser->size()); 
	m_pSliceView->m_pVolume->ReDraw();
	m_pSliceView->updateSelection(SLICE_X_AXIS);
    	//SaveConfig();	
}

void WinSliceMain::cb_RadioA(Fl_Round_Button* o, void* v) {
  ((WinSliceMain*)(o->parent()->user_data()))->cb_RadioA_i(o,v);
}

void WinSliceMain::cb_RadioB_i(Fl_Round_Button* o, void*)
{
	if(m_pSliceView == NULL || m_pSliceView->m_pVolume == NULL)
		return;
	//m_pSlider->step(2/m_pSliceView->m_pVolume->m_vSize[1]);
	SetVolume(m_pSliceView->m_pVolume, SLICE_Y_AXIS);
	m_pSliceView->m_pVolume->CalculateContourLines(m_contour, m_pBrowser->size()); 
	m_pSliceView->m_pVolume->ReDraw();
	m_pSliceView->updateSelection(SLICE_Y_AXIS);
    	//SaveConfig();	
}

void WinSliceMain::cb_RadioB(Fl_Round_Button* o, void* v) {
  ((WinSliceMain*)(o->parent()->user_data()))->cb_RadioB_i(o,v);
}

void WinSliceMain::cb_RadioC_i(Fl_Round_Button* o, void*)
{
	if(m_pSliceView == NULL || m_pSliceView->m_pVolume == NULL)
		return;
	//m_pSlider->step(2/m_pSliceView->m_pVolume->m_vSize[2]);
	SetVolume(m_pSliceView->m_pVolume, SLICE_Z_AXIS);
	m_pSliceView->m_pVolume->CalculateContourLines(m_contour, m_pBrowser->size()); 
	m_pSliceView->m_pVolume->ReDraw();
	m_pSliceView->updateSelection(SLICE_Z_AXIS);
    	//SaveConfig();	
}

void WinSliceMain::cb_LB_corr_i(Fl_Light_Button* o, void* v){
	m_pSliceView->setCorrMode(!(m_pSliceView->getCorrMode()));
}

void WinSliceMain::cb_LB_corr(Fl_Light_Button* o, void* v){
	((WinSliceMain*)(o->parent()->user_data()))->cb_LB_corr_i(o,v);
}

void WinSliceMain::cb_LB(Fl_Light_Button* o, void* v) {
  ((WinSliceMain*)(o->parent()->user_data()))->cb_LB_i(o,v);
}

void WinSliceMain::cb_LB_i(Fl_Light_Button* o, void*)
{
	if(m_pSliceView == NULL || m_pSliceView->m_pVolume == NULL)
		return;
	m_bLBseleced = o->value();
	
	m_pSliceView->m_pVolume->EnableLineContours(m_bLBseleced); 
	if(m_bLBseleced)
		m_pSliceView->m_pVolume->CalculateContourLines(m_contour, m_pBrowser->size()); 
	m_pSliceView->m_pVolume->ReDraw();
	
	//SaveConfig();	
}

void WinSliceMain::cb_LB_color(Fl_Light_Button* o, void* v) {
  ((WinSliceMain*)(o->parent()->user_data()))->cb_LB_color_i(o,v);
}

void WinSliceMain::cb_LB_color_i(Fl_Light_Button* o, void*)
{
	if(m_pSliceView == NULL || m_pSliceView->m_pVolume == NULL)
		return;
	m_bLBseleced = o->value();
	
	m_pSliceView->m_pVolume->EnableColorContours(m_bLBseleced); 
	//if(m_bLBseleced)
	//	m_pSliceView->m_pVolume->CalculateContourLines(m_contour, m_pBrowser->size()); 
	m_pSliceView->m_pVolume->ReDraw();
	
	//SaveConfig();	
}

void WinSliceMain::cb_SaveButton(Fl_Button* o, void* v) {
  ((WinSliceMain*)(o->parent()->user_data()))->cb_SaveButton_i(o,v);
}

void WinSliceMain::cb_SaveButton_i(Fl_Button* o, void*)
{
	if(m_pSliceView == NULL || m_pSliceView->m_pVolume == NULL)
		return;
	if(m_bLBseleced){
		char str[20];
		sprintf(str,"%f",m_contour);
		m_pBrowser->topline(0);
		for(int t=1; t<= m_pBrowser->size(); t++){
			const char* item = m_pBrowser->text(t);
			if(item && strcmp(item,str)==0)
				return;
		}
		m_pBrowser->add(str);
		m_pSliceView->m_pVolume->CalculateContourLines(m_contour, m_pBrowser->size()); 
		m_pSliceView->m_pVolume->ReDraw();
	}
}

void WinSliceMain::cb_SaveTAC(Fl_Button* o, void* v) {
  ((WinSliceMain*)(o->parent()->user_data()))->cb_SaveTAC_i(o,v);
}

void WinSliceMain::cb_SaveTAC_i(Fl_Button* o, void*)
{
	if(m_pSliceView == NULL || m_pSliceView->m_pVolume == NULL)
		return;
	m_pSliceView->m_pVolume->SaveTAC_vol();
}

void WinSliceMain::cb_SaveGTAC(Fl_Button* o, void* v) {
  ((WinSliceMain*)(o->parent()->user_data()))->cb_SaveGTAC_i(o,v);
}

void WinSliceMain::cb_SaveGTAC_i(Fl_Button* o, void*)
{
	if(m_pSliceView == NULL || m_pSliceView->m_pVolume == NULL)
		return;
	m_pSliceView->m_pVolume->SaveGTAC_vol();
}

void WinSliceMain::cb_ClearButton(Fl_Button* o, void* v) {
  ((WinSliceMain*)(o->parent()->user_data()))->cb_ClearButton_i(o,v);
}

void WinSliceMain::cb_ClearButton_i(Fl_Button* o, void*)
{
	if(m_pSliceView == NULL || m_pSliceView->m_pVolume == NULL)
		return;
	if(m_bLBseleced){
		m_pBrowser->clear();
		m_pSliceView->m_pVolume->CalculateContourLines(m_contour, m_pBrowser->size()); 
		m_pSliceView->m_pVolume->ReDraw();
	}
}

void WinSliceMain::cb_RadioC(Fl_Round_Button* o, void* v) {
  ((WinSliceMain*)(o->parent()->user_data()))->cb_RadioC_i(o,v);
}

WinSliceView::WinSliceView(int x,int y,int w,int h,const char *l):Fl_Gl_Window(x,y,w,h,l)
{
    m_pVolume = NULL;
    m_nAxis = SLICE_X_AXIS;
    m_bBlack = true;
    m_bRegion = false;
	m_bcorr = false;
	m_crossx = m_crossy = 0.0;
}


WinSliceView::~WinSliceView()
{

}

void WinSliceView::updateSelection(int axis)
{
	//m_pVolume->SetSelected2DRegion(	axis, (m_startx + 0.75)/1.5, 
	//				(m_starty + 0.75)/1.5, 
	//				(m_finishx + 0.75)/1.5, 
	//				(m_finishy + 0.75)/1.5);
	m_nAxis = axis;
	if(m_pVolume){
		if (!(m_pVolume && m_pVolume->m_bCorrelation/*m_bcorr*/) && m_bRegion) {
			m_pVolume->SetSelected2DRegion(	m_nAxis, (m_startx + 0.75)/1.5, 
							(1-(m_starty + 0.75)/1.5), 
							(m_finishx + 0.75)/1.5, 
							(1-(m_finishy + 0.75)/1.5));
			m_pVolume->EnableSelection(true);
		}
		if((m_pVolume && m_pVolume->m_bCorrelation/*m_bcorr*/)){
			m_pVolume->SetCorrCenterPoint(m_nAxis, (m_crossx + 0.75)/1.5, (m_crossy + 0.75)/1.5);
		}
	}
	//m_pVolume->EnableSelection(true);
}

int WinSliceView::handle(int event1)
{
    int ret = 1;
    //int ret = Fl_Gl_Window::handle(event1);
#ifndef LINUX
    int mouse_button = Fl::event_button();
#endif
	/*
    if(event1 == FL_DRAG)
	printf("mouse (drag) button %d: event FL_DRAG\n",mouse_button);
    else
	printf("mouse button %d: event %d\n",mouse_button, event1);
    */

    switch(event1) {
	case FL_DRAG:
	{
#ifdef LINUX
		if(Fl::event_button1()){
#else
		if (mouse_button == FL_LEFT_MOUSE ){//Fl::event_button1()
#endif
			if(!(m_pVolume && m_pVolume->m_bCorrelation/*m_bcorr*/)){
				m_bRegion = true;
			}
			
			if(m_pVolume){
				update();
				if(m_pVolume->m_bCorrelation/*m_bcorr*/){
					m_crossx = m_finishx;
					m_crossy = m_finishy;
					m_pVolume->SetCorrCenterPoint(m_nAxis, (m_finishx + 0.75)/1.5, (m_finishy + 0.75)/1.5);
				}
			}
#ifdef LINUX
		} else if (Fl::event_button3()){
#else
		} else if (mouse_button == 3){ //Fl::event_button2())
#endif
				//m_bRegion = false;
				//m_pVolume->EnableSelection(m_bRegion);
				if(!(m_pVolume && m_pVolume->m_bCorrelation/*m_bcorr*/)){
				m_bRegion = true;
				
				float centerx = (m_finishx + m_startx)/2.0;
    				float centery = (m_finishy + m_starty)/2.0;
    				float diffx = ((float)Fl::event_x()/(float)w()*2.0-1.0) - centerx;
    				float diffy = ((float)Fl::event_y()/(float)h()*2.0-1.0) - centery;
    				//printf("diffx -> %f, diffy -> %f\n",diffx, diffy);
				moveupdate1(diffx, diffy);
				
				//moveupdate();
				m_pVolume->SetSelected2DRegion(	m_nAxis, (m_startx + 0.75)/1.5, 
									(1-(m_starty + 0.75)/1.5), 
									(m_finishx + 0.75)/1.5, 
									(1-(m_finishy + 0.75)/1.5));
				m_pVolume->EnableSelection(true);
			}
		}     
		
		redraw();
		return 1;
	}
	case FL_PUSH:
		{
		//m_bRegion = false;
		//m_pVolume->EnableSelection(m_bRegion);
		redraw();
		if (Fl::event_button1()) {
			
			initline();
			if((m_pVolume && m_pVolume->m_bCorrelation/*m_bcorr*/)){
				m_pVolume->SetCorrCenterPoint(m_nAxis, (m_startx + 0.75)/1.5, (m_starty + 0.75)/1.5);
			}
			//else
			//	set_corr_center();
		} 
		//else if (mouse_button == 3){
		//	//printf("received FL_PUSH for FL_RIGHT_MOUSE\n");
		//}

		//else if (Fl::event_button3()) {
		//	
		//}
	
		//redraw();
		break;
		}
	case FL_RELEASE:
		{
		float temp;
		//m_bRegion = false;
		m_finishx = (float)Fl::event_x()/(float)w()*2.0-1.0;
		m_finishy = (float)Fl::event_y()/(float)h()*2.0-1.0;
		
		if(m_finishx > 0.75){
			//m_startx = 0.75 - oldlenx;
			m_finishx = 0.75;
		}
		else if(m_finishx < -0.75){
			//m_startx = -0.75;
			m_finishx = -0.75;
		}
		
		if(m_finishy > 0.75){
			//m_startx = 0.75 - oldlenx;
			m_finishy = 0.75;
		}
		else if(m_finishy < -0.75){
			//m_startx = -0.75;
			m_finishy = -0.75;
		}
		
		if(m_finishx < m_startx){
			temp = m_startx;
			m_startx = m_finishx;
			m_finishx = temp;
		}
		
		if(m_finishy < m_starty){
			temp = m_starty;
			m_starty = m_finishy;
			m_finishy = temp;
		}
		/*
		float centerx = (m_finishx + m_startx)/2.0;
		float centery = (m_finishy + m_starty)/2.0;
		float diffx = ((float)Fl::event_x()/(float)w()*2.0-1.0) - centerx;
		float diffy = ((float)Fl::event_y()/(float)h()*2.0-1.0) - centery;
		//printf("diffx -> %f, diffy -> %f\n",diffx, diffy);
		moveupdate1(diffx, diffy);
    		*/
		if (!(m_pVolume && m_pVolume->m_bCorrelation/*m_bcorr*/) && m_bRegion) {
			m_pVolume->SetSelected2DRegion(	m_nAxis, (m_startx + 0.75)/1.5, 
						(1-(m_starty + 0.75)/1.5), 
						(m_finishx + 0.75)/1.5, 
						(1-(m_finishy + 0.75)/1.5));
			m_pVolume->EnableSelection(true);
		}

		if((m_pVolume && m_pVolume->m_bCorrelation/*m_bcorr*/)){
			m_crossx = m_finishx;
			m_crossy = m_finishy;
			m_pVolume->SetCorrCenterPoint(m_nAxis, (m_finishx + 0.75)/1.5, (m_finishy + 0.75)/1.5);
		}
		//return 1;
		break;
		}

	default:
		ret = Fl_Gl_Window::handle(event1);
		break;
    }
    return ret;
}

void WinSliceView::update()
{
    m_finishx = (float)Fl::event_x()/(float)w()*2.0-1.0;
    if(m_finishx > 0.75)
	m_finishx = 0.75;
    else if(m_finishx < -0.75)
	m_finishx = -0.75;
    m_finishy = (float)Fl::event_y()/(float)h()*2.0-1.0;
    if(m_finishy > 0.75)
	m_finishy = 0.75;
    else if(m_finishy < -0.75)
	m_finishy = -0.75;

	m_crossx = m_finishx;
	m_crossy = m_finishy;
}


void WinSliceView::moveupdate()
{
    float centerx = (m_finishx + m_startx)/2.0;
    float centery = (m_finishy + m_starty)/2.0;
    float diffx = ((float)Fl::event_x()/(float)w()*2.0-1.0) - centerx;
    float diffy = ((float)Fl::event_y()/(float)h()*2.0-1.0) - centery;
    float oldlenx = m_finishx - m_startx;
    float oldleny = m_finishy - m_starty; 
    m_finishx += diffx;
    m_finishy += diffy;
    m_startx += diffx;
    m_starty += diffy;
    
    if(m_finishx > 0.75){
    	m_startx = 0.75 - oldlenx;
	m_finishx = 0.75;
    }
    else if(m_finishx < -0.75+oldlenx){
    	m_startx = -0.75;
	m_finishx = -0.75 + oldlenx;
    }
    
    if(m_finishy > 0.75){
    	m_starty = 0.75 - oldleny;
	m_finishy = 0.75;
    }
    else if(m_finishy < -0.75 + oldleny){
        m_starty = -0.75;
	m_finishy = -0.75 + oldleny;
    }
    
    if(m_startx > 0.75){
    	m_finishx = 0.75 - oldlenx;
	m_startx = 0.75;
    }
    else if(m_startx < -0.75){
        m_finishx = -0.75 + oldlenx;
	m_startx = -0.75;
    }
    if(m_starty > 0.75){
        m_finishy = 0.75 - oldleny;
	m_starty = 0.75;
    }
    else if(m_starty < -0.75){
    	m_finishy = -0.75 + oldleny;
	m_starty = -0.75;
    }
}

/*
void WinSliceView::moveupdate1(float diffx, float diffy)
{
    float oldlenx = m_finishx - m_startx;
    float oldleny = m_finishy - m_starty; 
    m_finishx += diffx;
    m_finishy += diffy;
    m_startx += diffx;
    m_starty += diffy;
    
    if(m_finishx > 0.75){
    	m_startx = 0.75 - oldlenx;
	m_finishx = 0.75;
    }
    else if(m_finishx < -0.75+oldlenx){
    	m_startx = -0.75;
	m_finishx = -0.75 + oldlenx;
    }
    
    if(m_finishy > 0.75){
    	m_starty = 0.75 - oldleny;
	m_finishy = 0.75;
    }
    else if(m_finishy < -0.75 + oldleny){
        m_starty = -0.75;
	m_finishy = -0.75 + oldleny;
    }
    
    if(m_startx > 0.75){
    	m_finishx = 0.75 - oldlenx;
	m_startx = 0.75;
    }
    else if(m_startx < -0.75){
        m_finishx = -0.75 + oldlenx;
	m_startx = -0.75;
    }
    if(m_starty > 0.75){
        m_finishy = 0.75 - oldleny;
	m_starty = 0.75;
    }
    else if(m_starty < -0.75){
    	m_finishy = -0.75 + oldleny;
	m_starty = -0.75;
    }
}
*/
void WinSliceView::moveupdate1(float diffx, float diffy)
{
    float temp;
    float oldlenx = m_finishx - m_startx;
    float oldleny = m_finishy - m_starty; 
    m_finishx += diffx;
    m_finishy += diffy;
    m_startx += diffx;
    m_starty += diffy;
    
    if(m_finishx < m_startx){
    	temp = m_startx;
    	m_startx = m_finishx;
    	m_finishx = temp;
    }
    
    if(m_finishy < m_starty){
    	temp = m_starty;
    	m_starty = m_finishy;
    	m_finishy = temp;
    }
    
    if(m_finishx > 0.75){
    	m_startx = 0.75 - oldlenx;
	m_finishx = 0.75;
    }
    else if(m_finishx < -0.75){
    	m_startx = -0.75;
	m_finishx = -0.75 + oldlenx;
    }
    
    if(m_finishy > 0.75){
    	m_starty = 0.75 - oldleny;
	m_finishy = 0.75;
    }
    else if(m_finishy < -0.75){
        m_starty = -0.75;
	m_finishy = -0.75 + oldleny;
    }
    
    if(m_startx > 0.75){
    	m_finishx = 0.75 - oldlenx;
	m_startx = 0.75;
    }
    else if(m_startx < -0.75){
        m_finishx = -0.75 + oldlenx;
	m_startx = -0.75;
    }
    if(m_starty > 0.75){
        m_finishy = 0.75 - oldleny;
	m_starty = 0.75;
    }
    else if(m_starty < -0.75){
    	m_finishy = -0.75 + oldleny;
	m_starty = -0.75;
    }
}

void WinSliceView::initline(bool is_erase)
{
	m_startx = (float)Fl::event_x()/(float)w()*2.0-1.0;
	if(m_startx > 0.75)
		m_startx = 0.75;
	else if(m_startx < -0.75)
		m_startx = -0.75;
    
	m_starty = (float)Fl::event_y()/(float)h()*2.0-1.0;
	if(m_starty > 0.75)
		m_starty = 0.75;
	else if(m_starty < -0.75)
		m_starty = -0.75;

	m_crossx = m_startx;
	m_crossy = m_starty;
}


void WinSliceView::draw()
{

     if(!valid())
     {
	glViewport(0,0,w(),h());
	
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
	// Problem: glewInit failed, something is seriously wrong.
	fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
/* 
	// verify FBOs are supported (otherwise we get FuBar'd Objects)
	if (!glutExtensionSupported ("GL_EXT_framebuffer_object") )
	{
	cerr << "FBO extension unsupported" << endl;
	//exit (1);
	}
*/
    }

	// Set background color
	// Set background color
    if (m_bBlack)   
       glClearColor( 0, 0, 0, 1 );
    else
       glClearColor( 1, 1, 1, 1 );
       glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
#ifndef LINUX
       glDisable(GL_DEPTH_TEST);
#endif
       glMatrixMode(GL_PROJECTION);	
       glLoadIdentity();
       glMatrixMode(GL_MODELVIEW);
       glLoadIdentity();
	
// 	glScalef(2, 2, 1);
//     	glTranslatef(-0.5, -0.5, 0);
//     
// 	glColor3f(1, 0, 0);
// 	
// 	glBegin(GL_QUADS);
// 		glVertex2f(0, 0);
// 		glVertex2f(1, 0);
// 		glVertex2f(1, 1);
// 		glVertex2f(0, 1);
// 	glEnd();
       if (m_pVolume != NULL) {
       //m_pVolume->SetSliceAxis(m_nAxis); 
       m_pVolume->Draw(DRAW_2D_SLICE, m_nAxis, h(), w());
    }
    
    if(!(m_pVolume && m_pVolume->m_bCorrelation/*m_bcorr*/) && m_bRegion)
    {
	glLineWidth(4.0);
	glColor3f(0.0f,0.0f,0.0f);
	glBegin(GL_LINE_LOOP);
		glVertex2f(m_startx, -m_starty);
		glVertex2f(m_startx, -m_finishy);
		glVertex2f(m_finishx, -m_finishy);
		glVertex2f(m_finishx,-m_starty);
	glEnd();

	glLineWidth(3.0);
	glColor3f(1.0f,1.0f,0.0f);
	glBegin(GL_LINE_LOOP);
		glVertex2f(m_startx, -m_starty);
		glVertex2f(m_startx, -m_finishy);
		glVertex2f(m_finishx, -m_finishy);
		glVertex2f(m_finishx,-m_starty);
	glEnd();

	char text[200];
	gl_font(FL_HELVETICA_BOLD, 12);
	gl_color(FL_RED);
	int x_size = m_pVolume->GetSizeSlice(m_nAxis, 'x');
	int y_size = m_pVolume->GetSizeSlice(m_nAxis, 'y');

	if(x_size != 0)
		x_size--;
	if(y_size != 0)
		y_size--;

	float s_offsetx = -0.07, f_offsetx = 0.02, s_offsety = 0.02, f_offsety = -0.06;
	if(m_starty > m_finishy)
	{
		s_offsety = -0.06;
		f_offsety = 0.02;
	}

	if(m_startx > m_finishx)
	{
		s_offsetx = 0.02;
		f_offsetx = -0.07;
	}

	sprintf(text,"%d, %d",(int)((m_startx + 0.75)/1.5 * x_size), (int)((1-(m_starty + 0.75)/1.5) * y_size));
	gl_draw(text, float(m_startx + s_offsetx), float(-m_starty + s_offsety));	

	sprintf(text,"%d, %d",(int)((m_startx + 0.75)/1.5 * x_size), (int)((1-(m_finishy + 0.75)/1.5) * y_size));
	gl_draw(text, float(m_startx + s_offsetx), float(-m_finishy + f_offsety));

	sprintf(text,"%d, %d",(int)((m_finishx + 0.75)/1.5 * x_size), (int)((1-(m_finishy + 0.75)/1.5) * y_size));
	gl_draw(text, float(m_finishx + f_offsetx), float(-m_finishy + f_offsety));

	sprintf(text,"%d, %d",(int)((m_finishx + 0.75)/1.5 * x_size), (int)((1-(m_starty + 0.75)/1.5) * y_size));
	gl_draw(text, float(m_finishx + f_offsetx), float(-m_starty + s_offsety));

     }
     else if(m_pVolume && m_pVolume->m_bCorrelation/*m_bcorr*/){
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);    // Use The Good Calculations
	glEnable(GL_LINE_SMOOTH);            // Enable Anti-Aliasing
	glShadeModel(GL_SMOOTH);  // Because each line is the same color.

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glLineWidth(5.0);
	glColor3f(0.0f,0.0f,0.0f);
	glBegin(GL_LINES);
		glVertex2f(m_crossx-0.03, -m_crossy);
		glVertex2f(m_crossx+0.03, -m_crossy);
	glEnd();
	glBegin(GL_LINES);
		glVertex2f(m_crossx, -m_crossy-0.1);
		glVertex2f(m_crossx, -m_crossy+0.1);
	glEnd();
	glLineWidth(2.0);
	glColor3f(1.0f,1.0f,0.0f);
	glBegin(GL_LINES);
		glVertex2f(m_crossx-0.03, -m_crossy);
		glVertex2f(m_crossx+0.03, -m_crossy);
	glEnd();
	glBegin(GL_LINES);
		glVertex2f(m_crossx, -m_crossy-0.1);
		glVertex2f(m_crossx, -m_crossy+0.1);
	glEnd();

	glShadeModel(GL_SMOOTH);
	glDisable(GL_LINE_SMOOTH);	
     }
}


