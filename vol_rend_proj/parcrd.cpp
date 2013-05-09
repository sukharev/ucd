


//#include <FL/glut.H>
#include <iostream>
#include <fstream>
#include <assert.h>

#include "GL/glew.h"
#include <GL/glu.h>
#include <FL/gl.h>
#include <FL/glut.H>
#include <time.h>
#include "parcrd.h"
#define TEXTSIZE 12

using namespace std;

PCoord::PCoord(int X,int Y,int W,int H, cxVolume * pVolume, int sample) : Fl_Gl_Window(X,Y,W,H-COLOR_CHOOSER_HEIGHT)
//PCoord::PCoord(int X,int Y,int W,int H, cxVolume * pVolume, int sample) : Fl_Widget(X,Y,W,H)
{
	m_xor = false;
	m_translucent = false;

	m_init = false;
	//m_data = NULL;

	m_windowWidth = w();
	m_windowHeight = h()-COLOR_CHOOSER_HEIGHT;
	
	m_bg_color.R = m_bg_color.G = m_bg_color.B = 0.0;

	m_netcdf = NULL;
	m_start_time = 0;
	m_numvars = 0;
	m_datautils = NULL;
	m_data = NULL;
	m_sample = ((sample == 0) ? 1 : sample);
	m_pVolume = pVolume;
	m_regiondata = NULL;
	
	m_context_x_start = 0;
	m_context_x_end = 0;
	m_context_y_start = 0;
	m_context_y_end = 0;
	m_context_z_start = 0;
	m_context_z_end = 0;
	m_bDisplay = false;
	srand((unsigned)time(NULL));
}

PCoord::~PCoord()
{
	delete m_regiondata;
	delete m_datautils;
	delete m_fltr;
}

bool PCoord::SetInitialTimeStep(int new_start_time_step)
{
	bool bRet=false;
	m_start_time = new_start_time_step;
	bRet = InitData();
	if(bRet)
		redraw();
	return bRet; 
}

bool PCoord::InitData()
{
	//m_data = data;
	if(m_netcdf) {
		if(m_init && m_datautils && m_fltr){
			m_data = m_datautils->ReadVarData(m_start_time,1,true);
		}
		else{
			m_fltr = new Filter();
			m_fltr->m_dim = 4;
			m_datautils = new NetCDFUtils(m_netcdf, m_fltr);
			m_data = m_datautils->ReadVarData(m_start_time,1,true);
			m_numvars = m_datautils->get_num_vars();
			m_init = true;
			m_context_x_start = 0;
			m_context_x_end = m_pVolume->m_vSize[0];
			m_context_y_start = 0;
			m_context_y_end = m_pVolume->m_vSize[1];
			m_context_z_start = 0;
			m_context_z_end = m_pVolume->m_vSize[2];
		}
	}
	return m_init; 
}

void PCoord::draw()
{
   	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0,1.0,1.0,1.0);
    	glMatrixMode(GL_PROJECTION);	
	glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);	
	glLoadIdentity();
	
/*	
 	//glScalef(2, 2, 1);
     	//glTranslatef(-0.5, -0.5, 0);
     
 	glColor3f(1, 0, 0);
 	
 	glBegin(GL_QUADS);
 		glVertex2f(0, 0);
 		glVertex2f(1, 0);
 		glVertex2f(1, 1);
 		glVertex2f(0, 1);
 	glEnd();
*/	
	
 	//gl_start();
  	//fl_clip(x(),y(),w(),h());
	fl_color(FL_DARK3);
	//SetDisplaySize(w(), h());
  	SetDisplaySize(2.0, 2.0);
 	    	
 	glViewport(0, 0, w(), h());
	Display();
	//fl_pop_matrix();
	//fl_pop_clip();
	//gl_finish(); 

}
/*
bool PCoord::SaveSubset(int x_start, int y_start, int z_start,
			     int x_end, int y_end, int z_end,
			     int time_in, int time_count)
{
	NetCDFUtils* datautils = NULL;
	Filter* fltr = NULL;
	int numvars = 0;
	if(m_netcdf) {
		fltr = new Filter();
		fltr->m_dim = 4;
		datautils = new NetCDFUtils(m_netcdf, fltr);
		numvars = datautils->get_num_vars();
		datautils->SaveSubset(x_start, y_start, z_start,
			   x_end, y_end, z_end,
			   time_in, time_count);
		if(datautils)
			delete datautils;
		if(fltr)
			delete fltr;	
	}
	return true;
}
*/

int PCoord::GetContextCoord(bool start, char axis)
{
	int retval = 0;
	if(m_netcdf && m_datautils){
		switch (axis){
			case 'X':
				if(start)
					retval = (int)(m_context_x_start*100.0/m_pVolume->m_vSize[0]);
				else
					retval = (int)(m_context_x_end*100.0/m_pVolume->m_vSize[0]);
				break;
			case 'Y':
				if(start)
					retval = (int)(m_context_y_start*100.0/m_pVolume->m_vSize[1]);
				else
					retval = (int)(m_context_y_end*100.0/m_pVolume->m_vSize[1]);
				break;
			case 'Z':
				if(start)
					retval = (int)(m_context_z_start*100.0/m_pVolume->m_vSize[2]);
				else
					retval = (int)(m_context_z_end*100.0/m_pVolume->m_vSize[2]);
				break;
			default:
				break;
		}
	}
	return retval;
}

bool PCoord::ChangeDataSize(int new_start, int new_count, char axis, int* out_val)
{
	bool bRet = true;
	bool bStart = true;
	float m_old_size;
	if (new_start > -1)
		bStart = true;
	else if(new_count > -1)
		bStart = false;
			
	//int out_count = new_count;
	if(m_netcdf && m_datautils){
		switch (axis){
			case 'X':
				m_old_size = m_context_x_end - m_context_x_start;
				
				if (new_start > -1){
					m_context_x_end =  (m_context_x_end + ((float)new_start/100.0)*m_pVolume->m_vSize[0] - m_context_x_start);
					m_context_x_start = ((float)new_start/100.0)*m_pVolume->m_vSize[0];
				}
				else if(new_count > -1)
					m_context_x_end = m_context_x_start + ((float)new_count/100.0)*m_pVolume->m_vSize[0];
				
				if (m_context_x_end > m_pVolume->m_vSize[0] ){
					m_context_x_end = m_pVolume->m_vSize[0]-1;
					bRet = false;
				}
				if (m_context_x_start > m_pVolume->m_vSize[0] ){
					m_context_x_start = m_pVolume->m_vSize[0]-1;
					bRet = false;
				}	
				
				if(m_old_size > (m_context_x_end-m_context_x_start) && new_start > -1)
					m_context_x_start = m_context_x_end - m_old_size;
				
				//printf("m_context_x_start = %d, m_context_x_end = %d, new_start = %d, new_count = %d \n", m_context_x_start, m_context_x_end, new_start, new_count);
				break;
			case 'Y':
				m_old_size = m_context_y_end - m_context_y_start;
				if (new_start > -1){
					m_context_y_end =  (m_context_y_end + ((float)new_start/100.0)*m_pVolume->m_vSize[1] - m_context_y_start);
					m_context_y_start = ((float)new_start/100.0)*m_pVolume->m_vSize[1];
				}
				else if(new_count > -1)
					m_context_y_end = m_context_y_start + ((float)new_count/100.0)*m_pVolume->m_vSize[1];
				printf("m_context_y_start = %d, m_context_y_end = %d, new_start = %d, new_count = %d \n", m_context_y_start, m_context_y_end, new_start, new_count);
				
				if (m_context_y_end > m_pVolume->m_vSize[1] ){
					m_context_y_end = m_pVolume->m_vSize[1]-1;
					bRet = false;
				}
				if (m_context_y_start > m_pVolume->m_vSize[1] ){
					m_context_y_start = m_pVolume->m_vSize[1]-1;
					bRet = false;
				}
				
				if(m_old_size > (m_context_y_end-m_context_y_start) && new_start > -1)
					m_context_y_start = m_context_y_end - m_old_size;
				break;
			case 'Z':
				m_old_size = m_context_z_end - m_context_z_start;
				if (new_start > -1){
					m_context_z_end =  (m_context_z_end + ((float)new_start/100.0)*m_pVolume->m_vSize[2] - m_context_z_start);
					m_context_z_start = ((float)new_start/100.0)*m_pVolume->m_vSize[2];
				}
				else if(new_count > -1)
					m_context_z_end = m_context_z_start + ((float)new_count/100.0)*m_pVolume->m_vSize[2];
				
				if (m_context_z_end > m_pVolume->m_vSize[2] ){
					m_context_z_end = m_pVolume->m_vSize[2]-1;
					bRet = false;
				}
				if (m_context_z_start > m_pVolume->m_vSize[2] ){
					m_context_z_start = m_pVolume->m_vSize[2]-1;
					bRet = false;
				}
				
				if(m_old_size > (m_context_z_end-m_context_z_start) && new_start > -1)
					m_context_z_start = m_context_z_end - m_old_size;
				break;
			default:
				break;	
		}
		//if(!m_datautils->ChangeDataSize(new_start, new_count, axis, &out_count))
		//	cerr << "TODO: change the slider position back!\n";
		//m_data = m_datautils->ReadVarData(0,1,true);
		
	}
	m_pVolume->SetVolSelect(m_context_x_start, m_context_x_end, m_context_y_start, m_context_y_end, m_context_z_start, m_context_z_end);
	if(out_val)
		*out_val = GetContextCoord(bStart, axis);
	return bRet;
	
}



void PCoord::DrawAxes()
{
    Color c(0.5,0.5,0.5);

    SetForeground(c, 1.0);
    int i;
    for (i=0; i<m_numvars; i++) {
        double x = XMap(i);
        DrawLine(x, m_windowHeight/10, x, m_windowHeight*0.8);
    }

    float dimwidth = m_windowWidth/m_numvars;
	

    for (i=0; i<m_numvars; i++)
    {
        float x = XMap(i);
        //DrawConfinedString(x, y()+100 ,m_datautils->names[i],0);
	DrawConfinedString(x, m_windowHeight*0.93 ,m_datautils->names[i],0);
	
        // Draw the MAX values of the dimension
		char buf[50];
	if(m_datautils->dim_min[i] < 0 && m_datautils->dim_min[i] > -0.01)
		sprintf(buf, "%.02E", m_datautils->dim_max[i]);
	else
        	sprintf(buf, "%.02f", m_datautils->dim_max[i]);
        DrawConfinedString(x, m_windowHeight*0.84,buf,0);

        // Draw the MIN values of the dimension
        if(m_datautils->dim_min[i] < 0 && m_datautils->dim_min[i] > -0.01)
        	sprintf(buf, "%.02E", m_datautils->dim_min[i]);
        else
        	sprintf(buf, "%.02f", m_datautils->dim_min[i]);
        DrawConfinedString(x, 0.03 ,buf,0);
    }
    // Flush drawing commands
    //glutSwapBuffers();
}



//////////////////////////////////////////////////////
// utilities

void PCoord::BeginDraw()
{

    if (m_xor == true)
    {
        // Setup XOR drawing mode
        glEnable(GL_COLOR_LOGIC_OP);
        glLogicOp(GL_INVERT);
        glDrawBuffer(GL_FRONT);
    } else {
        glColor4f(m_fg_color.R, m_fg_color.G, m_fg_color.B, m_alpha );
    }

    if (m_translucent == true) // && asize != 0)
    {
        // Turn on Translucency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    }
/*
    if (m_flatcolor == False)  {
        // Turn on color interpolation
        glShadeModel(GL_SMOOTH);
    } else {
        glShadeModel(GL_FLAT);
    }
*/


}

void PCoord::EndDraw()
{
    if (m_xor == true) {
        glDisable(GL_COLOR_LOGIC_OP);
        glDrawBuffer(GL_BACK);
    }
    if (m_translucent == true) { // && asize != 0)
        glDisable(GL_BLEND);
    }
/*
    if (m_flatcolor == False) {
        glShadeModel(GL_FLAT);
    }
*/
}


void PCoord::DrawLine( float x1, float y1, float x2, float y2)
{
    BeginDraw();
	//glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex2f(x1,y1);
    glVertex2f(x2,y2);
	glEnd();
    EndDraw();
}

void PCoord::Display()
{
    glClearColor( 0, 0, 0, 1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if(m_bDisplay){
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);    // Use The Good Calculations
    glEnable(GL_LINE_SMOOTH);            // Enable Anti-Aliasing
        
    if (!m_data) {
	if(!InitData())
		return;
    }

    
    glClear(GL_COLOR_BUFFER_BIT);

    //glScalef(2, 2, 1);
    glTranslatef(-1.0, -1.0, 0);
    glLineWidth(1);
    glShadeModel(GL_SMOOTH);  // Because each line is the same color.

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    //glColor4f(0.0, 0.4, 0.5, 0.006);
    glColor4f(0.0, 1.0, 0.0, 0.03);
    
    int size = 0;
    
    for (int i=0; i<m_numvars-1; i++)
    {
	if(m_datautils->m_size[i] > size)
		size = m_datautils->m_size[i];
    }

    int stepj;
    if(m_sample != 1)
    	stepj = (int)((float)size/(50000.0-(float)m_sample+1.0));
    else
    	stepj = 1;
    for(int x1=m_context_x_start; x1 <= m_context_x_end; x1++){
	for(int y1=m_context_y_start; y1 <= m_context_y_end; y1++){
		for(int z1=m_context_z_start; z1 <= m_context_z_end; z1++){
			int id = x1 + y1 * m_pVolume->m_vSize[0] + z1 * m_pVolume->m_vSize[0] * m_pVolume->m_vSize[1];
			if(id % stepj != 0)
				continue;
			//glLineWidth(1.0);
			//glColor4f(1.0, 0.0, 0.0, 0.03);
			glBegin(GL_LINE_STRIP);
			for (int i=0; i<m_numvars-1; i++)
			{
				float* data = m_data[i];
				if (!data)
					break;
				if (m_datautils->m_size[i] < 1)
					break;
				int index = m_datautils->GetCoordPosition(i, id);
				glVertex2f( XMap(i), m_windowHeight*0.1+(YMap(i,data[index],m_datautils->dim_num[i])) );
				//printf("coord: x->%f y->%f\n",XMap(i), (YMap(i,data[index],m_datautils->dim_num[i])));
			}
			glEnd();
		}
	}
    }
    
    //glDisable(GL_BLEND);
    if(m_pVolume->IsSelected2DRegion())
    {
        //glLineWidth(1);
    	//glShadeModel(GL_SMOOTH);  // Because each line is the same color.

    	//glEnable(GL_BLEND);
    	//glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	//glColor4f(1.0, 0.0, 0.0, 0.09);
	glColor4f(1.0, 0.0, 0.0, 0.1);
	int x_start, x_end, y_start, y_end, z_start, z_end;
	if(m_pVolume->GetSelected2DRegion(&x_start,&x_end,&y_start,&y_end,&z_start,&z_end)){
		int zcontr = 1 + (z_end-z_start);
		int ycontr = 1 + (y_end-y_start);
		int xcontr = 1 + (x_end-x_start);
		if(xcontr > 1)
			x_end--;
		if(ycontr > 1)
			y_end--;
		if(zcontr > 1)
			z_end--;
		int size_selected = zcontr*xcontr*ycontr;
		float alfa = 0.3 - size_selected/size * 0.3;
		printf("ALFA %f\n", alfa);
		for(int x1=x_start; x1 <= x_end; x1++){
			for(int y1=y_start; y1 <= y_end; y1++){
				for(int z1=z_start; z1 <= z_end; z1++){
					int id = x1 + y1 * m_pVolume->m_vSize[0] + z1 * m_pVolume->m_vSize[0] * m_pVolume->m_vSize[1];
					glLineWidth(1.0);
					glColor4f(1.0, 0.0, 0.0, 0.08);
					glBegin(GL_LINE_STRIP);
					for (int i=0; i<m_numvars-1; i++)
					{
						float* data = m_data[i];
						if (!data)
							break;
						if (m_datautils->m_size[i] < 1)
							break;
						int index = m_datautils->GetCoordPosition(i, id);
						glVertex2f( XMap(i), m_windowHeight*0.1+(YMap(i,data[index],m_datautils->dim_num[i])) );
					}
					glEnd();
				}
			}
		}
	}
	//glDisable(GL_BLEND);
    }
    glDisable(GL_BLEND);
    glLineWidth(7);
    
    DrawAxes();
    glShadeModel(GL_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    }
}

void PCoord::DrawConfinedString( float x, float y, const char *text, float width)
{
/*
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();    
*/
    gl_font(FL_HELVETICA_BOLD, 12);
    gl_color(FL_WHITE);
    gl_draw(text, x, y);	
    //gl_draw("GG",0,0);
/*    
    glPopMatrix();
    glPopMatrix();
*/
}

void PCoord::DrawString( float x, float y, const char *text, int N )
{
    gl_font(FL_HELVETICA_BOLD, 9);
    gl_color(FL_WHITE);
    gl_draw(text, x, y);
}
/*
void PCoord::DrawString( const char *text, int N )
{
	assert(text);
    if (N<0) N=strlen(text);
	m_font_base = 1000;
    if (m_font_base == 0) return;
    glListBase(m_font_base);
    glCallLists(N,GL_UNSIGNED_BYTE,text);
}
*/


void PCoord::SetForeground( const Color &color, float a )
{
    // Set normal foreground
    m_fg_color = color;
    m_alpha = a;
    if (a!=1.0) m_translucent = true;
    else m_translucent = false;
}

void PCoord::SetForeground( unsigned long color, float a )
{
    SetForeground(Color(color),a);
}

void PCoord::UpdateSampleSize(int sample) 
{
	int temp_sample = sample / 20;
	if(temp_sample > 0){
		temp_sample = sample - rand()%temp_sample;
		if (sample >= 1)
			sample = temp_sample; 
	}
	m_sample = sample; 
	if(m_sample == 0) 
		m_sample = 1; 
	redraw();
}

double PCoord::YMap(int xc, float yc, int dim_num)
{ 
	return (m_windowHeight*0.7) * yc;
	//return (m_windowHeight-30) * yc;
}


void PCoord::SetSelectedData(float* buffer, int height, int width)
{
	int buffersize = width * height;
	if(m_regiondata)
		delete [] m_regiondata;
	m_regiondata = new float[buffersize];
	//TODO: do we have to keep copying the data
	// on every slice redraw
	for(int i=0; i < buffersize;i++)
		m_regiondata[i] = buffer[i];	
}
