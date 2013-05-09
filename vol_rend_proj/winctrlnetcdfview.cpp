/***************************************************************************
                          winctrlnetcdfview.cpp  -  description
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


#include "winctrlnetcdfview.h"
#include <FL/Fl_File_Input.H>
#include <FL/Fl_File_Chooser.H>
#include "winvisview.h"
#include "cxvolume.h"
#include <iostream>
#include <fstream>
#include "netcdf_read.h"

using namespace std;

#define TEXTSIZE 12



WinCtrlNetCDFView::WinCtrlNetCDFView ( int x = 0, int y =0, int w = 200, int h = 200, cxVolume *pVolume = NULL )
{
	GenWindow ( x,y, w,h );
	m_pcoord = NULL;
	m_pVolume =  pVolume;
	netcdf = NULL;
	m_tfn = NULL;
	reset();

}

WinCtrlNetCDFView::~WinCtrlNetCDFView()
{
}

// x = left, y = top, w = right, h = bottom
void WinCtrlNetCDFView::GenWindow ( int x, int y, int w, int h )
{
	int yy, c_x, c_y, c_w, c_h;
	int h_near = 5;
	int w_near = 3;
	m_wMainWin = new Fl_Window ( x, y,w,h, "Control Center" );
	m_wMainWin->user_data ( ( void* ) ( this ) );


	int step = h/20;
	m_pGroupA = new Fl_Group ( 2,30, w-2, h/2, "NetCDF data file:" );
	m_pGroupA->box ( FL_ENGRAVED_BOX );
	m_pGroupA->align ( FL_ALIGN_TOP_LEFT );
	m_pGroupA->user_data ( ( void* ) ( this ) );

	yy = m_pGroupA->y();


	m_pInputA = new Fl_Input ( m_pGroupA->x() +w_near,32,
	                           m_pGroupA->w()-120-w_near*2,30,"" );
	m_pInputA->value ( "" );

	c_x = m_pInputA->x();
	c_y = m_pInputA->y();
	c_h = m_pInputA->h();
	c_w = m_pInputA->w();
	m_pFileChooser = NULL;
	m_pButtonChooser = new Fl_Button ( c_x+c_w, c_y, 120, 30 ,"Browse.." );
	m_pButtonChooser->callback ( ( Fl_Callback * ) cb_ButtonChooser );

	m_pBrowser = new Fl_Select_Browser ( c_x, m_pInputA->y() +m_pInputA->h(),
	                                     m_pGroupA->w() /2 - 2*w_near,
	                                     m_pGroupA->h()/3*2 - m_pInputA->h()-h_near*10 );
	int br_widths[] = {200};               // widths for each column
	m_pBrowser->column_widths ( br_widths );
	m_pBrowser->callback ( ( Fl_Callback * ) b_cb, this );

	text1= new Fl_Box ( FL_FRAME_BOX,c_x, m_pBrowser->y() + m_pBrowser->h(),
	                    m_pGroupA->w() /2 - 2*w_near,h_near*6,m_pBrowser->text ( m_pBrowser->value() ) );
	text1->align ( FL_ALIGN_CENTER );

	m_min1 = new Fl_Input(c_x, text1->y() + text1->h() + h_near*3,
	                    (m_pGroupA->w()/4) - w_near,h_near*6, "min");
	m_min1->align(FL_ALIGN_TOP);
	m_min1->value("0");

	m_max1 = new Fl_Input(c_x + m_min1->w(), text1->y() + text1->h() + h_near*3,
	                    (m_pGroupA->w()/4) - w_near,h_near*6, "max");
	m_max1->align(FL_ALIGN_TOP);
	m_max1->value("0");

/*
	text1= new Fl_Box ( FL_FRAME_BOX,c_x, m_pBrowser->y() + m_pBrowser->h(),
	                    m_pGroupA->w() /2 - 2*w_near,h_near*6,m_pBrowser->text ( m_pBrowser->value() ) );
	text1->align ( FL_ALIGN_CENTER );
*/
	//m_pBrowser->column_char('\t');                                                       // tabs as column delimiters
	//m_pBrowser->type(FL_MULTI_BROWSER);

	m_pBrowser2 = new Fl_Select_Browser ( m_pBrowser->x() + m_pBrowser->w() + w_near,
	                                      m_pInputA->y() +m_pInputA->h(),
	                                      m_pBrowser->w(),
	                                      m_pGroupA->h()/3*2 - m_pInputA->h()-h_near*10 );
	m_pBrowser2->column_widths ( br_widths );
	m_pBrowser2->callback ( ( Fl_Callback * ) b_cb2, this );
	m_pBrowser2->hide();
	text2= new Fl_Box ( FL_FRAME_BOX,m_pBrowser->x() + m_pBrowser->w() + w_near,
	                    m_pBrowser2->y() + m_pBrowser2->h(),
	                    m_pGroupA->w() /2 - 2*w_near,
	                    h_near*6,
	                    m_pBrowser2->text ( m_pBrowser2->value() ) );
	text2->align ( FL_ALIGN_CENTER );
	text2->hide();

	m_min2 = new Fl_Input(m_pBrowser->x() + m_pBrowser->w() + w_near, text1->y() + text1->h() + h_near*3,
			(m_pGroupA->w()/4) - w_near,h_near*6, "min");
	m_min2->align(FL_ALIGN_TOP);
	m_min2->value("0");
	m_min2->hide();

	m_max2 = new Fl_Input(m_min2->x() + m_min2->w(), text1->y() + text1->h() + h_near*3,
	                    (m_pGroupA->w()/4) - w_near,h_near*6, "max");
	m_max2->align(FL_ALIGN_TOP);
	m_max2->value("0");
	m_max2->hide();
	m_pGroupA->end();


	y = m_pGroupA->y() + m_pGroupA->h() + step;
	m_pGroupD = new Fl_Group (	2, m_pGroupA->y() + m_pGroupA->h() + h_near*3,
	                           w-2, m_wMainWin->h()-m_pGroupA->h()-h_near*3-30*2 /*h/2-60*/, "Settings" );
	m_pGroupD->box ( FL_ENGRAVED_BOX );
	m_pGroupD->align ( FL_ALIGN_TOP_LEFT );
	m_pGroupD->user_data ( ( void* ) ( this ) );

	yy = m_pGroupD->y();

	m_pCheckA = new Fl_Check_Button ( 7, yy+0*step, 25, 20, "Activate Correlation Browser" );
	m_pCheckA->labelsize ( TEXTSIZE );
	m_pCheckA ->down_box ( FL_DOWN_BOX );
	m_pCheckA->callback ( ( Fl_Callback * ) cb_CheckA, this );

	m_pCheckAB = new Fl_Check_Button ( 7 + m_pCheckA->w() + 300, yy+0*step, 25, 20, "Freeze Reference Point" );
	m_pCheckAB->labelsize ( TEXTSIZE );
	m_pCheckAB ->down_box ( FL_DOWN_BOX );
	m_pCheckAB->callback ( ( Fl_Callback * ) cb_CheckAB, this );

#ifdef POINT_CLOUD
	m_inp_xblock = new Fl_Input ( 7, yy+0*step+25, 40, 20, "x block size" );
	m_inp_xblock->align ( FL_ALIGN_RIGHT );
	m_inp_xblock->value ( "6" );

	m_inp_yblock = new Fl_Input ( 7, yy+0*step+25*2, 40, 20, "y block size" );
	m_inp_yblock->align ( FL_ALIGN_RIGHT );
	m_inp_yblock->value ( "3" );
	m_inp_zblock = new Fl_Input ( 7, yy+0*step+25*3, 40, 20, "z block size" );
	m_inp_zblock->align ( FL_ALIGN_RIGHT );
	m_inp_zblock->value ( "3" );
	m_inp_thold = new Fl_Input ( 7, yy+0*step+25*4, 100, 20, "point cloud variance threshold" );
	m_inp_thold->align ( FL_ALIGN_RIGHT );
	m_inp_thold->value ( "0.1" );
	m_update = new Fl_Button ( 7, yy+0*step+25*5, 100, 20, "update" );
	m_update->callback ( ( Fl_Callback * ) cb_update, this );
	m_pGroupD->end();
#endif

	yy = m_pGroupD->y() + m_pGroupD->h();

	//m_pButtonA = new Fl_Button(3, yy + 1*step, 100, 20, "Reset View");
	//m_pButtonA -> labelsize(TEXTSIZE);
	//m_pButtonA->callback((Fl_Callback *)cb_ButtonA);


	m_wMainWin->end();
	m_wMainWin->resizable ( m_wMainWin );
	m_wMainWin->size_range ( 200,200 );
}

void WinCtrlNetCDFView::cb_update ( Fl_Button* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_update_i ( o,v );
}

inline void WinCtrlNetCDFView::cb_update_i ( Fl_Button* o, void* v )
{
	WinCtrlNetCDFView* tf = reinterpret_cast<WinCtrlNetCDFView*> ( v );
	if ( tf->m_pVolume )
	{
		tf->m_pVolume->UpdatePointCloudSettings ( atoi ( m_inp_xblock->value() ),
		        atoi ( m_inp_yblock->value() ),
		        atoi ( m_inp_zblock->value() ),
		        atof ( m_inp_thold->value() ) );
		tf->m_pVolume->ReDraw();
	}
}

/*
void WinCtrlNetCDFView::setrowheader_cb(Fl_Widget* w)
{

    Fl_Check_Button* flcb = ((Fl_Check_Button*)w);
        if ( flcb->value() == 1 ) {
                flcb->labelcolor(FL_RED);
                flcb->label("Label&Scale");
                new Fl_Box(0, 20, 1000, 20, " first ");
                new Fl_Box(0, 640, 1000, 20, "  second ");
                new Fl_Box(0, 660, 1000, 20, " third ");
        }
        else {
                flcb->labelcolor(FL_BLACK);
                flcb->label("No Label&Scale");
        }
        flcb->damage(1);
        flcb->parent()->redraw();
}
*/

void WinCtrlNetCDFView::b_cb ( Fl_Widget* o, void* v )
{
	printf ( "callback, selection = %d, event_clicks = %d\n",
	         ( ( Fl_Select_Browser* ) o )->value(), Fl::event_clicks() );
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->b_cb_i ( o,v );
}

inline void WinCtrlNetCDFView::b_cb_i ( Fl_Widget* o, void* v )
{
	if ( m_pCheckA->value() != 1 )
	{
		if ( ( ( Fl_Browser* ) o )->value() > 0 )
		{
			float fmin1 = atof(m_min1->value());
			float fmax1 = atof(m_max1->value());
			float fmin2 = atof(m_min2->value());
			float fmax2 = atof(m_max2->value());
			// 
			// if max and min are equal then use default values for min and max
			if(fmin1 <= fmax1){
				m_pVolume->SetMinMax(fmin1, fmax1, fmin2, fmax2);
				text1->label ( ( ( Fl_Browser* ) o )->text ( ( ( Fl_Browser* ) o )->value() ) );
				m_pVolume->Read ( ( ( Fl_Browser* ) o )->text ( ( ( Fl_Browser* ) o )->value() ) );
			}
		}
	}
	else
	{
		if ( ( ( Fl_Browser* ) o )->value() > 0 )
		{
			text1->label ( ( ( Fl_Browser* ) o )->text ( ( ( Fl_Browser* ) o )->value() ) );
		}
	}
	m_pVolume->ReDraw();
}

void WinCtrlNetCDFView::b_cb2 ( Fl_Widget* o, void* v )
{
	printf ( "callback, selection = %d, event_clicks = %d\n",
	         ( ( Fl_Select_Browser* ) o )->value(), Fl::event_clicks() );
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->b_cb2_i ( o,v );
}

inline void WinCtrlNetCDFView::b_cb2_i ( Fl_Widget* o, void* v )
{
	if ( m_pCheckA->value() == 1 )
	{
		if ( m_pBrowser->value() > 0 )
			if ( ( ( Fl_Browser* ) o )->value() > 0 )
			{
				float fmin1 = atof(m_min1->value());
				float fmax1 = atof(m_max1->value());
				float fmin2 = atof(m_min2->value());
				float fmax2 = atof(m_max2->value());
				if(fmin1 < fmax1 && fmin2 < fmax2){
					m_pVolume->SetMinMax(fmin1, fmax1, fmin2, fmax2);
					text2->label ( ( ( Fl_Browser* ) o )->text ( ( ( Fl_Browser* ) o )->value() ) );
					m_pVolume->Read ( ( ( Fl_Browser* ) o )->text ( ( ( Fl_Browser* ) o )->value() ),
				                  m_pBrowser->text ( m_pBrowser->value() ),true );
				}
			}
	}
	m_pVolume->ReDraw();
}

void WinCtrlNetCDFView::show ( int argc, char **argv )
{
	m_wMainWin->show ( argc,argv );
}

inline void WinCtrlNetCDFView::cb_CheckA_i ( Fl_Check_Button* o, void* )
{
	if ( o->value() == 1 )
	{
		m_pBrowser2->show();
		text2->show();
		m_min2->show();
		m_max2->show();
		m_tfn->setCorrDefaults();
	}
	else
	{
		m_pBrowser2->hide();
		text2->hide();
		m_min2->hide();
		m_max2->hide();
		m_tfn->setDefaults();
	}
}

void WinCtrlNetCDFView::cb_CheckA ( Fl_Check_Button* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_CheckA_i ( o,v );
}

inline void WinCtrlNetCDFView::cb_CheckAB_i ( Fl_Check_Button* o, void* )
{
	if ( m_pVolume == NULL )
		return;
	bool val = ( o->value() == 1 ) ? true:false;
	m_pVolume->SetCorrFreeze ( val );
	m_pVolume->ReDraw();
	SaveConfig();
}

void WinCtrlNetCDFView::cb_CheckAB ( Fl_Check_Button* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_CheckAB_i ( o,v );
}

inline void WinCtrlNetCDFView::cb_CheckB_i ( Fl_Check_Button* o, void* )
{
	if ( m_pVolume == NULL )
		return;
	m_pVolume->m_pVisStatus->m_bDrawFrame = ( o->value() == 1 ) ? true:false;
	m_pVolume->ReDraw();
	SaveConfig();
}

void WinCtrlNetCDFView::cb_CheckB ( Fl_Check_Button* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_CheckB_i ( o,v );
}

inline void WinCtrlNetCDFView::cb_CheckC_i ( Fl_Check_Button* o, void* )
{
	if ( m_pVolume == NULL )
		return;
	m_pVolume->m_pVisStatus->m_bDrawVolume = ( o->value() == 1 ) ? true:false;
	m_pVolume->ReDraw();
	SaveConfig();
}

void WinCtrlNetCDFView::cb_CheckC ( Fl_Check_Button* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_CheckC_i ( o,v );
}

inline void WinCtrlNetCDFView::cb_CheckD_i ( Fl_Check_Button* o, void* )
{
	if ( m_pVolume == NULL )
		return;
	//m_pVisMain->m_wView->m_pObject->m_bDrawTorus = (o->value() == 1)? true:false;
	m_pVolume->m_bDrawColorMap = ( o->value() == 1 ) ? true:false;
	m_pVolume->ReDraw();
	SaveConfig();
}

void WinCtrlNetCDFView::cb_CheckD ( Fl_Check_Button* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_CheckD_i ( o,v );
}


inline void WinCtrlNetCDFView::cb_CheckE_i ( Fl_Check_Button* o, void* )
{
	if ( m_pVolume == NULL )
		return;

	m_pVolume->m_pVisView->m_bBlack = ( o->value() == 1 ) ? true:false;
	m_pVolume->ReDraw();
	SaveConfig();
}


void WinCtrlNetCDFView::cb_CheckE ( Fl_Check_Button* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_CheckE_i ( o,v );
}


inline void WinCtrlNetCDFView::cb_CheckF_i ( Fl_Check_Button* o, void* )
{
	if ( m_pVolume == NULL )
		return;

	SaveConfig();
}


void WinCtrlNetCDFView::cb_CheckF ( Fl_Check_Button* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_CheckF_i ( o,v );
}


inline void WinCtrlNetCDFView::cb_CheckG_i ( Fl_Check_Button* o, void* )
{
	if ( m_pVolume == NULL )
		return;
	SaveConfig();
}


void WinCtrlNetCDFView::cb_CheckG ( Fl_Check_Button* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_CheckG_i ( o,v );
}

inline void WinCtrlNetCDFView::cb_CheckH_i ( Fl_Check_Button* o, void* )
{
	if ( m_pVolume == NULL )
		return;

	SaveConfig();
}

void WinCtrlNetCDFView::cb_CheckH ( Fl_Check_Button* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_CheckH_i ( o,v );
}


inline void WinCtrlNetCDFView::cb_SliderA_i ( Fl_Value_Slider* o, void* v )
{
	if ( m_pVolume == NULL )
		return;

	m_pVolume->m_pVisStatus->m_fLightPar[0] = o->value();

	m_pVolume->ReDraw();
	SaveConfig();
}


void WinCtrlNetCDFView::cb_SliderA ( Fl_Value_Slider* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_SliderA_i ( o,v );
}


inline void WinCtrlNetCDFView::cb_SliderB_i ( Fl_Value_Slider* o, void* v )
{
	if ( m_pVolume == NULL )
		return;

	m_pVolume->m_pVisStatus->m_fLightPar[1] = o->value();

	m_pVolume->ReDraw();
	SaveConfig();
}


void WinCtrlNetCDFView::cb_SliderB ( Fl_Value_Slider* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_SliderB_i ( o,v );
}

inline void WinCtrlNetCDFView::cb_SliderC_i ( Fl_Value_Slider* o, void* v )
{
	if ( m_pVolume == NULL )
		return;

	m_pVolume->m_pVisStatus->m_fLightPar[2] = o->value();

	m_pVolume->ReDraw();
	SaveConfig();
}


void WinCtrlNetCDFView::cb_SliderC ( Fl_Value_Slider* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_SliderC_i ( o,v );
}

inline void WinCtrlNetCDFView::cb_SliderD_i ( Fl_Value_Slider* o, void* v )
{
	if ( m_pVolume == NULL )
		return;

	m_pVolume->m_pVisStatus->m_fLightPar[3] = o->value();

	m_pVolume->ReDraw();
	SaveConfig();
}


void WinCtrlNetCDFView::cb_SliderD ( Fl_Value_Slider* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_SliderD_i ( o,v );
}



inline void WinCtrlNetCDFView::cb_SliderE_i ( Fl_Value_Slider* o, void* v )
{
	if ( m_pVolume == NULL )
		return;

	m_pVolume->m_pVisStatus->m_fSampleSpacing = o->value();

	m_pVolume->ReDraw();
	SaveConfig();
}


void WinCtrlNetCDFView::cb_SliderE ( Fl_Value_Slider* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_SliderE_i ( o,v );
}


inline void WinCtrlNetCDFView::cb_SliderF_i ( Fl_Value_Slider* o, void* v )
{
	if ( m_pVolume == NULL )
		return;

	float min = o->value();
	float max = m_pSliderG->value();

	if ( min > max )
		min = max;

	o->value ( min );

	if ( m_pRoundA->value() ==1 )
		m_pVolume->m_pVisStatus->m_vRangeMin[0] = min;
	else if ( m_pRoundB->value() ==1 )
		m_pVolume->m_pVisStatus->m_vRangeMin[1] = min;
	else if ( m_pRoundC->value() ==1 )
		m_pVolume->m_pVisStatus->m_vRangeMin[2] = min;


	m_pVolume->ReDraw();
	SaveConfig();
}


void WinCtrlNetCDFView::cb_SliderF ( Fl_Value_Slider* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_SliderF_i ( o,v );
}


inline void WinCtrlNetCDFView::cb_SliderG_i ( Fl_Value_Slider* o, void* v )
{
	if ( m_pVolume == NULL )
		return;

	float max = o->value();
	float min = m_pSliderF->value();

	if ( min > max )
		max = min;

	o->value ( max );

	if ( m_pRoundA->value() ==1 )
		m_pVolume->m_pVisStatus->m_vRangeMax[0] = max;
	else if ( m_pRoundB->value() ==1 )
		m_pVolume->m_pVisStatus->m_vRangeMax[1] = max;
	else if ( m_pRoundC->value() ==1 )
		m_pVolume->m_pVisStatus->m_vRangeMax[2] = max;

	m_pVolume->ReDraw();
	SaveConfig();
}


void WinCtrlNetCDFView::cb_SliderG ( Fl_Value_Slider* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_SliderG_i ( o,v );
}


inline void WinCtrlNetCDFView::cb_RoundA_i ( Fl_Round_Button* o, void* )
{
	if ( m_pVolume == NULL )
		return;

	float min = m_pVolume->m_pVisStatus->m_vRangeMin[0];
	float max = m_pVolume->m_pVisStatus->m_vRangeMax[0];

	m_pSliderF->value ( min );
	m_pSliderG->value ( max );
}

void WinCtrlNetCDFView::cb_RoundA ( Fl_Round_Button* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_RoundA_i ( o,v );
}

inline void WinCtrlNetCDFView::cb_RoundB_i ( Fl_Round_Button* o, void* )
{
	if ( m_pVolume == NULL )
		return;

	float min = m_pVolume->m_pVisStatus->m_vRangeMin[1];
	float max = m_pVolume->m_pVisStatus->m_vRangeMax[1];

	m_pSliderF->value ( min );
	m_pSliderG->value ( max );
}

void WinCtrlNetCDFView::cb_RoundB ( Fl_Round_Button* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_RoundB_i ( o,v );
}


inline void WinCtrlNetCDFView::cb_RoundC_i ( Fl_Round_Button* o, void* )
{
	if ( m_pVolume == NULL )
		return;

	float min = m_pVolume->m_pVisStatus->m_vRangeMin[2];
	float max = m_pVolume->m_pVisStatus->m_vRangeMax[2];

	m_pSliderF->value ( min );
	m_pSliderG->value ( max );
}

void WinCtrlNetCDFView::cb_RoundC ( Fl_Round_Button* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_RoundC_i ( o,v );
}


inline void WinCtrlNetCDFView::cb_ButtonA_i ( Fl_Button* o, void* )
{
	if ( m_pVolume == NULL )
		return;
	m_pVolume->m_pVisView->Reset();
	m_pVolume->ReDraw();
	SaveConfig();
}

inline void WinCtrlNetCDFView::cb_ButtonChooser_i ( Fl_Button* o, void* )
{

	if ( !m_pFileChooser )
		m_pFileChooser = new Fl_File_Chooser ( "", "", Fl_File_Chooser::SINGLE, "" );
	m_pFileChooser->show();

	// Block until user picks something.
	//     (The other way to do this is to use a callback())
	//
	while ( m_pFileChooser->shown() )
	{
		Fl::wait();
	}

	// Print the results
	if ( m_pFileChooser->value() == NULL )
	{
		printf ( "(User hit 'Cancel')\n" );
		return;
	}

	printf ( "DIRECTORY: '%s'\n", m_pFileChooser->directory() );
	printf ( "    VALUE: '%s'\n", m_pFileChooser->value() );
	m_pInputA->value ( m_pFileChooser->value() );

	m_pBrowser->clear();
	m_pBrowser2->clear();

	//if(netcdf)
	//	delete netcdf;

	netcdf = m_pVolume->openNetCDF_file ( const_cast<char*> ( m_pFileChooser->value() ) );

	//netcdf = m_pVolume->openNetCDF_file("D:/jeffs/data/temp_salt.nc");

	int num_vars;
	char** var_names = netcdf->get_varnames ( &num_vars );
	for ( int i = 0; i < num_vars; i++ )
	{
		m_pBrowser->add ( var_names[i] ); // lines of tab delimited data
		m_pBrowser2->add ( var_names[i] ); // lines of tab delimited data
	}
	m_pcoord->SetNetCDF ( netcdf );
	//m_pcoord->SetInitialTimeStep(m_pVolume->GetStartTimeStep());

	SaveConfig();
}

void WinCtrlNetCDFView::cb_ButtonA ( Fl_Button* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_ButtonA_i ( o,v );
}

void WinCtrlNetCDFView::cb_ButtonChooser ( Fl_Button* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_ButtonChooser_i ( o,v );
}

inline void WinCtrlNetCDFView::cb_ButtonB_i ( Fl_Button* o, void* )
{
	if ( m_pVolume == NULL )
		return;
	SaveConfig();
}

void WinCtrlNetCDFView::cb_ButtonB ( Fl_Button* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_ButtonB_i ( o,v );
}

inline void WinCtrlNetCDFView::cb_ButtonC_i ( Fl_Button* o, void* )
{
	if ( m_pVolume == NULL )
		return;



	m_pVolume->ReDraw();
	SaveConfig();
}

void WinCtrlNetCDFView::cb_ButtonC ( Fl_Button* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_ButtonC_i ( o,v );
}

inline void WinCtrlNetCDFView::cb_ButtonD_i ( Fl_Button* o, void* )
{
	if ( m_pVolume == NULL )
		return;

	//delete
	int size = m_pBrowserA->size();
	if ( size <= 0 )
		return;


	m_pVolume->ReDraw();
	SaveConfig();
}

void WinCtrlNetCDFView::cb_ButtonD ( Fl_Button* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_ButtonD_i ( o,v );
}

inline void WinCtrlNetCDFView::cb_BrowserA_i ( Fl_Hold_Browser* o, void* )
{
	if ( m_pVolume == NULL )
		return;
	SaveConfig();
}

void WinCtrlNetCDFView::cb_BrowserA ( Fl_Hold_Browser* o, void* v )
{
	( ( WinCtrlNetCDFView* ) ( o->parent()->user_data() ) )->cb_BrowserA_i ( o,v );
}



void WinCtrlNetCDFView::reset()
{

	char filename[1024];

#ifdef MULTI_VARIABLES
	sprintf ( filename, "ctrldisplay%d.cfg", m_pVolume->m_nVolumeID );
#else
	sprintf ( filename, "ctrldisplay.cfg" );
#endif

	ifstream inf ( filename );

	if ( inf )
	{
		float value;


	}
	else
	{

	}
}


void WinCtrlNetCDFView::SaveConfig()
{
	char filename[1024];

#ifdef MULTI_VARIABLES
	sprintf ( filename, "ctrldisplay%d.cfg", m_pVolume->m_nVolumeID );
#else
	sprintf ( filename, "ctrldisplay.cfg" );
#endif

	ofstream outf ( filename );

	float value;

	outf.close();
}
