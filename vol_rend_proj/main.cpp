//
// C++ Implementation: main.cpp
//
// Description: 
//
//
// Author: Hongfeng Yu <hfyu@ucdavis.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include "winvismain.h"
#include "winvisview.h"
#include "winsliceview.h"
#include "cxvolume.h"
#include "wincolormap.h"
#include "winctrlview.h"
#include "winctrlmain.h"
#include "winctrllight.h"
#include "winctrlhistory.h"
#include "winctrlnetcdf.h"
#include <string>

using namespace std;

#define TITLE 27
#define BORDER 10

#ifdef PUBLIC_VERSION
    #define WIN_SIZE    512
#else
    #define WIN_SIZE    768
#endif

#ifdef DRAW_LARGEIMAGE
    #define MAINWIN_SIZE    1024//512
#else
    #define MAINWIN_SIZE    1600/3*2//1400/3*2//1600/3*2 //768 can only be used when WIN_SLICE is undefined
#endif


enum side{myright, myleft};

WinVisMain *CreateMainWindow(void);
WinCtrlMain * CreateCtrlMainWindow(WinVisMain *wMain, cxVolume * pVolume, int begin_y, int hight, side sd, int &end_y);
WinCtrlMain * CreateCtrlMainNetCDFWindow(WinCtrlNetCDF *wMain, cxVolume * pVolume, int begin_y, int hight, side sd, int &end_y);

WinSliceMain * CreateSliceViewWindow(WinCtrlNetCDF *wMain, int axis);
WinCtrlHistory * CreateCtrlHistoryWindow(WinVisMain *wMain, cxVolume * pVolume, WinCtrlMain * pCtrlMain);
WinCtrlNetCDF * CreateCtrlNetCDFWindow(cxVolume * pVolume, int begin_y, int hight, side sd, int &end_y);

int single_variable(int argc, char **argv)
{
    //create a volume object   
    cxVolume * pVolume = new cxVolume(); 
    
    int end_y = 0;

    //WinVisMain * wVisMain = CreateMainWindow();
    
    //wVisMain->SetVolume(pVolume);
 
    WinCtrlNetCDF * wCtrlNetCDF = CreateCtrlNetCDFWindow(pVolume, 0, 900, myright, end_y);
    wCtrlNetCDF->show(argc,argv);


#ifdef HISTOGRAM
    WinCtrlMain * wCtrlMain = CreateCtrlMainNetCDFWindow(wCtrlNetCDF, pVolume, 0, 405, myright, end_y);    
#else
    WinCtrlMain * wCtrlMain = CreateCtrlMainNetCDFWindow(wCtrlNetCDF, pVolume, 0, 600, myright, end_y);    
#endif
    wCtrlMain->m_wCtrlView->reset();
    wCtrlMain->m_wCtrlLight->reset();   
    wCtrlMain->show(argc,argv);    
     
/* 
#ifdef WIN_SLICE    

    WinSliceMain * wSliceMain = NULL;
    wSliceMain = CreateSliceViewWindow(wCtrlNetCDF, SLICE_X_AXIS);
    wSliceMain->show(argc,argv);
    wSliceMain->SetVolume(pVolume, SLICE_X_AXIS);
  
    wSliceMain = CreateSliceViewWindow(wCtrlNetCDF, SLICE_Y_AXIS);
    wSliceMain->show(argc,argv);
    wSliceMain->SetVolume(pVolume, SLICE_Y_AXIS);
    
    wSliceMain = CreateSliceViewWindow(wCtrlNetCDF, SLICE_Z_AXIS);
    wSliceMain->show(argc,argv);
    wSliceMain->SetVolume(pVolume, SLICE_Z_AXIS);

#endif
*/
    

#ifdef WIN_HISTORY
    WinCtrlHistory * wCtrlHistory = CreateCtrlHistoryWindow(wVisMain, pVolume, wCtrlMain);
    wCtrlHistory->show(argc,argv);
#endif 

    Fl::visual(FL_DOUBLE|FL_RGB);
    wCtrlNetCDF->show(argc,argv);
    //wVisMain->Show(argc,argv);    

    Fl::run();
    delete pVolume;
    //delete wVisMain;
    delete wCtrlMain;
    return 0;
}

int multi_variable(int argc, char **argv)
{
    WinVisMain * wVisMain = CreateMainWindow();

    for (int i = 0; i < 2; i++) {
        //create a volume object   
        cxVolume * pVolume = new cxVolume(i); 
        
        int end_y = 0;
        wVisMain->AddVolume(pVolume);

        WinCtrlMain * wCtrlMain = CreateCtrlMainWindow(wVisMain, pVolume, 0 + i * 330, 300, myright, end_y);    
        wCtrlMain->m_wCtrlView->reset();
        wCtrlMain->m_wCtrlLight->reset();   
        wCtrlMain->show(argc,argv);    
    }
    
    Fl::visual(FL_DOUBLE|FL_RGB);
    wVisMain->Show(argc,argv);    

    Fl::run();
    return 0;
}

int main(int argc, char **argv) 
{
#ifdef MULTI_VARIABLES
    multi_variable(argc, argv);    
#else
    single_variable(argc, argv);    
#endif
    
    return 0;
}

//new main window
WinVisMain *CreateMainWindow(void)
{
	int totalw = Fl::w();
	int totalh = Fl::h();

    int mainw = MAINWIN_SIZE;
    int mainh = MAINWIN_SIZE;
    
#if defined(CLIMATE)
    mainh = MAINWIN_SIZE / 2;
#endif

	int mainx = (totalw - mainw - MAINWIN_SIZE)/2;
	int mainy = (totalh-mainh)/2 - 50;
	mainy = (mainy > 40)? mainy : 40;

	WinVisMain * pVisMain = new WinVisMain(mainx, mainy,mainw, mainh);
	return pVisMain;
}


WinCtrlNetCDF * CreateCtrlNetCDFWindow(cxVolume * pVolume, int begin_x, int hight, side sd, int &end_y)
{
	int totalw = Fl::w();
	int totalh = Fl::h();

    int mainw = MAINWIN_SIZE;
    int mainh = MAINWIN_SIZE;
    
#if defined(CLIMATE)
    mainh = MAINWIN_SIZE;// *3 /4 ;
    //mainh = MAINWIN_SIZE / 2;
#endif

	int mainx = (totalw - mainw - MAINWIN_SIZE)/2;
	int mainy = (totalh-mainh)/2 - 50;
	mainy = (mainy > 40)? mainy : 40;


    WinCtrlNetCDF * pCtrlMain = new WinCtrlNetCDF(mainx, mainy,mainw, mainh, pVolume);    
    //pCtrlShow->SetVisMain(wMain);
    
    
    return pCtrlMain;   
}

WinCtrlMain * CreateCtrlMainWindow(WinVisMain * wMain, cxVolume * pVolume, int begin_y, int hight, side sd, int &end_y)
{
    int mainw = wMain->m_wView->w();
    int mainx = wMain->m_wView->x();
    int mainy = wMain->m_wView->y();

    //int flw = 204;
    int flw = WIN_SIZE;
    int flh = hight;
    int flx;
    if(sd == myleft)
        flx = mainx - flw- BORDER;
    else
        flx = mainx + mainw + BORDER;
    int fly = mainy + begin_y;

    WinCtrlMain * pCtrlMain = new WinCtrlMain(flx,fly,flw,flh, pVolume);    
    //pCtrlShow->SetVisMain(wMain);
    
    end_y = fly + flh - mainy + TITLE;
    
    return pCtrlMain;   
}

WinCtrlMain * CreateCtrlMainNetCDFWindow(WinCtrlNetCDF * wMain, cxVolume * pVolume, int begin_y, int hight, side sd, int &end_y)
{
    int mainw = wMain->m_wMainWin->w();
    int mainx = wMain->m_wMainWin->x();
    int mainy = wMain->m_wMainWin->y();

    //int flw = 204;
    int flw = WIN_SIZE;
    int flh = hight;
    int flx;
    if(sd == myleft)
        flx = mainx - flw- BORDER;
    else
        flx = mainx + mainw + BORDER;
    int fly = mainy + begin_y;

    WinCtrlMain * pCtrlMain = new WinCtrlMain(flx,fly,flw,flh, pVolume);    
    //pCtrlShow->SetVisMain(wMain);
    
    end_y = fly + flh - mainy + TITLE;
    
    return pCtrlMain;   
}


WinSliceMain * CreateSliceViewWindow(WinCtrlNetCDF *wMain, int axis)
{
    int mainw = wMain->m_wMainWin->w();
    int mainh = wMain->m_wMainWin->h();
    int mainx = wMain->m_wMainWin->x();
    int mainy = wMain->m_wMainWin->y();

    int flw = mainw / 3 - BORDER;
    int flh = flw + 30;
    int flx = mainx + (flw + BORDER) * (axis - SLICE_X_AXIS);
    int fly = mainy + mainh + BORDER * 3;
    
    string buf;
    
    switch(axis){
        case SLICE_X_AXIS : buf = "slice - x axis"; //sprintf(buf, "%s", "slice - x axis"); 
                            break;
        case SLICE_Y_AXIS : buf = "slice - y axis"; //sprintf(buf, "%s", "slice - y axis"); 
                            break;
        case SLICE_Z_AXIS : buf = "slice - z axis"; //sprintf(buf, "%s", "slice - z axis"); 
                            break;
    }
    
    WinSliceMain * pSliceMain = new WinSliceMain(flx,fly,flw,flh, (char *)buf.c_str()); 

    return pSliceMain;
}

WinCtrlHistory * CreateCtrlHistoryWindow(WinVisMain *wMain, cxVolume * pVolume, WinCtrlMain * pCtrlMain)
{

    int mainw = wMain->m_wView->w();
    int mainx = wMain->m_wView->x();
    int mainy = wMain->m_wView->y();

    int flw = mainw;
    int flh = 100;
    int flx = mainx;
    int fly = mainy - flh - 25;
    
    WinCtrlHistory * pCtrlHistory = new WinCtrlHistory(flx,fly,flw,flh); 
    pCtrlHistory->SetVolume(pVolume);
    pCtrlHistory->SetCtrlMain(pCtrlMain);
    
    return pCtrlHistory;
}

