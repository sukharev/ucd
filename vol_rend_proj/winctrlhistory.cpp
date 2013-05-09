//
// C++ Implementation: winctrlhistory
//
// Description: 
//
//
// Author: Hongfeng Yu <hfyu@ucdavis.edu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "winctrlhistory.h"
#include "winvisview.h"

#define TEXTSIZE 12
#define TIME_OUT  0.01

cxSnapshot::cxSnapshot(int x, int y, int w, int h):Fl_Button(x, y, w, h)
{
    m_pVisStatus = NULL;
    m_pActiveImage = NULL;
    m_pDeactiveImage = NULL;    
}

cxSnapshot::~cxSnapshot()
{
    if ( m_pVisStatus != NULL )
        delete m_pVisStatus;
        
    if ( m_pActiveImage != NULL)
        delete m_pActiveImage;
        
    if ( m_pDeactiveImage != NULL)
        delete m_pDeactiveImage;
}


WinCtrlHistory::WinCtrlHistory(int x = 0, int y =0, int w = 200, int h = 20)
{
    GenWindow(x, y, w, h);    
    m_pVolume = NULL;
    m_pCtrlMain = NULL;
    m_pActiveSnapshot = NULL;
    m_nInterSteps = 20;
}


WinCtrlHistory::~WinCtrlHistory()
{
}

void WinCtrlHistory::show(int argc, char **argv){
    m_wMainWin->show(argc,argv);
}


void WinCtrlHistory::GenWindow(int x, int y, int w, int h)
{   
    m_wMainWin = new Fl_Window(x, y,w,h, "History - Snapshot");
    m_wMainWin->user_data((void*)(this));
    
    m_pScrollA = new Fl_Scroll(0, 0, w - 82, h);
    m_pScrollA->type(Fl_Scroll::HORIZONTAL_ALWAYS);
    m_pScrollA->box(FL_DOWN_BOX);
    m_pScrollA->user_data((void*)(this));
    
    m_pPack = new Fl_Pack(0, -8, w - 82, h);
    m_pPack->user_data((void*)(this));
    m_pPack->type(FL_HORIZONTAL);
    m_pPack->box(FL_DOWN_FRAME);
    
    m_pPack->end();
    
    m_pScrollA->end();
    
    m_pButtonA = new Fl_Button( w-80, 0,  80, 20, "Add");
    m_pButtonA->labelsize(TEXTSIZE);
    m_pButtonA->callback((Fl_Callback *)cb_ButtonA);
    
    m_pButtonB = new Fl_Button( w-80, 20, 80, 20, "Delete");    
    m_pButtonB->labelsize(TEXTSIZE);
    m_pButtonB->callback((Fl_Callback *) cb_ButtonB);
    
    m_pButtonC = new Fl_Button( w-80, 40, 80, 20, "Play");    
    m_pButtonC->labelsize(TEXTSIZE);
    m_pButtonC->callback((Fl_Callback *) cb_ButtonC);
    m_pButtonC->deactivate();
    
    m_pSliderA = new Fl_Slider(w-80, 80, 80, 20, "Speed");
    m_pSliderA->labelsize(TEXTSIZE);
    m_pSliderA->type(FL_HORIZONTAL);
    m_pSliderA->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
    m_pSliderA->range(5, 50);
    m_pSliderA->value(20);
    m_pSliderA->step(1);
    m_pSliderA->callback((Fl_Callback *) cb_SliderA);
    m_pSliderA->deactivate();
            
    m_wMainWin->end();
}

/* 
    Button A
*/
inline void WinCtrlHistory::cb_ButtonA_i(Fl_Button * o, void * v)
{
    int h = m_pScrollA->h() - 20;
    int w = h;    
    int x = 0;
    
    cxSnapshot * pSnapshot = NULL;
            
    /* the position of the new snapshot button*/
    int num = m_lSnapshot.size();
    if ( num > 0) {
        pSnapshot = *(m_lSnapshot.rbegin());
        x = pSnapshot->x() + w;
    }
    
    /* the snapshot*/
    int image_w = w - 2;
    int image_h = h - 2;
    unsigned char * image = new unsigned char[ image_w * image_h * 3];
    assert(m_pVolume);    
    m_pVolume->GetSnapshot(image, image_w, image_h);
    Fl_RGB_Image * rgb = new Fl_RGB_Image(image, image_w, image_h, 3);
    Fl_Image * dergb = rgb->copy();
    dergb->inactive();
    
    /* new snapshot*/
    pSnapshot = new cxSnapshot(x, 0, w, h);    
    pSnapshot->box(FL_PLASTIC_UP_FRAME);    
    pSnapshot->image(rgb);    
    pSnapshot->callback((Fl_Callback *) cb_Snapshot);
    pSnapshot->m_pActiveImage = rgb;
    pSnapshot->m_pDeactiveImage = dergb;
    
    pSnapshot->m_pVisStatus = new cxVisStatus();
    pSnapshot->m_pVisStatus->Copy(m_pVolume->m_pVisStatus);
        
    m_lSnapshot.push_back(pSnapshot);
    pSnapshot->m_pIter = m_lSnapshot.end();
    pSnapshot->m_pIter--;
    
    m_pPack->add(pSnapshot);
    m_pScrollA->redraw();
    
    if (m_pActiveSnapshot != pSnapshot) {
        if (m_pActiveSnapshot)
            m_pActiveSnapshot->image(m_pActiveSnapshot->m_pDeactiveImage);
        m_pActiveSnapshot = pSnapshot;
    }
    
    if (m_lSnapshot.size() > 1) {
        m_pButtonC->activate();
        m_pSliderA->activate();
    }
    
    delete [] image;  
}

void WinCtrlHistory::cb_ButtonA(Fl_Button * o, void * v)
{
    ((WinCtrlHistory*)(o->parent()->user_data()))->cb_ButtonA_i(o,v);
}


/*
    Button B
*/
inline void WinCtrlHistory::cb_ButtonB_i(Fl_Button * o, void * v)
{
    list<cxSnapshot *>::iterator iter;

    
    if (m_pActiveSnapshot != NULL) {
        
        iter = m_pActiveSnapshot->m_pIter;
                
        if (m_pActiveSnapshot->m_pIter != m_lSnapshot.begin())
            iter--;
        else
            iter++;
        
        m_lSnapshot.erase(m_pActiveSnapshot->m_pIter);
        m_pPack->remove(m_pActiveSnapshot);
        delete m_pActiveSnapshot;
        m_pActiveSnapshot = NULL;
    }
    
    if (m_lSnapshot.size() != 0) {
        m_pActiveSnapshot = *(iter);
        m_pActiveSnapshot->image(m_pActiveSnapshot->m_pActiveImage);
        
        m_pVolume->m_pVisStatus->Copy(m_pActiveSnapshot->m_pVisStatus);
        UpdateView(m_pActiveSnapshot->m_pVisStatus);
        UpdateCtrl(m_pActiveSnapshot->m_pVisStatus);
        m_pVolume->ReDraw();
    }
    
    if (m_lSnapshot.size() < 2) {
        m_pButtonC->deactivate();
        m_pSliderA->deactivate();
    }
    
    m_pScrollA->redraw();
}

void WinCtrlHistory::cb_ButtonB(Fl_Button * o, void * v)
{
    ((WinCtrlHistory*)(o->parent()->user_data()))->cb_ButtonB_i(o,v);
}


/* 
    Button C
*/

inline void WinCtrlHistory::cb_ButtonC_i(Fl_Button * o, void * v)
{
    if ( strcmp(o->label(),"Play") == 0 && m_lSnapshot.size() > 1){
        o->label("Pause");
        m_pButtonA->deactivate();
        m_pButtonB->deactivate();
        m_pCtrlMain->DeActivate();
        
        m_iCur = m_lSnapshot.begin();
        
        m_nFrameCount = 0;
        
        m_pVolume->m_pVisView->m_bSaveImage = true;
        
        Fl::add_timeout(TIME_OUT, cb_wPlayOn, this);
    }else{
        o->label("Play");        
        m_pButtonA->activate();
        m_pButtonB->activate();
        m_pCtrlMain->Activate();
        m_pVolume->m_pVisView->m_bSaveImage = false;
        Fl::remove_timeout(cb_wPlayOn, this);
    }
    
}

void WinCtrlHistory::cb_ButtonC(Fl_Button * o, void * v)
{
    ((WinCtrlHistory*)(o->parent()->user_data()))->cb_ButtonC_i(o,v);
}

/* 
    Slider A
*/

inline void WinCtrlHistory::cb_SliderA_i(Fl_Slider * o, void * v)
{
    m_nInterSteps = o->value();
}

void WinCtrlHistory::cb_SliderA(Fl_Slider * o, void * v)
{
    ((WinCtrlHistory*)(o->parent()->user_data()))->cb_SliderA_i(o,v);
}


/* 
    Snapshot
*/

inline void WinCtrlHistory::cb_Snapshot_i(cxSnapshot * o, void * v)
{
    if (m_pVolume == NULL)
        return;
        
    m_pVolume->m_pVisStatus->Copy(o->m_pVisStatus);
    UpdateView(o->m_pVisStatus);
    UpdateCtrl(o->m_pVisStatus);
    m_pVolume->ReDraw();
    
    if (m_pActiveSnapshot != o) {
        if (m_pActiveSnapshot) 
            m_pActiveSnapshot->image(m_pActiveSnapshot->m_pDeactiveImage);
        m_pActiveSnapshot = o;
        m_pActiveSnapshot->image(m_pActiveSnapshot->m_pActiveImage);
        m_pScrollA->redraw();
    }
}

void WinCtrlHistory::cb_Snapshot(cxSnapshot * o, void * v)
{
    ((WinCtrlHistory*)(o->parent()->user_data()))->cb_Snapshot_i(o,v);
}


void WinCtrlHistory::UpdateView(cxVisStatus * pVisStatus) 
{
    if (m_pVolume == NULL)
        return;
        
    memcpy(m_pVolume->m_pVisView->m_curquat, pVisStatus->m_curquat, sizeof(float) * 4);
    m_pVolume->m_pVisView->m_scale = pVisStatus->m_scale;
    m_pVolume->m_pVisView->m_deltax = pVisStatus->m_deltax;
    m_pVolume->m_pVisView->m_deltay = pVisStatus->m_deltay;
    
    
}

void WinCtrlHistory::UpdateCtrl(cxVisStatus * pVisStatus)
{
    if (m_pCtrlMain == NULL)
        return;
        
    /* update the colormap*/
    m_pCtrlMain->m_wColorMap->LoadColorFromVolume();
    
    /* update the ctrlview*/
    m_pCtrlMain->m_wCtrlView->m_pCheckA->value(pVisStatus->m_bDrawAxes);
    m_pCtrlMain->m_wCtrlView->m_pCheckB->value(pVisStatus->m_bDrawFrame);
    m_pCtrlMain->m_wCtrlView->m_pCheckC->value(pVisStatus->m_bDrawVolume);
    
    if (m_pCtrlMain->m_wCtrlView->m_pRoundA->value())
        m_pCtrlMain->m_wCtrlView->m_pRoundA->do_callback();
    
    if (m_pCtrlMain->m_wCtrlView->m_pRoundB->value())
        m_pCtrlMain->m_wCtrlView->m_pRoundB->do_callback();
        
    if (m_pCtrlMain->m_wCtrlView->m_pRoundC->value())
        m_pCtrlMain->m_wCtrlView->m_pRoundC->do_callback();
        
    /* update the ctrllight*/    
    m_pCtrlMain->m_wCtrlLight->m_pSliderA->value(pVisStatus->m_fLightPar[0]);
    m_pCtrlMain->m_wCtrlLight->m_pSliderB->value(pVisStatus->m_fLightPar[1]);
    m_pCtrlMain->m_wCtrlLight->m_pSliderC->value(pVisStatus->m_fLightPar[2]);
    m_pCtrlMain->m_wCtrlLight->m_pSliderD->value(pVisStatus->m_fLightPar[3]);
    m_pCtrlMain->m_wCtrlLight->m_pSliderE->value(pVisStatus->m_fSampleSpacing);
    
    
}


void cb_wPlayOn(void * pData)
{
    if (pData == NULL)
        return;
        
    /* get the contrl point*/
    WinCtrlHistory* pCtrl = reinterpret_cast<WinCtrlHistory*>(pData);        
    
    /* get the volume point*/
    cxVolume * pVolume = pCtrl->m_pVolume;            
    
    /* return if non volume*/
    if (pVolume == NULL)
        return;            
        
    /* return if the snapshot list size is less than 2*/
    if (pCtrl->m_lSnapshot.size() < 2)
        return;
    
    static unsigned int frame_num = 0;
        
    /* return if the view has not updated yet*/
    if ( pCtrl->m_pVolume->m_pVisView->m_nFrameNum == frame_num) {
        Fl::repeat_timeout(TIME_OUT, cb_wPlayOn, pData);
        return;
    }
        
    unsigned int nInterSteps = pCtrl->m_nInterSteps;
    
    /* get the current frame number*/
    frame_num = pCtrl->m_pVolume->m_pVisView->m_nFrameNum;
    
    /* calcuate the interpolation weight*/
    float weight = (float)(pCtrl->m_nFrameCount % nInterSteps) / (nInterSteps - 1);
    
    /* get the current and next status*/    
    list <cxSnapshot *>::iterator prev, next, temp;
    prev = pCtrl->m_iCur;
    temp = pCtrl->m_iCur;
    temp++;
    next = temp;
    
    assert( next != pCtrl->m_lSnapshot.end());        
        
    cxSnapshot * pPrev = *prev;
    cxSnapshot * pNext = *next;
        
    cxVisStatus visStatus;
    visStatus.Interpolate(pPrev->m_pVisStatus, pNext->m_pVisStatus, 1.0 - weight);
    
    pVolume->m_pVisStatus->Copy(&visStatus);
    pCtrl->UpdateView(&visStatus);
    pCtrl->UpdateCtrl(&visStatus);
    pVolume->ReDraw();
    
    if ( pCtrl->m_nFrameCount % nInterSteps == nInterSteps - 1){
        pCtrl->m_iCur++;
        
        temp = pCtrl->m_iCur;
        temp++;
        
        if (temp == pCtrl->m_lSnapshot.end())
            pCtrl->m_iCur = pCtrl->m_lSnapshot.begin();
    }
    
    if (pCtrl->m_pActiveSnapshot != *(pCtrl->m_iCur)) {
        if (pCtrl->m_pActiveSnapshot) 
            pCtrl->m_pActiveSnapshot->image(pCtrl->m_pActiveSnapshot->m_pDeactiveImage);
        pCtrl->m_pActiveSnapshot = *(pCtrl->m_iCur);
        pCtrl->m_pActiveSnapshot->image(pCtrl->m_pActiveSnapshot->m_pActiveImage);
        pCtrl->m_pScrollA->redraw();
    }
    
    pCtrl->m_nFrameCount++;
    
    Fl::repeat_timeout(TIME_OUT, cb_wPlayOn, pData);

}

