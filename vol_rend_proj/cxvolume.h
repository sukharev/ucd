//
// C++ Interface: cxvolume
//
// Description: 
//
//
// Author: Hongfeng Yu <hfyu@ucdavis.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CXVOLUME_H
#define CXVOLUME_H

/**
	@author Hongfeng Yu <hfyu@ucdavis.edu>
*/

#include "cx3dvector.h"
#include "wincolormap.h"
#include "netcdf_read.h"
#include "marchingsquares.h"
#include "parcrd.h"
#include "clusterview2d.h"

#ifdef LINUX
#define MAX_PATH 512
#endif

#include <string>
#include <vector>

using namespace std;

#define VOLUME_TYPE_FLOAT   10000
#define VOLUME_TYPE_SHORT   10001
#define VOLUME_TYPE_BYTE    10002

#define SLICE_X_AXIS 10000
#define SLICE_Y_AXIS 10001
#define SLICE_Z_AXIS 10002

#define DRAW_2D_SLICE 20000
#define DRAW_3D_VOLUME 20001

/*combustion*/
#define DATA_MIXFRAC    3000
#define DATA_HO2        3001
#define DATA_OH         3002
#define DATA_CHI        3003

/*climate*/
#define DATA_TEMP       3004
#define DATA_SALT       3005
#define DATA_WT       3006

#define DATA_TEMP_A     3007
#define DATA_SALT_A		3008

#define DATA_CORR		3009
#define DATA_UNKNOWN    3010
#define DATA_TEMP_TEMP	3011
#define DATA_SALT_SALT	3012
#define DATA_WT_WT		3013
#define DATA_SALT_TEMP	3014
#define DATA_WT_TEMP	3015
#define DATA_WT_SALT	3016


#define GRID_SAMPLE     1024

class PCoord;
class WinVisView;
class WinSliceView;

class cxVisStatus
{
public:
    cxVisStatus();
    ~cxVisStatus();
    
    void Copy(cxVisStatus * pVisStatus);
    void Interpolate(cxVisStatus * pPrev, cxVisStatus * pNext, float weight);
    
    void WriteToFile();
    void ReadFromFile();
    
public:
    /* colormap settins : transfer function*/
    cxColor m_Colors[TF_SIZE];
      
    /* colormap settings : buttons*/
    vector<float> m_vTFButtonSettings;
    
    /* colormap settings : lines*/
    vector<float> m_vTFLineSettings;
    
    /* view settings : draw axes*/
    bool            m_bDrawAxes;
    
    /* view settings : draw frame*/
    bool            m_bDrawFrame;    
    
    /* view settings : draw volume*/
    bool            m_bDrawVolume;
        
    /* view settings : the min cutting plan aligned with axes*/
    cx3DVector      m_vRangeMin;    
    
    /* view settings : the max cutting plan aligned with axes*/
    cx3DVector      m_vRangeMax;
    
    /* effect settings : lighting parameters - ka, kd, ks and shiness*/    
    float           m_fLightPar[4];       
    
    /* effect settings : sampling spacing*/
    float           m_fSampleSpacing; 
    
    /* slicing position : x axis*/
    float           m_fSlicePosX;
    
    /* slicing position : y axis*/
    float           m_fSlicePosY;
    
    /* slicing position : z axis*/
    float           m_fSlicePosZ;
    
    float           m_fShiftDis;
    
    /* view */
    float m_curquat[4];
    float m_scale;
    float m_deltax;
    float m_deltay;
    
    int m_nVolumeID;

	/* Pearson correlation reference point */
	float m_fXLocation;
	float m_fYLocation;
	float m_fZLocation;

	/* for Point clouds */
	int xblock;
	int yblock;
	int zblock;
	float thold;
};


class cxVolume
{
public:
    cxVolume(int volID = 0);
    ~cxVolume();
    
public:
	bool vartominmax(int var, float* pdata, float* arr_fMin, 
						   float* arr_fMax, int var_index, int timesteps, int sizevol);
	bool calculate_corr(const char* filename, string var_names[],  //requested variable names
						   int num_vars);
public:
    //Draw
    void ReDraw(bool bReDrawAll = true);
    void Draw(int nDrawMode = DRAW_3D_VOLUME, int nSliceAxis = SLICE_X_AXIS, int h=0, int w=0);    
    void GetSnapshot(unsigned char * output, int width, int height);
        
    void SetSliceAxis(int axis){
        m_nSliceAxis = axis;
    }
    
    //Get
    cx3DVector GetSize();
    cx3DVector GetCeneter();
    
    //Set
    void SetSize(cx3DVector size);
    void SetSize(int x, int y, int z);
	void SetTimeLen(int time);
    void SetCenter(cx3DVector center);
    void SetCenter(int x, int y, int z);   
    
    void SetVisView(WinVisView * pVisView) {
        m_pVisView = pVisView;
    }

	int GetSizeSlice(int axis, char dim);

public:    
    //Initialize the volume
    void InitVolume();    
    
    //Initialize the shader
    void InitProgram();
    
    //Read volume: first read header file, then read the data file
    void Read();

    void Read(const char* listitem, 
			  const char* listitem2 = NULL, 
			  bool bCorrelation = false);

    void ReadNetCDF(NetCDF* netcdf, const char* index, int time_in, int time_out);
    
	void ReadNetCDF3(NetCDF* netcdf,		//opened NetCDF file pointer
						   int time_in,			//initial timestep 
						   int count,			//number of timesteps requested (no less than 1)
						   string var_names[],  //requested variable names
						   int num_vars,			//number of variables requested (no more than 2)
						   bool savetac =true);

	void ReadNetCDF2(NetCDF* netcdf,		//opened NetCDF file pointer
						   int time_in,			//initial timestep 
						   int count,			//number of timesteps requested (no less than 1)
						   string var_names[],  //requested variable names
						   int num_vars			//number of variables requested (no more than 2)
						   );

	void ReadFlexible(NetCDF* netcdf,		//opened NetCDF file pointer
							int time_in,			//initial timestep 
							int count,			//number of timesteps requested (no less than 1)
							const char* var_name	//requested variable name
							);
#ifdef OLD_CODE
	void ReadFile();
    
    void ReadHeader(char * filename);
    
    void ReadData(char * filename); 
#endif           
    void ReadGrid(char * filename);

	void ReadGrid(NetCFD_var* var_info);
    
    void ReadTime(char * filename);
        
    void DrawFrame();
    void DrawVolSel();
    
	void DrawReferenceLocation();

    void DrawAxes();
    
    void Draw3DVolume();
    
    void Draw2DSlice(int nSliceAxis, int h=0, int w=0);
        
    void Draw2DSliceBoundary();
    
    void DrawVolume();
    
    void DrawCube();
    
    void DrawColorMap();
#ifdef POINT_CLOUD
	void DrawPointCloud();
#endif
    void DrawImportance();
    
    void DrawGrid();
    
    void CreateRayDirTex();
        
    void CreateDataTex();

	void UpdatePointCloudSettings(int new_xblock, int new_yblock, int new_zblock, float new_thold);
	
	void CreateDataTexPearson();

#ifdef SHOW_CLUSTER
	void CreateClusterTex();
	void UpdateClusterId();
#endif

#ifdef SV_CORRELATION
	void CreateCorrTex();
#endif

	void CreateIndexDataTex();
    
    void CreateTfTex();
    
    void CreateColorMapTex();
    
    void CreateGridTex();
    
    void TurnOffTextureAndPrograms();
    
    void ClearVolume();

    void ClearVolTex();
    
	void ClearVolTexPearson();

    void GetEyePos();
    
    bool Forward();
    
    void CreateHistogram();

    void CalculateContourLines(int contourpercent, int num_saved);

    NetCDF* openNetCDF_file(char* filename);
    void SetSelected2DRegion(int axis, float x1, float y1, float x2, float y2);
    void EnableSelection(bool selection);
    
    bool IsSelectionEnabled() { return m_selection;}
	void SetCorrCenterPoint(int axis, float x1, float y1);
	float AllignToGrid(int axis, float old_coord);
        
	int GetStartTimeStep() {return m_time_in; }

	void SetMinMax(float min1, float max1, float min2, float max2)
	{	
		m_fMin1 = min1;
		m_fMax1 = max1;
		m_fMin2 = min2;
		m_fMax2 = max2;
	}
	void GetMinMax(float& min1, float& max1, float& min2, float& max2)
	{
		min1 = m_fMin1;
		max1 = m_fMax1;
		min2 = m_fMin2;
		max2 = m_fMax2;
	}
	void getLabels_ijk(char xlabel[], char ylabel[], char zlabel[]);

public:
    NetCDF* m_netcdf;
    /* current NetCDF variable index */
    char m_curnetcdf_var[MAX_PATH];   

    /* the volume file*/
    string          m_sFilename;
    
    /* memory buffer for volume*/
    float*          m_pVolume;

    /* memory buffer for index volume*/
    float*          m_pIndVolume;
    
    /* volume type: float, byte or unsigned short*/
    int             m_nVolumeType;
    
    /* size of memory in bytes*/
    int             m_nVolumeSize;
    
    /* volume demension*/
    cx3DVector      m_vSize;    

	int				m_timelen;
    
    /* the maximum edge size*/
    float           m_fMaxSize;
    
    /* center of the volume*/
    cx3DVector      m_vCenter;    
    
    /* shader*/
    unsigned int    m_nVolProgram;    

    /* 3D volume texture*/
    unsigned int    m_nTexVol;    

#ifdef SHOW_CLUSTER
	/* 3D volume cluster texture*/
    unsigned int    m_nTexClus;
	ClusterView2D   *m_pClusView;
#endif

#ifdef SV_CORRELATION
	/* 3D volume cluster texture*/
    unsigned int    m_nTexCorr;
#endif

    /* 3D index volume texture*/
    unsigned int   m_nIndTexVol;
    
    /* 2D texutre for 3 back faces*/
    unsigned int    m_nTexBack;
    
    /* 1D TF texture*/
    unsigned int    m_nTexTF;
    
    /* 2D Colormap texture*/
    unsigned int    m_nTexColorMap;
    
    /* 1D Grid texture */
    unsigned int    m_nTexGrid[3];
        
    /* the normalized spacing for alpha adjusting*/
    float           m_fNormalSpacing;
    
    /* indicator for drawing which slice*/    
    int             m_nSliceAxis;
    
    /* pointer to the vis view*/
    WinVisView *    m_pVisView;
    
    /* pointers to the three slice views*/
    WinSliceView *  m_pSliceView[3];
    
    /* eye position*/
    cx3DVector      m_vEye;
    
#ifdef ORTHO
   
    /* eye direction */
    cx3DVector      m_vEyeDir;

#endif

    /* draw the slice boundary*/
    bool            m_bDraw2DSliceBoundary;
    
    /* draw the color map*/
    bool            m_bDrawColorMap;
    
    /* volume id for multi_variables rendering*/
    int             m_nVolumeID;
    
    /* start time step */
    int             m_nStartTime;
    
    
    /* end time step */
    int             m_nEndTime;
    
    /* current rendered time step */
    int             m_nCurTime;
    
    /* variable */
    int             m_nVariable;
    
    /* number of importance values to show */
    
    int             m_nImportance;
    
    WinHistogram *  m_pHistogram;

    cxVisStatus *   m_pVisStatus;
    
    float           *m_pGrid[3];   
	int			*m_GridSpace[3];
    float	    *m_GridMap[3];
    float           *m_pTime; 
    
    bool m_bReload;
    
    float           m_fMin1;
	float           m_fMax1;
	float           m_fMin2;
	float           m_fMax2;

	bool			m_bcorr;

	bool			m_bDrawPointCloud;

public:

    void SetTimePeriod(int time_in, int time_out);
    void SetCurVar(char* varname);


    void SetYstop(int size);
    void SetXstop(int size);
    void SetYstart(int start);
    void SetXstart(int start);

    void EnableLineContours(bool option) {  m_bContours = option; }
    bool IsEnabledLineContours() { return m_bContours;}

    void EnableColorContours(bool option){  m_bColorContours = option; }
    bool IsEnabledColorContours() { return m_bColorContours;}

	void CalcSVCorrelation(float* x, float* y, int N, float& correlation);

#ifdef SHOW_CLUSTER
	void SetClusterView(ClusterView2D *pClusView) {m_pClusView = pClusView; }
#endif

	bool IsSelected2DRegion();
    void Discard2DRegion();
    bool GetSelected2DRegion(int* x1, int* x2, int* y1, int* y2, int* z1, int* z2);
    void SetParCoord(PCoord* pCoord) {m_pCoord = pCoord;}
    void SetVolSelect(int x1, int x2, int y1, int y2, int z1, int z2)
    {
    	m_context_x_start = x1; 
    	m_context_x_end = x2;
    	m_context_y_start = y1;
    	m_context_y_end = y2;
    	m_context_z_start = z1;
    	m_context_z_end = z2;
    	m_bvolsel = true;
    }
    
    void SetTAC_finish(int TAC_finish)
    {
    	m_TAC_finish = TAC_finish;
    }
    
    void SaveTAC_vol();
	void SaveGTAC_vol();
	void SaveCustomTAC_vol(char* filename, int start, int count);
	/*void SaveTACSubSet(	int x_start, int x_end, 
						int y_start, int y_end, 
						int z_start, int z_end,
						int time_in, int time_count);
	*/

	void getLabels(char xlabel[], char ylabel[], char zlabel[]);

	void SetCorrFreeze(bool freeze){m_bCorrFreeze = freeze;}
private:
    // rectangular bounds for the highlighted section of the VR
    // it gets regularly updated by Parallel Coordinates's sliders.
    int m_startIndX;
    int m_stopIndX;
    int m_startIndY;
    int m_stopIndY;
    int m_IndYsize;
    int m_IndXsize;

    int m_time_in;
    int m_time_out;

    float m_contour_value;
    bool m_bContours;
    bool m_bColorContours;
    int m_contour_num_saved;
    MarchingSquares* m_ms;


    int m_selectx1;
    int m_selectx2;
    int m_selecty1;
    int m_selecty2;
    int m_selectz1;
    int m_selectz2;

	float m_corrx;
	float m_corry;
	float m_corrz;

    bool m_selection;
    PCoord* m_pCoord;
    
    
    int m_context_x_start; 
    int m_context_x_end;
    int m_context_y_start;
    int m_context_y_end;
    int m_context_z_start;
    int m_context_z_end;
    bool m_bvolsel;
    
    int m_TAC_start;
    int m_TAC_finish;

	float** m_corrdata;
	float* m_corrmap;

    bool		m_bPearsonVariable;
    int         m_nStartTimePearson;
    int         m_nEndTimePearson;
    int         m_nNumTimePearson;
	unsigned int    m_nTexVolPearson;
	//unsigned int    m_nTexVolPearsonNext;

	//float*          m_pVolumePearson;
	int			m_nLocationPearsonStart;
	int			m_nLocationPearsonEnd;
	int			m_nLocationPearson;
	int         *m_nXLocationPearson;
	int         *m_nYLocationPearson;
	int         *m_nZLocationPearson;
	bool        m_bLocationPearsonAnimation;
#ifdef POINT_CLOUD
	int			m_nt_pointcloud;
#endif	
	// used to move the representation of the dataset to the left or to the right
	float m_fOffsetX;

	char m_bCorrItem1[MAX_PATH];
	char m_bCorrItem2[MAX_PATH];
public:
	// this flag activates/deactivates Pearson correlation
	bool m_bCorrelation;
	bool m_bCorrFreeze;
	bool IsCorrelationActive() { return m_bCorrelation;}

private:
	string m_vars[2];
};

#endif
