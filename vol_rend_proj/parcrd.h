#ifndef PAR_CRD
#define PAR_CRD
#ifndef LINUX
#include <windows.h>
#endif
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
#include <FL/fl_draw.H>
#include "color.h"
#include "vec2.h"
//#include "FL/glut.h"
#include "netcdf_read.h"
#include "cxvolume.h"

#define COLOR_CHOOSER_WIDTH  180
#define COLOR_CHOOSER_HEIGHT 60
#define MARGIN 20 //15

class TestDataset
{
public:
	TestDataset()	{}
	~TestDataset()	{}
	float* dim_min;
	float* dim_max;
	char** names;
	int m_dim;
private:

};



class PCoord : public Fl_Gl_Window
{
public:
	PCoord(int X,int Y,int W,int H, cxVolume * pVolume, int sample=1);
	~PCoord();
	void draw();

	bool SetInitialTimeStep(int new_start_time_step);
	bool InitData();
	void InitParentWindow(Fl_Widget* wind) 
	{	m_wind = wind; }
	inline double XMap(float i) 
	{ return (m_windowWidth) * (((i)) / m_datautils->get_num_vars()); }
	//{ return (m_windowWidth) * (((i)+0.5) / m_datautils->get_num_vars()); }

	//inline double YMap(int xc, float yc)
	//{ return (m_windowHeight- y()) * (1.0-((yc) - m_datautils->dim_min[xc]) / 
	//			(m_datautils->dim_max[xc] - m_datautils->dim_min[xc])); }
/*
	inline double YMap(int xc, float yc)
	{ return (m_windowHeight-30) * (((yc) - m_datautils->dim_min[xc]) / 
				(m_datautils->dim_max[xc] - m_datautils->dim_min[xc])); }
	
	inline double YinvMap(int x, float y) 
	{ return (m_windowHeight-30) * (((1.0-(y)) * (m_datautils->dim_max[x] - 
				m_datautils->dim_min[x])) + m_datautils->dim_min[x]); }
*/
	
	double YMap(int xc, float yc, int dimnum);
	/*
	inline double YinvMap(int x, float y) 
	{ return (m_windowHeight-30) * (((1.0-(y)) * (m_datautils->dim_max[x] - 
				m_datautils->dim_min[x])) + m_datautils->dim_min[x]); }
	*/
	
	void SetSelectedData(float* buffer, int height, int width);
	int GetContextCoord(bool start, char axis);
public:
	void DrawLine( float x1, float y1, float x2, float y2);
	void BeginDraw();
	void EndDraw();
	void Display();
	
	void DrawAxes();
	void DrawData();

public:

	void DrawConfinedString( float x, float y, const char *text, float width);
	void DrawString( float x, float y, const char *text, int N );
	void DrawString( const char *text, int N );

	void SetForeground( const Color &color, float a );
	void SetForeground( unsigned long color, float a );

	void SetDisplaySize(float windowWidth, float windowHeight)
	{	m_windowWidth = windowWidth; m_windowHeight = windowHeight;}
	
	void SetNetCDF(NetCDF* netcdf) { m_netcdf = netcdf; }

	bool ChangeDataSize(int new_size, int new_count, char axis, int* out_val = NULL);
	void UpdateSampleSize(int sample);
	void EnablePCoord(bool bEnable) { m_bDisplay = bEnable;}
	
	//bool SaveSubset(int x_start, int y_start, int z_start,
	//		     int x_end, int y_end, int z_end,
	//		     int time_in, int time_count);
private:
	bool m_init;
	//TestDataset* m_data;

        bool m_bDisplay;
	bool m_xor;
	bool m_translucent;
	bool m_flatcolor;
	Color m_fg_color;
	Color m_bg_color;		//	background color
	float m_alpha;
	/*GLuint*/ unsigned int m_font_base;

	float m_windowWidth;
	float m_windowHeight;
	Fl_Widget* m_wind;

	float m_x;
	float m_y;


	//TODO: dangerous use of pointer. If a caller suddenly deallocates or changes its pointer we
	// will enter the wrold of pain.
	NetCDF* m_netcdf;
	NetCDFUtils* m_datautils;
	int m_start_time; // initial timestep
	float** m_data;
	int m_numvars;

	Fl_Check_Button *m_pCheck1;
	Filter* m_fltr;

	cxVolume * m_pVolume;
	int m_sample;
	float* m_regiondata;
	
	int m_context_x_start;
	int m_context_x_end;
	int m_context_y_start;
	int m_context_y_end;
	int m_context_z_start;
	int m_context_z_end;
};

#endif // PAR_CRD
