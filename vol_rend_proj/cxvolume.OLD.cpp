//
// C++ Implementation: cxvolume
//
// Description: 
//
//
// Author: Hongfeng Yu <hfyu@ucdavis.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <iostream>
#include <fstream>
#include <string>
#include "GL/glew.h"
#include <FL/glut.H>
#include "cxvolume.h"
#include "cxparameters.h"
#include "shader.h"
#include "winvisview.h"
#include "winsliceview.h"
#include "cxmatrix_c.h"
#include "kmlsample.h"

#ifdef LINUX
#include <unistd.h>
#endif

#include <iomanip>
#include <sstream>
#include <cmath>

#ifdef LINUX
/*extern "C" {
#include <udunits.h>
}
*/
#endif

using namespace std;

void swapvalues(int& start, int& end)
{
	int temp = 0;
	if(start > end){
		temp = start;
		start = end;
		end = temp;
	}
}

void slerp(float * qa, float * qb, float * qm, double t) {

	// Calculate angle between them.
	double cosHalfTheta = qa[3] * qb[3] + qa[0] * qb[0] + qa[1] * qb[1] + qa[2] * qb[2];

	// if qa=qb or qa=-qb then theta = 0 and we can return qa
	if (abs(cosHalfTheta) >= 1.0){
		qm[3] = qa[3]; qm[0] = qa[0]; qm[1] = qa[1]; qm[2] = qa[2];
		return;
	}
	// Calculate temporary values.
	double halfTheta = acos(cosHalfTheta);
	double sinHalfTheta = sqrt(1.0 - cosHalfTheta*cosHalfTheta);
	// if theta = 180 degrees then result is not fully defined
	// we could rotate around any axis normal to qa or qb
	if (fabs(sinHalfTheta) < 0.001){ // fabs is floating point absolute
		qm[3] = (qa[3] * 0.5 + qb[3] * 0.5);
		qm[0] = (qa[0] * 0.5 + qb[0] * 0.5);
		qm[1] = (qa[1] * 0.5 + qb[1] * 0.5);
		qm[2] = (qa[2] * 0.5 + qb[2] * 0.5);
		return;
	}
	double ratioA = sin((1 - t) * halfTheta) / sinHalfTheta;
	double ratioB = sin(t * halfTheta) / sinHalfTheta; 
	//calculate Quaternion.
	qm[3] = (qa[3] * ratioA + qb[3] * ratioB);
	qm[0] = (qa[0] * ratioA + qb[0] * ratioB);
	qm[1] = (qa[1] * ratioA + qb[1] * ratioB);
	qm[2] = (qa[2] * ratioA + qb[2] * ratioB);    
}

cxVisStatus::cxVisStatus()
{
	m_bDrawAxes = true;        
	m_bDrawFrame = true;
	m_bDrawVolume = true;
	m_vRangeMin = cx3DVector(0, 0, 0);
	//m_vRangeMin = cx3DVector(0, 0.5, 0);
	m_vRangeMax = cx3DVector(1, 1, 1);
	m_fLightPar[0] = 0.1;
	m_fLightPar[1] = 0.9;
	m_fLightPar[2] = 0.6;
	m_fLightPar[3] = 20.0;
	m_fSampleSpacing = 1.0/128.0;
	m_fSlicePosX = 0;
	m_fSlicePosY = 0;
	m_fSlicePosZ = 0;
	m_fShiftDis = 0;

	m_fXLocation = 0.535;
	m_fYLocation = 0.5;
	m_fZLocation = 1.0;

	xblock = 6;
	yblock = 3;
	zblock = 3;
	thold = 0.00008;
}

cxVisStatus::~cxVisStatus()
{
}

void cxVisStatus::Copy(cxVisStatus * pVisStatus)
{
	assert(pVisStatus);

	memcpy(m_Colors, pVisStatus->m_Colors, sizeof(cxColor) * TF_SIZE);

	m_vTFButtonSettings.clear();
	int size = pVisStatus->m_vTFButtonSettings.size();
	for ( int i = 0; i < size; i++)
		m_vTFButtonSettings.push_back(pVisStatus->m_vTFButtonSettings[i]);

	m_vTFLineSettings.clear();
	size = pVisStatus->m_vTFLineSettings.size();
	for ( int i = 0; i < size; i++)
		m_vTFLineSettings.push_back(pVisStatus->m_vTFLineSettings[i]);

	m_bDrawAxes = pVisStatus->m_bDrawAxes;

	m_bDrawFrame = pVisStatus->m_bDrawFrame;

	m_bDrawVolume = pVisStatus->m_bDrawVolume;

	m_vRangeMin = pVisStatus->m_vRangeMin;

	m_vRangeMax = pVisStatus->m_vRangeMax;

	memcpy(m_fLightPar, pVisStatus->m_fLightPar, sizeof(float) * 4);

	m_fSampleSpacing = pVisStatus->m_fSampleSpacing;

	m_fSlicePosX = pVisStatus->m_fSlicePosX;

	m_fSlicePosY = pVisStatus->m_fSlicePosY;

	m_fSlicePosZ = pVisStatus->m_fSlicePosZ;

	m_fShiftDis = pVisStatus->m_fShiftDis;

	memcpy(m_curquat, pVisStatus->m_curquat, sizeof(float) * 4);
	m_scale = pVisStatus->m_scale;
	m_deltax = pVisStatus->m_deltax;
	m_deltay = pVisStatus->m_deltay;
}


void cxVisStatus::Interpolate(cxVisStatus * pPrev, cxVisStatus * pNext, float weight)
{
	assert(pPrev);
	assert(pNext);

	if (weight < 0.5)
		Copy(pNext);
	else
		Copy(pPrev);

	for ( int i = 0; i < TF_SIZE; i++) {
		m_Colors[i].r() = pPrev->m_Colors[i].r() * weight + pNext->m_Colors[i].r() * ( 1.0 - weight);
		m_Colors[i].g() = pPrev->m_Colors[i].g() * weight + pNext->m_Colors[i].g() * ( 1.0 - weight);
		m_Colors[i].b() = pPrev->m_Colors[i].b() * weight + pNext->m_Colors[i].b() * ( 1.0 - weight);
		m_Colors[i].a() = pPrev->m_Colors[i].a() * weight + pNext->m_Colors[i].a() * ( 1.0 - weight);
	}

	m_vRangeMin = pPrev->m_vRangeMin * weight + pNext->m_vRangeMin * ( 1.0 - weight);

	m_vRangeMax = pPrev->m_vRangeMax * weight + pNext->m_vRangeMax * ( 1.0 - weight);

	for ( int i = 0; i < 4; i++) {
		m_fLightPar[i] = pPrev->m_fLightPar[i] * weight + pNext->m_fLightPar[i] * (1.0 - weight);
	}

	m_fSampleSpacing = pPrev->m_fSampleSpacing * weight + pNext->m_fSampleSpacing * ( 1.0 - weight);

	m_fSlicePosX = pPrev->m_fSlicePosX * weight + pNext->m_fSlicePosX * ( 1.0 - weight);

	m_fSlicePosY = pPrev->m_fSlicePosY * weight + pNext->m_fSlicePosY * ( 1.0 - weight);

	m_fSlicePosZ = pPrev->m_fSlicePosZ * weight + pNext->m_fSlicePosZ * ( 1.0 - weight);

	m_fShiftDis = pPrev->m_fShiftDis * weight + pNext->m_fShiftDis * ( 1.0 - weight);

	/* slerp interpolation of two quaternion rotations*/
	slerp(pPrev->m_curquat, pNext->m_curquat, m_curquat, 1.0 - weight);


	m_scale = pPrev->m_scale * weight + pNext->m_scale * ( 1.0 - weight);

	m_deltax = pPrev->m_deltax * weight + pNext->m_deltax * ( 1.0 - weight);

	m_deltay = pPrev->m_deltay * weight + pNext->m_deltay * ( 1.0 - weight);
}

void cxVisStatus::WriteToFile()
{
	char filename[1024];
#ifdef MULTI_VARIABLES
	sprintf(filename, "ctrlstatus%d.cfg", m_nVolumeID);
#else
	sprintf(filename, "ctrlstatus.cfg");
#endif

	ofstream outf(filename);
	assert(outf);


	outf.write(reinterpret_cast<char *>(m_Colors), sizeof(cxColor)*TF_SIZE);    
	outf.write(reinterpret_cast<char *>(m_fLightPar), sizeof(float) * 4);
	outf.write(reinterpret_cast<char *>(&m_fSampleSpacing), sizeof(float));
	outf.write(reinterpret_cast<char *>(m_curquat), sizeof(float)*4);
	outf.write(reinterpret_cast<char *>(&m_scale), sizeof(float));
	outf.write(reinterpret_cast<char *>(&m_deltax), sizeof(float));
	outf.write(reinterpret_cast<char *>(&m_deltay), sizeof(float));
	outf.write(reinterpret_cast<char *>(&m_fShiftDis), sizeof(float));

	outf.close();
}

void cxVisStatus::ReadFromFile()
{
	char filename[1024];
#ifdef MULTI_VARIABLES
	sprintf(filename, "ctrlstatus%d.cfg", m_nVolumeID);
#else
	sprintf(filename, "ctrlstatus.cfg");
#endif

	ifstream inf(filename);

	if (!inf)
		return;

	inf.read(reinterpret_cast<char *>(m_Colors), sizeof(cxColor)*TF_SIZE);    
	inf.read(reinterpret_cast<char *>(m_fLightPar), sizeof(float) * 4);
	inf.read(reinterpret_cast<char *>(&m_fSampleSpacing), sizeof(float));
	inf.read(reinterpret_cast<char *>(m_curquat), sizeof(float)*4);
	inf.read(reinterpret_cast<char *>(&m_scale), sizeof(float));
	inf.read(reinterpret_cast<char *>(&m_deltax), sizeof(float));
	inf.read(reinterpret_cast<char *>(&m_deltay), sizeof(float));
	inf.read(reinterpret_cast<char *>(&m_fShiftDis), sizeof(float));

	inf.close();

}


cxVolume::cxVolume(int volID)
{
	m_pVolume = NULL;

	m_pIndVolume = NULL;
	/*
	m_startIndX = 0;
	m_stopIndX = 0;
	m_startIndY = 99;
	m_stopIndY = 99;
	m_IndYsize = 100;
	m_IndXsize = 100;
	*/
	m_startIndX = 0;
	m_stopIndX = 99;
	m_startIndY = 0;
	m_stopIndY = 99;
	m_IndYsize = 100;
	m_IndXsize = 100;

	m_time_in = 0;
	m_time_out = 1;
	m_timelen = 0;

	m_nVolumeType = VOLUME_TYPE_FLOAT;
	m_nTexVol = 0;
	m_nTexVolPearson = 0;
#ifdef SHOW_CLUSTER
	m_nTexClus = 0;
	m_pClusView = NULL;
#endif

#ifdef SV_CORRELATION
	m_nTexCorr = 0;
#endif

	m_nIndTexVol = 0;
	m_nTexBack = 0;
	m_nVolProgram = 0;
	m_fNormalSpacing = 1.0/64.0;     
	m_nSliceAxis = SLICE_X_AXIS;
	m_bDraw2DSliceBoundary = false;
	m_pVisStatus = new cxVisStatus;
	m_pVisStatus->m_nVolumeID = volID;

	m_bReload = false;

	m_nTexColorMap = 0;
	m_nTexTF = 0;
	m_bDrawColorMap = true;

	for ( int i = 0; i < 3; i++){
		m_pGrid[i] = NULL;
		m_nTexGrid[i] = 0;
		m_GridSpace[i] = NULL;
	}

	m_nVolumeID = volID;
	m_nImportance = 5;
	InitVolume();

	m_pTime = NULL;

#ifdef LINUX
/*	if(utInit("/etc/udunits.dat") != 0){
		printf("Error! Can not init udunits\n");
	}
*/
#endif

	m_netcdf = NULL;
	m_contour_value = 0; //TODO: what should be a good m_contour_value?
	m_bContours = false;
	m_bColorContours = false;
	m_contour_num_saved = 0;
	m_ms = NULL;

	m_selection = false;
	m_pCoord = NULL;

	m_context_x_start = 0; 
	m_context_x_end = 0;
	m_context_y_start = 0;
	m_context_y_end = 0;
	m_context_z_start = 0;
	m_context_z_end = 0;
	m_bvolsel = false;

	m_TAC_start = 0;
	m_TAC_finish = 1;

	m_GridMap[0] = NULL;
	m_GridMap[1] = NULL;
	m_GridMap[2] = NULL;

	m_pSliceView[0] = NULL;
	m_pSliceView[1] = NULL;
	m_pSliceView[2] = NULL;

	m_bcorr = false;
	m_corrdata = NULL;
	m_corrmap = NULL;

	m_selectx1=m_selectx2=m_selecty1=m_selecty2=m_selectz1=m_selectz2=0;
#ifdef CLIMATE
	m_fOffsetX = 0.0;//-0.15;
#else
	m_fOffsetX = 0;
#endif

	/*
	// two variables
	m_bPearsonVariable = true;
	//m_nStartTimePearson = 280;
	//m_nEndTimePearson = 297;
	//m_nNumTimePearson = 36;
	m_nStartTimePearson = 0;
	m_nEndTimePearson = 17;
	m_nNumTimePearson = 36;
	*/

	//Pearson Correlation
	m_bCorrelation = false; // this flag activates all Pearson correlation.

	// one variable
	m_bPearsonVariable = false;
	//m_nStartTimePearson = 0;
	//m_nEndTimePearson = 17;  
	m_nStartTimePearson = 280;
	//m_nEndTimePearson = 315;
	//m_nNumTimePearson = 12; //36;  // same as NetCDF count of timesteps
	m_nEndTimePearson = 351;
	m_nNumTimePearson = 12;
	//m_nStartTimePearson = 0;
	//m_nEndTimePearson = 35;
	//m_nNumTimePearson = 36;

	//m_bLocationPearsonAnimation = false;

	m_nLocationPearsonStart = 0;
	m_nLocationPearsonEnd = 291;
	m_nLocationPearson = 0;
	m_bLocationPearsonAnimation = true;
	m_nXLocationPearson = NULL;
	m_nYLocationPearson = NULL;
	m_nZLocationPearson = NULL;

	m_corrx = 0.535;
	m_corry = 0.5;
	m_corrz = 1.0;
	strcpy(m_bCorrItem1,"");
	strcpy(m_bCorrItem2,"");

	m_bDrawPointCloud = false;

	m_bCorrFreeze=false;
#ifdef POINT_CLOUD
	m_nt_pointcloud = 10; // number of timesteps used in variance map calculation
#endif
	m_fMin1 = 0.0;
	m_fMin2 = 0.0;
	m_fMax1 = 0.0;
	m_fMax2 = 0.0;
}

cxVolume::~cxVolume()
{
	ClearVolume();
	if(m_netcdf)
		delete m_netcdf;
	if(m_ms){
		delete m_ms;
		m_ms = NULL;
	}
}


cx3DVector cxVolume::GetCeneter()
{
	return m_vCenter;
}

void cxVolume::CalculateContourLines(int contourpercent, int num_saved)
{
	if(m_pVolume && m_bContours){
		//float fRange = m_fMax - m_fMin;
		m_contour_num_saved = num_saved;
		//m_contour_value = m_fMin + contourpercent * fRange / 100;

		//assuming we are using our simple QUANTIZATION method
		m_contour_value = (float)contourpercent  / 100;
		if (!m_ms){
			m_ms = new MarchingSquares(m_contour_value, m_GridMap, num_saved, m_time_in);
			//m_ms->compute(m_vSize[0], m_vSize[1], m_vSize[2], m_pVolume); 
			//delete m_ms;
			//m_ms = NULL;
		}
		else{
			m_ms->update(m_contour_value, num_saved, m_time_in);
			//m_ms->compute(m_vSize[0], m_vSize[1], m_vSize[2], m_pVolume); 
		}

	}
}

unsigned char Stretch_Linear(int x, int y, int desX, int desY, int srcX, int srcY, const unsigned char * Vx)
{
	float xx = x*srcX/((float)desX);
	float yy = y*srcY/((float)desY);

	float n = xx - ((int)xx);
	float b = yy - ((int)yy);

	int PAX = (int)xx;
	int PAY = (int)yy;
	int PBX = (PAX+1 > srcX-1)? PAX : PAX+1;
	int PBY = PAY;
	int PCX = PAX;
	int PCY = (PAY+1 > srcY-1)? PAY : PAY+1;
	int PDX = PBX;
	int PDY = PCY;

	int idA = PAX + PAY * srcX;
	int idB = PBX + PBY * srcX;
	int idC = PCX + PCY * srcX;
	int idD = PDX + PDY * srcX;

	float vx = n*b*Vx[idA] + n*(1-b)*Vx[idB] + (1-n)*b*Vx[idC] + (1-n)*(1-b)*Vx[idD];

	return (unsigned char)vx;
}

void cxVolume::GetSnapshot(unsigned char * output, int output_w, int output_h)
{
#ifdef WIN_SLICE
	int width = m_pVisView->m_nImagewidth;
	int height = m_pVisView->m_nImageheight;
	static unsigned char * buffer = m_pVisView->m_pImage;
#else
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);

	int width = viewport[2]; 
	int height = viewport[3];
	long buffersize = width*height*3;
	static unsigned char * buffer = new unsigned char[buffersize];    
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer); 
#endif    


	int colorsize = width*height;

	unsigned char *R = new unsigned char[colorsize];
	unsigned char *G = new unsigned char[colorsize];
	unsigned char *B = new unsigned char[colorsize];

	assert(output);
	assert(R);
	assert(G);
	assert(B);

	for ( int i = 0; i < colorsize; i++){
		R[i] = buffer[i*3+0];
		G[i] = buffer[i*3+1];
		B[i] = buffer[i*3+2];
	}

	for (int y = 0; y < output_h; y++)
		for (int x = 0; x < output_w; x++){        
			int id = x + y * output_w;
			y = output_h - y - 1;
			output[id*3 + 0] = Stretch_Linear(x, y, output_w, output_h, width, height, R);
			output[id*3 + 1] = Stretch_Linear(x, y, output_w, output_h, width, height, G);
			output[id*3 + 2] = Stretch_Linear(x, y, output_w, output_h, width, height, B);
		}

		delete [] R;
		delete [] G;
		delete [] B;
}


void cxVolume::SetSize(cx3DVector size)
{ 
	m_vSize = size;
	m_fMaxSize = m_vSize[0];

	if ( m_fMaxSize < m_vSize[1] )
		m_fMaxSize = m_vSize[1];

	if ( m_fMaxSize < m_vSize[2] )
		m_fMaxSize = m_vSize[2];

}

void cxVolume::SetTimeLen(int time)
{
	m_timelen = time;
}

void cxVolume::SetSize(int x, int y, int z)
{
	SetSize(cx3DVector(x, y, z));
}


void cxVolume::SetCenter(cx3DVector center)
{ 
	m_vCenter = center;
}

void cxVolume::SetCenter(int x, int y, int z)
{
	m_vCenter = cx3DVector(x, y, z);
}

void cxVolume::ReDraw(bool bReDrawAll)
{
#ifdef WIN_SLICE
	for ( int i = 0; i < 3; i++) {
		if (m_pSliceView[i] != NULL)
			m_pSliceView[i]->redraw();
	}
#endif    

	if (bReDrawAll) {        
		if (m_pVisView != NULL) {
			m_pVisView->redraw();
		}
	}

}

void cxVolume::ClearVolume()
{
	if (m_pVolume != NULL)
	{
		delete [] (float*) m_pVolume;
		m_pVolume = NULL;
	}

	if (m_pIndVolume != NULL)
	{
		delete [] (float*) m_pIndVolume;
		m_pIndVolume = NULL;
	}

}

void cxVolume::ClearVolTex()
{
	if (m_nTexVol != 0) {
		glDeleteTextures(1, (const GLuint*)&m_nTexVol);
		m_nTexVol = 0;
	}

	if (m_nIndTexVol != 0) {
		glDeleteTextures(1, (const GLuint*)&m_nIndTexVol);
		m_nIndTexVol = 0;
	}
#ifdef SHOW_CLUSTER
	if (m_nTexClus != 0) {
		glDeleteTextures(1, (const GLuint*)&m_nTexClus);
		m_nTexClus = 0;
	}
#endif

#ifdef SV_CORRELATION
	if (m_nTexCorr != 0) {
		glDeleteTextures(1, (const GLuint*)&m_nTexCorr);
		m_nTexCorr = 0;
	}
#endif

	if (m_nTexVolPearson != 0) {
		glDeleteTextures(1, (const GLuint*)&m_nTexVolPearson);
		m_nTexVolPearson = 0;
	}
}

void cxVolume::ClearVolTexPearson()
{
	if (m_nTexVolPearson != 0) {
		glDeleteTextures(1, (const GLuint*)&m_nTexVolPearson);
		m_nTexVolPearson = 0;
	}
}

void cxVolume::InitVolume()
{
	//read parameters from file
	cxParameters parameters;
	parameters.ParseFile("volumerender.cfg");
	m_sFilename = parameters.GetInputFile(m_nVolumeID); 
	m_nStartTime = parameters.m_nStartTime;
	m_nEndTime = parameters.m_nEndTime;
	m_nCurTime = m_nStartTime;  

	m_pVisStatus->ReadFromFile();
}

#ifdef OLD_CODE
void cxVolume::Read()
{

	ClearVolume();
	ClearVolTex();

	ReadFile();

	CreateDataTex();

	ClearVolume();

	//ReadGrid("C:/jeffs/uc_davis/src/vol_rend/vol_rend_proj/volumerender/grid.dat");

#ifdef LINUX
	//ReadTime("D:/jeffs/src/vol_rend/vol_rend_proj/volumerender/time.dat");
#endif
}

#endif

void cxVolume::Read(const char* listitem, 
					const char* listitem2, 
					bool bCorrelation)
{
	m_bCorrelation = bCorrelation;
	ClearVolume();
	ClearVolTex();

#ifdef POINT_CLOUD
	ReadFlexible(m_netcdf, m_time_in, m_nt_pointcloud, listitem);
	CreateDataTex();
	CreateIndexDataTex();
#else

	if(!bCorrelation){    
		ReadNetCDF(m_netcdf, listitem, m_time_in, m_time_out);
		CreateDataTex();
		CreateIndexDataTex();

	}else{
		strcpy(m_bCorrItem1,listitem);
		strcpy(m_bCorrItem2,listitem);
		// reading data to be used with (GPU) Pearson Correlation
		int num_vars = 1;
		m_bPearsonVariable = false;
		string vars[2];
		vars[0] = listitem;
		if(strcmp(listitem, listitem2)!=0){
			num_vars = 2;
			m_bPearsonVariable = true;
			vars[1] = listitem2;
		}

		ReadNetCDF2(m_netcdf, m_time_in, m_nNumTimePearson/*m_time_out*/, vars, num_vars);
		CreateDataTexPearson();
	}
#endif

#ifdef SHOW_CLUSTER
	CreateClusterTex();
#endif

#ifdef SV_CORRELATION
	CreateCorrTex();
#endif
	//ClearVolume();
	//ReadGrid("c:/jeffs/uc_davis/src/vol_rend/vol_rend_proj/volumerender/grid.dat");
#ifdef LINUX
//	ReadTime("D:/jeffs/src/vol_rend/vol_rend_proj/volumerender/time.dat");
#endif
}


bool cxVolume::Forward()
{
	m_nCurTime++;

	if (m_nCurTime > m_nEndTime) {
		m_nCurTime = m_nStartTime;
		m_pVisView->m_bEnd = true;
		return false;

	}

	return true;
}

//TODO: needs to be re-written
#ifdef PEARSONCPU
void cxVolume::ReadDataPearson(float mpVolume[], char * filehead, int const mnVariable, unsigned int mnTexVolPearson,
							   bool debug, char fn[], int start, int end)
{
	int ts; 
	//int volumesize = m_nVolumeSize / m_nNumTimePearson;
	char head_file[1024];
	char data_file[1024];
	char imp_file[1024];
	char temp[1024];
	char ts_string[256];
	float *ptVolume = NULL;
	ostringstream timestep;

	if (mnTexVolPearson != 0)  return;
	//if (mnTexVolPearson != 0)  
	//   glDeleteTextures(1, (const GLuint*)&mnTexVolPearson);

	//mpVolume = new float[m_nVolumeSize];
	//float *mpVolume = new float[m_nVolumeSize*m_nNumTimePearson];
	//if(mpVolume == NULL) {
	//    cerr << "Memory leak" << endl;
	//    return;
	//}
	ptVolume = mpVolume;

	for (ts=m_nStartTimePearson; ts<=m_nEndTimePearson; ts++)
	{
#ifdef HURRICANE
		timestep << setw(2) << setfill('0') << ts;
#else
		timestep << setw(4) << setfill('0') << ts;
#endif
		cout << "Load timestep " << ts << endl;

		sprintf(temp, "%s.dat", filehead);
		sprintf(ts_string, "%04d", ts);
		sprintf(data_file, temp, ts_string);
		printf("%s\n", data_file);

		ifstream datafile(data_file, ios::binary);

		if (datafile == NULL) {
			cerr << "Can not open " << data_file << endl;
			return;
		}    

		if (m_nVolumeType == VOLUME_TYPE_FLOAT) {
			printf("address = %d\n", ptVolume);
			datafile.read(reinterpret_cast<char *>(ptVolume), sizeof(float) * m_nVolumeSize);
			//datafile.read(reinterpret_cast<char *>(ptVolume), sizeof(float) * volumesize);
			//ptVolume += sizeof(float) * m_nVolumeSize;
			ptVolume += m_nVolumeSize;
			//ptVolume += volumesize;
		}
		datafile.close();
	}
	/*
	FILE *out;
	out = fopen("test_all.txt", "wb");
	for (int i=0; i<m_nVolumeSize; i++)
	fprintf(out, "%f\n", mpVolume[i]);
	fclose(out);
	*/
	// normalize the data

	float min, max, dummy = -10000000000.0f;

#ifdef QUANTIZATION
	if (mnVariable != DATA_MIXFRAC) {

		if (mnVariable == DATA_HO2) {
			min = 0;
			//max = 0.000343621;
			max = 0.000316;
		}            
		else if (mnVariable == DATA_OH) {
			min = 0;
			max = 0.0207485;
		}            
		else if (mnVariable == DATA_CHI) {
			//min = 0;
			//max = 12.2035;
			min = -2.0;
			//max = 0.75;
			max = 0.5;
		}
		else if (mnVariable == DATA_TEMP) {
			//min = 20.0f;
			//max = 30.0f;
			min = 100.0f;
			max = -100.0f;
			//min = max = mpVolume[start];
			for (int i = start; i <= end; i++) {
				if (mpVolume[i] != dummy)
				{
					if (min > mpVolume[i])
						min = mpVolume[i];
					if (max < mpVolume[i])
						max = mpVolume[i];
				}
			}
			printf("min = %f, max = %f\n", min, max);
		}
		else if (mnVariable == DATA_SALT) {
			//min = 30.0f;
			//max = 37.0f;  
			min = 100.0f;
			max = -100.0f;
			//min = max = mpVolume[start];
			for (int i = start; i <= end; i++) {
				if (mpVolume[i] != dummy)
				{
					if (min > mpVolume[i])
						min = mpVolume[i];
					if (max < mpVolume[i])
						max = mpVolume[i];
				}
			}
			printf("min = %f, max = %f\n", min, max);
		}
		else if (mnVariable == DATA_AMP) {
			min = 0.0f;
			max = 1.0f;            
		}
		else if (mnVariable == DATA_VORTS) {
			min = 0.002292f;
			max = 12.446266f;
		}
		else if (mnVariable == DATA_QVAPOR) {
			min = 0.0f;
			max = 0.02368f;
		}
		else if (mnVariable == DATA_Y_OH) {
			min = 0.0f;
			max = 0.008756f;
		}
		else {

#ifdef SUPERNOVA
			min = 0.0f;
			max = 204.728 * 1.5;

			//#elif defined (CLIMATE)
			//								min = 20.0f; max = 30.0f; // climate temp
			//min = 30.0f; max = 37.0f; // climate salt

			//#elif defined (EARTHQUAKE)
			//								min = 0.0f;
			//max = 3.029119f; // earthquake amplitude
			//								max = 1.0f;
#elif defined (FIVEJETS)
			min = 237647.640625f;
			//max = 346325.312500f; // fivejets energy 
			max = 300000.0f; 
#else
			min = max = mpVolume[start];

			//for (int i = 1; i < m_nVolumeSize*m_nNumTimePearson; i++) {
			for (int i = start; i <= end; i++) {
				//for (int i = 1; i < m_nVolumeSize; i++) {
				if (min > mpVolume[i])
					min = mpVolume[i];
				if (max < mpVolume[i])
					max = mpVolume[i];
			}
#endif                
			cout << "min " << min << " max " << max << endl;                
		}

		m_fMin = min;
		m_fMax = max;    
		//for (int i = 0; i < m_nVolumeSize*m_nNumTimePearson; i++) {
		for (int i = start; i <= end; i++) {
			//for (int i = 0; i < m_nVolumeSize; i++) {
			if (mnVariable == DATA_CHI) {
				if (mpVolume[i] > 0)
					mpVolume[i] = (log((float)mpVolume[i] / 10400.0f)/ log(10.0f) - min) / (max - min);

				if (mpVolume[i] < 0)
					mpVolume[i] = 0;
				if (mpVolume[i] > 1)
					mpVolume[i] = 1;
			}
			else {
#if defined (CLIMATE) 
				if (mpVolume[i] != dummy)
				{
					float value = (mpVolume[i] - min) / (max - min);
					//if (value>1.0 || value<0.0) printf("value = %f out of range!\n", value);
					// [0, 1] -> [0.1, 1]
					value = 0.9 * value + 0.1;
					mpVolume[i] = value;
				}
				else 
					mpVolume[i] = 0.0;
#else
				float value = (mpVolume[i] - min) / (max - min);
				mpVolume[i] = value;
#endif                                                        
				//*/
			}
		}                        
		if (debug)
		{
			FILE *out;
			out = fopen(fn, "wb");
			for (int i=0; i<m_nVolumeSize; i++)
				fprintf(out, "%f\n", mpVolume[i]);
			fclose(out);
		}
	}

#endif        
}

#endif //PEARSONCPU 

#ifdef POINT_CLOUD
void cxVolume::ReadFlexible(NetCDF* netcdf,		//opened NetCDF file pointer
							int time_in,			//initial timestep 
							int count,			//number of timesteps requested (no less than 1)
							const char* var_name	//requested variable name
							)
{
	int x, y, z, t;
	x = y = z = t = 1;
	float* pdata;
	int start, end;
	FILE *out;			// for debugging
	char fn[512];		// for debugging
	bool debug = false;  // for debugging
	assert(netcdf != NULL);
	assert(m_netcdf != NULL);
	assert(count >= 1);
	int orig_count=count;

	if(debug){
		sprintf(fn,"d:\\jeffs\\corr_%s_debug.txt",var_name);
		out = fopen(fn, "w");
		assert(out);
	}

	if (m_pVolume != NULL){
		delete [] (float*) m_pVolume;
		m_pVolume = NULL;
	}

	ClearVolTex();

	if(!netcdf)
		netcdf = m_netcdf;
	if(m_netcdf != netcdf){
		delete m_netcdf;
		m_netcdf = netcdf;
	}

	NetCFD_var* vinfo = netcdf->get_varinfo(var_name);
	assert(vinfo);
	ReadGrid(vinfo);

	if (vinfo->rh_ndims == 4)
	{
		t = vinfo->dim[0]->length;
		z = vinfo->dim[1]->length;
		y = vinfo->dim[2]->length;
		x = vinfo->dim[3]->length;
	}
	m_nVolumeSize = x * y * z;
	assert(m_nVolumeSize > 0);

	//SetTimeLen(t);
	SetSize(x, y, z);
	SetCenter( x/2, y/2, z/2);

	// allocate memory for NetCDF data for all timesteps and and all selected variables
	m_pVolume = new float[m_nVolumeSize * orig_count*2]; // should be 2 
	assert(m_pVolume);

	pdata = m_pVolume;
	try{

		NetCFD_var* var_info;
		m_nVariable = DATA_CORR;

		// loop through the list of correlated variables 
		//fprintf(out, "test = %d\n",i);
		start = 0; end=m_nVolumeSize;
		//printf("i=%d, start=%d, end=%d\n",i,start,end);
		var_info = netcdf->get_varinfo(var_name);
		assert(var_info);
		assert(var_info->rh_ndims == 4);


		if (strcmp(var_info->var_name,"mixfrac") == 0)
			m_nVariable = DATA_MIXFRAC;

		else if (strcmp(var_info->var_name,"HO2") == 0)
			m_nVariable = DATA_HO2;

		else if (strcmp(var_info->var_name,"OH") == 0)
			m_nVariable = DATA_OH;

		else if (strcmp(var_info->var_name,"chi") == 0)
			m_nVariable = DATA_CHI;

		else if (strcmp(var_info->var_name,"temp") == 0)
			m_nVariable = DATA_TEMP;


		else if (strcmp(var_info->var_name,"salt") == 0)
			m_nVariable = DATA_SALT;
		else if (strcmp(var_info->var_name,"temp_a") == 0)
			m_nVariable = DATA_TEMP_A;

		else if (strcmp(var_info->var_name,"salt_a") == 0)
			m_nVariable = DATA_SALT_A;

		else if (strcmp(var_info->var_name,"wt") == 0)
			m_nVariable = DATA_WT;
		else
			m_nVariable = DATA_UNKNOWN;

		for(int j=0; j< count; j++){
			if ( var_info->rh_type == NC_FLOAT) {
				m_nVolumeType = VOLUME_TYPE_FLOAT;
			} else {
				assert(0); // some other data type
				m_nVolumeType = VOLUME_TYPE_FLOAT;
			}

			if (m_nVolumeType == VOLUME_TYPE_FLOAT) {
				// read data starting at timestep "time_in" and 
				// for the duration of "count" timesteps
				netcdf->get_vardata(const_cast<char *>(var_name), 
					time_in+j, 1, 0, 1, 
					reinterpret_cast<char **>(&pdata), true);
			}

			// process the data
			float min, max, dummy = -10000000000.0f;
#ifdef QUANTIZATION

			if (m_nVariable == DATA_HO2) {
				min = 0;
				//max = 0.000343621;
				max = 0.000316;
			}            
			else if (m_nVariable == DATA_OH) {
				min = 0;
				max = 0.0207485;
			}            
			else if (m_nVariable == DATA_CHI) {
				//min = 0;
				//max = 12.2035;
				min = -2.0;
				//max = 0.75;
				max = 0.5;
			}
			else if (m_nVariable == DATA_TEMP) {
				min = 20.0f;
				max = 30.0f;
				//min = 100.0f;
				//max = -100.0f;
				//printf("TEMP i=%d, start=%d, end=%d\n",i,start,end);
				for (int k = start; k < end; k++) {
					//assert(pdata[k]);
					if ( min > pdata[k] )
						pdata[k] = min;
					if ( max < pdata[k])
						pdata[k] = max;
				}
				printf("min = %f, max = %f\n", min, max);
			}
			else if (m_nVariable == DATA_SALT) {
				min = 30.0f;
				max = 37.0f;  
				//min = 100.0f;
				//max = -100.0f;
				//min = max = pdata[start];
				//printf("SALT i=%d, start=%d, end=%d\n",i,start,end);
				for (int k = start; k < end; k++) {
					//assert(pdata[k]);
					if ( min > pdata[k] )
						pdata[k] = min;
					if ( max < pdata[k])
						pdata[k] = max;
				}

				//printf("min = %f, max = %f\n", min, max);
			}
			else {

				min = max = pdata[start];

				//for (int i = 1; i < m_nVolumeSize*m_nNumTimePearson; i++) {
				for (int k = start; k < end; k++) {
					//for (int i = 1; i < m_nVolumeSize; i++) {
					if (min > pdata[k])
						min = pdata[k];
					if (max < pdata[k])
						max = pdata[k];
				}
				cout << "min " << min << " max " << max << endl;                
			}

			m_fMin = min;
			m_fMax = max;    
			//for (int i = 0; i < m_nVolumeSize*m_nNumTimePearson; i++) {
			for (int k = start; k < end; k++) {
				//for (int i = 0; i < m_nVolumeSize; i++) {
				if (m_nVariable == DATA_CHI) {
					if (pdata[k] > 0)
						pdata[k] = (log((float)pdata[k] / 10400.0f)/ log(10.0f) - min) / (max - min);

					if (pdata[k] < 0)
						pdata[k] = 0;
					if (pdata[k] > 1)
						pdata[k] = 1;
				}
				else {
					//#if defined (CLIMATE) 
					/*
					if (pdata[i] != dummy)
					{
					float value = (pdata[i] - min) / (max - min);
					//if (value>1.0 || value<0.0) printf("value = %f out of range!\n", value);
					// [0, 1] -> [0.1, 1]
					value = 0.9 * value + 0.1;
					pdata[i] = value;
					}
					else 
					pdata[i] = 0.0;
					*/
					//#else
					//						float value = (pdata[i] - min) / (max - min);
					//						pdata[i] = value;
					//#endif                                                        
				}
			}                        

			m_fMin = min;
			m_fMax = max;

			for (int k = 0; k < m_nVolumeSize; k++) {
				if ( m_nVariable == DATA_CHI) {
					if (pdata[k] > 0)
						pdata[k] = ((float)log((float)m_pVolume[k] / 10400.0)/ log((float)10) - min) / (max - min);

					if (pdata[k] < 0)
						pdata[k] = 0;
					if (pdata[k] > 1)
						pdata[k] = 1;
				}else {
					float value = (pdata[k] - min) / (max - min);

#if defined(CLIMATE) 
					if (pdata[k] < -100)
						pdata[k] = 0;
					else
						pdata[k] = (value < 0) ? 0.1 : value;
#else
					pdata[k] = value;
#endif    
				}
			}



#if defined(CLIMATE)            


			// reverse the z direction  
			for (int z = 0; z < m_vSize[2] / 2; z++){
				for (int y = 0; y < m_vSize[1]; y++) { 
					for (int x = 0; x < m_vSize[0]; x++) {
						int org_id = x + y * m_vSize[0] + z * m_vSize[0] * m_vSize[1];
						int new_id = x + y * m_vSize[0] + (m_vSize[2] - 1 - z) * m_vSize[0] * m_vSize[1];
						float temp = pdata[new_id];
						pdata[new_id] = pdata[org_id];            
						pdata[org_id] = temp;
					}
				}
			}
#endif            

			if(debug)
			{
				assert(out);
				fprintf(out, "____________timestep %d__________\n",j);
				for (int k=0; k<m_nVolumeSize; k++){
					if(k<20 || k > m_nVolumeSize-20){
						fprintf(out, "%f\n", pdata[k]);
					}
				}


			}

#endif			


			pdata += m_nVolumeSize; 


		} //for timesteps

		if(strcmp(var_name,"temp")==0)
			m_nVariable = DATA_TEMP_TEMP;
		else if(strcmp(var_name,"salt")==0)
			m_nVariable = DATA_SALT_SALT;
		else if(strcmp(var_name,"wt")==0)
			m_nVariable = DATA_WT_WT;


		if(debug && out)
			fclose(out);

	}catch(...)
	{
		m_nVolumeSize = 0;
		if(m_pVolume)
			delete [] m_pVolume;
		if(debug && out)
			fclose(out);
	}
}
#endif

#define MAX_NUM_CORR_VAR	2  //maximum number of correlated variables
void cxVolume::ReadNetCDF2(NetCDF* netcdf,		//opened NetCDF file pointer
						   int time_in,			//initial timestep 
						   int count,			//number of timesteps requested (no less than 1)
						   string var_names[],  //requested variable names
						   int num_vars			//number of variables requested (no more than 2)
						   )
{
	int x, y, z, t;
	x = y = z = t = 1;
	float* pdata;
	int start, end;
	FILE *out;			// for debugging
	char fn[512];		// for debugging
	bool debug = false;  // for debugging
	assert(netcdf != NULL);
	assert(m_netcdf != NULL);
	assert(count >= 1);
	assert(num_vars <= 2 && num_vars >= 1);
	int orig_count=count;

	float arr_fMin[2];
	float arr_fMax[2];
	arr_fMin[0] = m_fMin1;
	arr_fMin[1] = m_fMin2;
	arr_fMax[0] = m_fMax1;
	arr_fMax[1] = m_fMax2;

	if(num_vars > 1)
		count = count/2;

	if(debug){
		if(num_vars == 1)
			sprintf(fn,"d:\\jeffs\\corr_%s_%s_debug.txt",var_names[0].c_str(),var_names[0].c_str());
		else
			sprintf(fn,"d:\\jeffs\\corr_%s_%s_debug.txt",var_names[0].c_str(),var_names[1].c_str());
		out = fopen(fn, "w");
		assert(out);
	}

	if (m_pVolume != NULL){
		delete [] (float*) m_pVolume;
		m_pVolume = NULL;
	}

	ClearVolTex();

	if(!netcdf)
		netcdf = m_netcdf;
	if(m_netcdf != netcdf){
		delete m_netcdf;
		m_netcdf = netcdf;
	}

	NetCFD_var* vinfo = netcdf->get_varinfo(var_names[0].c_str());
	assert(vinfo);
	ReadGrid(vinfo);

	if (vinfo->rh_ndims == 4)
	{
		t = vinfo->dim[0]->length;
		z = vinfo->dim[1]->length;
		y = vinfo->dim[2]->length;
		x = vinfo->dim[3]->length;
	}
	m_nVolumeSize = x * y * z;
	assert(m_nVolumeSize > 0);

	//SetTimeLen(t);
	SetSize(x, y, z);
	SetCenter( x/2, y/2, z/2);

	// allocate memory for NetCDF data for all timesteps and and all selected variables
	m_pVolume = new float[m_nVolumeSize * orig_count*2]; // should be 2 
	assert(m_pVolume);

	pdata = m_pVolume;
	try{

		NetCFD_var* var_info[MAX_NUM_CORR_VAR];
		m_nVariable = DATA_CORR;

		// loop through the list of correlated variables 
		for(int i=0; i < num_vars; i++){
			//fprintf(out, "test = %d\n",i);
			start = i*m_nVolumeSize; end=(i+1)*m_nVolumeSize;
			//printf("i=%d, start=%d, end=%d\n",i,start,end);
			var_info[i] = netcdf->get_varinfo(var_names[i].c_str());
			assert(var_info[i]);
			assert(var_info[i]->rh_ndims == 4);


			if (strcmp(var_info[i]->var_name,"mixfrac") == 0)
				m_nVariable = DATA_MIXFRAC;

			else if (strcmp(var_info[i]->var_name,"HO2") == 0)
				m_nVariable = DATA_HO2;

			else if (strcmp(var_info[i]->var_name,"OH") == 0)
				m_nVariable = DATA_OH;

			else if (strcmp(var_info[i]->var_name,"chi") == 0)
				m_nVariable = DATA_CHI;

			else if (strcmp(var_info[i]->var_name,"temp") == 0)
				m_nVariable = DATA_TEMP;


			else if (strcmp(var_info[i]->var_name,"salt") == 0)
				m_nVariable = DATA_SALT;
			else if (strcmp(var_info[i]->var_name,"temp_a") == 0)
				m_nVariable = DATA_TEMP_A;

			else if (strcmp(var_info[i]->var_name,"salt_a") == 0)
				m_nVariable = DATA_SALT_A;

			else if (strcmp(var_info[i]->var_name,"wt") == 0)
				m_nVariable = DATA_WT;
			else
				m_nVariable = DATA_UNKNOWN;

			for(int j=0; j< count; j++){
				if ( var_info[i]->rh_type == NC_FLOAT) {
					m_nVolumeType = VOLUME_TYPE_FLOAT;
				} else {
					assert(0); // some other data type
					m_nVolumeType = VOLUME_TYPE_FLOAT;
				}

				if (m_nVolumeType == VOLUME_TYPE_FLOAT) {
					// read data starting at timestep "time_in" and 
					// for the duration of "count" timesteps
					netcdf->get_vardata(const_cast<char *>(var_names[i].c_str()), 
						time_in+j, 1, 0, 1, 
						reinterpret_cast<char **>(&pdata), true);
				}

				// process the data
				float min, max, dummy = -10000000000.0f;
#ifdef QUANTIZATION

				if (m_nVariable == DATA_HO2) {
					min = 0;
					//max = 0.000343621;
					max = 0.000316;
				}            
				else if (m_nVariable == DATA_OH) {
					min = 0;
					max = 0.0207485;
				}            
				else if (m_nVariable == DATA_CHI) {
					//min = 0;
					//max = 12.2035;
					min = -2.0;
					//max = 0.75;
					max = 0.5;
				}
				else if (m_nVariable == DATA_TEMP) {
					min = 10.0f;
					max = 30.0f;
					//min = 100.0f;
					//max = -100.0f;
					//printf("TEMP i=%d, start=%d, end=%d\n",i,start,end);
					
				}
				else if (m_nVariable == DATA_SALT) {
					min = 30.0f;
					max = 37.0f;  
					//min = 100.0f;
					//max = -100.0f;
					//min = max = pdata[start];
					//printf("SALT i=%d, start=%d, end=%d\n",i,start,end);
					

					//printf("min = %f, max = %f\n", min, max);
				}
				/*
				else if (m_nVariable == DATA_AMP) {
				min = 0.0f;
				max = 1.0f;            
				}
				else if (m_nVariable == DATA_VORTS) {
				min = 0.002292f;
				max = 12.446266f;
				}
				else if (m_nVariable == DATA_QVAPOR) {
				min = 0.0f;
				max = 0.02368f;
				}
				else if (m_nVariable == DATA_Y_OH) {
				min = 0.0f;
				max = 0.008756f;
				}
				*/
				else {
					if(arr_fMin[i] < arr_fMax[i]){
						min = arr_fMin[i];
						max = arr_fMax[i];
					}
					else{
						min = max = pdata[start];

						//for (int i = 1; i < m_nVolumeSize*m_nNumTimePearson; i++) {
						for (int k = start; k < end; k++) {
							//for (int i = 1; i < m_nVolumeSize; i++) {
							if (min > pdata[k])
								min = pdata[k];
							if (max < pdata[k])
								max = pdata[k];
						}
						cout << "min " << min << " max " << max << endl; 
						arr_fMin[i] = min;
						arr_fMax[i] = max;
					}
				}

				if(arr_fMin[i] < arr_fMax[i]){
					min = arr_fMin[i];
					max = arr_fMax[i];
				}
				else{
					arr_fMin[i] = min;
					arr_fMax[i] = max; 
				}

				for (int k = start; k < end; k++) {
						//assert(pdata[k]);
					if ( min > pdata[k] )
						pdata[k] = min;
					if ( max < pdata[k])
						pdata[k] = max;
				}

   
				//for (int i = 0; i < m_nVolumeSize*m_nNumTimePearson; i++) {
				for (int k = start; k < end; k++) {
					//for (int i = 0; i < m_nVolumeSize; i++) {
					if (m_nVariable == DATA_CHI) {
						if (pdata[k] > 0)
							pdata[k] = (log((float)pdata[k] / 10400.0f)/ log(10.0f) - min) / (max - min);

						if (pdata[k] < 0)
							pdata[k] = 0;
						if (pdata[k] > 1)
							pdata[k] = 1;
					}
					else {
						//#if defined (CLIMATE) 
						/*
						if (pdata[i] != dummy)
						{
						float value = (pdata[i] - min) / (max - min);
						//if (value>1.0 || value<0.0) printf("value = %f out of range!\n", value);
						// [0, 1] -> [0.1, 1]
						value = 0.9 * value + 0.1;
						pdata[i] = value;
						}
						else 
						pdata[i] = 0.0;
						*/
						//#else
						//						float value = (pdata[i] - min) / (max - min);
						//						pdata[i] = value;
						//#endif                                                        
					}
				}                        

				for (int k = 0; k < m_nVolumeSize; k++) {
					if ( m_nVariable == DATA_CHI) {
						if (pdata[k] > 0)
							pdata[k] = ((float)log((float)m_pVolume[k] / 10400.0)/ log((float)10) - min) / (max - min);

						if (pdata[k] < 0)
							pdata[k] = 0;
						if (pdata[k] > 1)
							pdata[k] = 1;
					}else {
						float value = (pdata[k] - min) / (max - min);

#if defined(CLIMATE) 
						if (pdata[k] < -100)
							pdata[k] = 0;
						else
							pdata[k] = (value < 0) ? 0.1 : value;
#else
						pdata[k] = value;
#endif    
					}
				}



#if defined(CLIMATE)            


				// reverse the z direction  
				for (int z = 0; z < m_vSize[2] / 2; z++){
					for (int y = 0; y < m_vSize[1]; y++) { 
						for (int x = 0; x < m_vSize[0]; x++) {
							int org_id = x + y * m_vSize[0] + z * m_vSize[0] * m_vSize[1];
							int new_id = x + y * m_vSize[0] + (m_vSize[2] - 1 - z) * m_vSize[0] * m_vSize[1];
							float temp = pdata[new_id];
							pdata[new_id] = pdata[org_id];            
							pdata[org_id] = temp;
						}
					}
				}
#endif            

				if(debug)
				{
					assert(out);
					fprintf(out, "____________timestep %d__________\n",j);
					for (int k=0; k<m_nVolumeSize; k++){
						if(k<20 || k > m_nVolumeSize-20){
							fprintf(out, "%f\n", pdata[k]);
						}
					}


				}

#endif			


				pdata += m_nVolumeSize; 


			} //for timesteps
		} //for variables
		// case of only one variable num_vars == 1

		/*
		if(num_vars == 1){
		int i = 0;
		while(i < m_nVolumeSize){
		m_pVolume[i+m_nVolumeSize] = m_pVolume[i];
		i++;
		}
		}
		*/

		if(num_vars == 1){
			if(strcmp(var_names[0].c_str(),"temp")==0)
				m_nVariable = DATA_TEMP_TEMP;
			else if(strcmp(var_names[0].c_str(),"salt")==0)
				m_nVariable = DATA_SALT_SALT;
			else if(strcmp(var_names[0].c_str(),"wt")==0)
				m_nVariable = DATA_WT_WT;
		}
		else if(num_vars == 2){
			if((strcmp(var_names[0].c_str(),"temp")==0 && strcmp(var_names[1].c_str(),"salt")==0)||
				(strcmp(var_names[0].c_str(),"salt")==0 && strcmp(var_names[1].c_str(),"temp")==0))
				m_nVariable = DATA_SALT_TEMP;
			else if((strcmp(var_names[0].c_str(),"temp")==0 && strcmp(var_names[1].c_str(),"wt")==0)||
				(strcmp(var_names[0].c_str(),"wt")==0 && strcmp(var_names[1].c_str(),"temp")==0))
				m_nVariable = DATA_WT_TEMP;
			else if((strcmp(var_names[0].c_str(),"salt")==0 && strcmp(var_names[1].c_str(),"wt")==0)||
				(strcmp(var_names[0].c_str(),"wt")==0 && strcmp(var_names[1].c_str(),"salt")==0))
				m_nVariable = DATA_WT_SALT;
		}

		if(debug && out)
			fclose(out);

	}catch(...)
	{
		m_nVolumeSize = 0;
		if(m_pVolume)
			delete [] m_pVolume;
		if(debug && out)
			fclose(out);
	}

}



void cxVolume::ReadNetCDF(NetCDF* netcdf, const char* index, int time_in, int time_out)
{
	// read header info
	if(!netcdf && !m_netcdf)
		return;
	ClearVolume();
	if(!netcdf && m_netcdf)
		netcdf = m_netcdf;
	if(m_netcdf != netcdf){
		delete m_netcdf;
		m_netcdf = netcdf;
	}
	try{

		strcpy(m_curnetcdf_var,index);

		int x, y, z, t;
		x = y = z = t = 1;
		string type;

		NetCFD_var* var_info = netcdf->get_varinfo(m_curnetcdf_var);
		ReadGrid(var_info);

		if (strcmp(var_info->var_name,"mixfrac") == 0)
			m_nVariable = DATA_MIXFRAC;

		else if (strcmp(var_info->var_name,"HO2") == 0)
			m_nVariable = DATA_HO2;

		else if (strcmp(var_info->var_name,"OH") == 0)
			m_nVariable = DATA_OH;

		else if (strcmp(var_info->var_name,"chi") == 0)
			m_nVariable = DATA_CHI;

		else if (strcmp(var_info->var_name,"temp") == 0)
			m_nVariable = DATA_TEMP;


		else if (strcmp(var_info->var_name,"salt") == 0)
			m_nVariable = DATA_SALT;
		else if (strcmp(var_info->var_name,"temp_a") == 0)
			m_nVariable = DATA_TEMP_A;

		else if (strcmp(var_info->var_name,"salt_a") == 0)
			m_nVariable = DATA_SALT_A;

		else if (strcmp(var_info->var_name,"wt") == 0)
			m_nVariable = DATA_WT;
		else
			m_nVariable = DATA_UNKNOWN;
		if (var_info->rh_ndims == 4)
		{
			t = var_info->dim[0]->length;
			z = var_info->dim[1]->length;
			y = var_info->dim[2]->length;
			x = var_info->dim[3]->length;
		}

		if ( var_info->rh_type == NC_FLOAT) {
			m_nVolumeType = VOLUME_TYPE_FLOAT;
		} else if (var_info->rh_type == NC_SHORT) {
			m_nVolumeType = VOLUME_TYPE_SHORT;       
		} else if (var_info->rh_type == NC_BYTE) {
			m_nVolumeType = VOLUME_TYPE_BYTE;       
		} else {
			//assert(0); // unknow data type
			m_nVolumeType = VOLUME_TYPE_FLOAT;
		}

		m_nVolumeSize = x * y * z;

		SetTimeLen(t);
		SetSize(x, y, z);
		SetCenter( x/2, y/2, z/2);

		//TODO: needs to be re-written and moved from here
		// maybe it needs be merged with this function (ReadNetCDF)...
#ifdef PEARSONCPU
		// read pearson correllation data:
		char pearson_data_file[256];
		sprintf(pearson_data_file, "%s", m_sFilename.c_str());
		m_pVolume = new float[m_nVolumeSize*m_nNumTimePearson];
		int start, end;
		if (!m_bPearsonVariable) {
			start = 0;
			end = (m_nVolumeSize*m_nNumTimePearson)-1;
			ReadDataPearson(m_pVolume, pearson_data_file, m_nVariable, m_nTexVolPearson, false, "test.txt", start, end);
		}
		else {
			start = 0;
			end = (m_nVolumeSize*m_nNumTimePearson/2)-1;
			ReadDataPearson(m_pVolume, pearson_data_file, m_nVariable, m_nTexVolPearson, false, "test.txt", start, end);
			ReadDataPearson(m_pVolume+(m_nVolumeSize*m_nNumTimePearson/2),
				"C:\\jeffs\\uc_davis\\src\\chaoli\\climate\\salt\\zorder\\salt_%s", DATA_SALT, m_nTexVolPearson,
				false, "salt.txt", start, end);
		}
#endif 

		// read data
		if ( m_nTexVol!= 0)  
			glDeleteTextures(1, (const GLuint*)&m_nTexVol);

		if ( m_nIndTexVol!= 0)
			glDeleteTextures(1, (const GLuint*)&m_nIndTexVol);

#ifdef SHOW_CLUSTER
		if ( m_nTexClus!= 0){
			glDeleteTextures(1, (const GLuint*)&m_nTexClus);
		}
#endif

		m_pVolume = new float[m_nVolumeSize];
		m_pIndVolume = new float[m_nVolumeSize];
		if( m_pVolume == NULL || m_pIndVolume == NULL) {
			cerr << "Memory leak" << endl;
			return;
		}

		if (m_nVolumeType == VOLUME_TYPE_FLOAT) {

			netcdf->get_vardata(const_cast<char *>(m_curnetcdf_var), 
				m_time_in, m_time_out, 0, 1, 
				reinterpret_cast<char **>(&m_pVolume), true);
			/*
			FILE *out;
			out = fopen("test1.bin", "wb");
			fwrite(m_pVolume, sizeof(float), m_vSize[0]*m_vSize[1]*m_vSize[2], out);

			fclose(out);  
			*/
			float min, max;

#ifdef QUANTIZATION
			if (m_nVariable != DATA_MIXFRAC ) {

				if (m_nVariable == DATA_HO2) {
					min = 0;
					//max = 0.000343621;
					max = 0.000316;
				}            
				else if (m_nVariable == DATA_OH) {
					min = 0;
					max = 0.0207485;
				}            
				else if (m_nVariable == DATA_CHI) {
					//min = 0;
					//max = 12.2035;
					min = -2.0;
					//max = 0.75;
					max = 0.5;
				}
				else if (m_nVariable == DATA_TEMP) {
					min = 20.0f;
					max = 30.0f;
					//min = max = m_pVolume[0];

					 
				}
				else if (m_nVariable == DATA_SALT) {
					min = 30.0f;
					max = 37.0f;
					//min = max = m_pVolume[0];

					
				}
				else if (m_nVariable == DATA_SALT_A) {
					min = 0.0f;
					max = 1.9649162f;
					//min = max = m_pVolume[0];

					
				}

				else if (m_nVariable == DATA_TEMP_A) {
					min = 0.00f;
					max = 3.69f;
					//min = max = m_pVolume[0];

					
				}
				else if (m_nVariable == DATA_WT) {
					//min = -0.00001f;
					//max = 0.00001f;    
					min = -0.00000001f;
					max = 0.00006;

					//min = max = m_pVolume[0];

					        
				}
				else { 
					if(m_fMin1 < m_fMax1){
						min = m_fMin1;
						max = m_fMax1;
					}
					else{
#ifdef SUPERNOVA
						min = 0.0f;
						max = 204.728 * 1.5;
#else
						min = max = m_pVolume[0];

						for (int i = 1; i < m_nVolumeSize; i++) {
							if ( min > m_pVolume[i] )
								min = m_pVolume[i];
							if ( max < m_pVolume[i])
								max = m_pVolume[i];
						}

						cout << "min " << min << " max " << max << endl;
#endif                
						m_fMin1 = min;
						m_fMax1 = max;
					}
				}

				if(m_fMin1 == m_fMax1){
					m_fMin1 = min;
					m_fMax1 = max;
				}
				else{
					min = m_fMin1;
					max = m_fMax1;
				}

				for (int i = 1; i < m_nVolumeSize; i++) {
						if ( m_fMin1 > m_pVolume[i] )
							m_pVolume[i] = m_fMin1;
						if ( m_fMax1 < m_pVolume[i])
							m_pVolume[i] = m_fMax1;
				}
				

				for (int i = 0; i < m_nVolumeSize; i++) {
					if ( m_nVariable == DATA_CHI) {
						if (m_pVolume[i] > 0)
							m_pVolume[i] = ((float)log((float)m_pVolume[i] / 10400.0)/ log((float)10) - min) / (max - min);

						if (m_pVolume[i] < 0)
							m_pVolume[i] = 0;
						if (m_pVolume[i] > 1)
							m_pVolume[i] = 1;
					}else {
						float value = (m_pVolume[i] - min) / (max - min);

#if defined(CLIMATE) 
						//m_fMin = 0.0;
						//m_fMax = 1.0;
						if (m_pVolume[i] < -100) // should this be user defined
							m_pVolume[i] = min;//0;
						else
							m_pVolume[i] = value;
						//	m_pVolume[i] = (value < 0) ? 0.1 : value;
#else
						m_pVolume[i] = value;
#endif    
					}
				}


				if (m_stopIndX == m_startIndX &&  m_startIndX == 0){
					m_stopIndX = m_vSize[0];
					m_startIndX = 0;
				} 

				if (m_stopIndY == m_startIndY &&  m_startIndY == 0){
					m_stopIndY = m_vSize[1];
					m_startIndY = 0;
				} 
#if defined(CLIMATE)            


				// reverse the z direction  
				for (int z = 0; z < m_vSize[2] / 2; z++)
					for (int y = 0; y < m_vSize[1]; y++) 
						for (int x = 0; x < m_vSize[0]; x++) {
							int org_id = x + y * m_vSize[0] + z * m_vSize[0] * m_vSize[1];
							int new_id = x + y * m_vSize[0] + (m_vSize[2] - 1 - z) * m_vSize[0] * m_vSize[1];
							float temp = m_pVolume[new_id];
							m_pVolume[new_id] = m_pVolume[org_id];            
							m_pVolume[org_id] = temp;

							if (	(x <= (m_stopIndX *m_vSize[0] /100) && x >= (m_startIndX *m_vSize[0] /100)) && 
								(y <= (m_stopIndY *m_vSize[1] /100) && y >= (m_startIndY*m_vSize[1] /100)))
								m_pIndVolume[new_id] = m_pVolume[new_id];
							else 
								m_pIndVolume[new_id] = 0; //30

						}

#endif            

			}

#endif

		} else if (m_nVolumeType == VOLUME_TYPE_SHORT) {

			unsigned short * pTempVolume = new unsigned short[m_nVolumeSize];
			assert(pTempVolume);
			netcdf->get_vardata(const_cast<char *>(m_curnetcdf_var), m_time_in, m_time_out, 0,1, reinterpret_cast<char **>(&pTempVolume), true);

			float max = pTempVolume[0];
			float min = pTempVolume[0];

			//find min and max
			for (int i = 0; i < m_nVolumeSize; i++) {
				if ( max < pTempVolume[i])
					max = pTempVolume[i];
				if ( min > pTempVolume[i])
					min = pTempVolume[i];
			}

			max *= 0.01;
			cout << "Max " << max << endl;
			cout << "Min " << min << endl;

			for (int i = 0; i < m_nVolumeSize; i++) 
				m_pVolume[i] = ((float) pTempVolume[i] - min) / (max - min);           

			//
			//for (int i = 0; i < m_nVolumeSize; i++) 
			//    m_pVolume[i] = ((float) pTempVolume[i] ) / 0xffff;            
			//

			delete [] pTempVolume;

		} else if (m_nVolumeType == VOLUME_TYPE_BYTE) {

			unsigned char * pTempVolume = new unsigned char[m_nVolumeSize];
			assert(pTempVolume);
			netcdf->get_vardata(const_cast<char *>(m_curnetcdf_var), m_time_in, m_time_out, 0,1,reinterpret_cast<char **>(&pTempVolume), true);

			for (int i = 0; i < m_nVolumeSize; i++) {
				m_pVolume[i] = ((float)pTempVolume[i]) / 0xff;
			}

			delete [] pTempVolume;
		}

#ifdef HISTOGRAM
		int wid = m_pHistogram->w();
		int hei = m_pHistogram->h();

		int * count = new int[wid];
		assert(count);

		memset(count, 0, sizeof(int) * wid);

		int max = 0;
		for (int i = 0; i < m_nVolumeSize; i++) {
			float value = m_pVolume[i];

			int id = int(value * float(wid - 1));
			if (id > wid - 1)
				id = wid - 1;

			count[id]++;

			if (count[id] > max)
				max = count[id];

		}

		int sum = 0;
		for ( int i = 0; i < wid; i++)
			sum += count[i];

		int avg = sum / wid;

		uchar * histogram = m_pHistogram->m_pImage;
		memset(histogram, 0, wid * hei * 3);

		for (int x = 0; x < wid; x++) {
			int max_id = int(float(count[x]) / float(avg) * float(hei)); 
			if (max_id > hei - 1)
				max_id = hei - 1;

			for (int y = hei - 1; y > hei -1 - max_id; y--) {
				int id = x + y * wid;
				histogram[id * 3 + 0] = 255;
				histogram[id * 3 + 1] = 255;
				histogram[id * 3 + 2] = 255;
			}
		}    
		delete count; 
#endif

		char fn[512];
		sprintf(fn,"d:\\jeffs\\1var_debug_%s.txt",var_info->var_name);
		bool debug = false;
		if (debug)
		{
			FILE *out;
			out = fopen(fn, "w");
			for (int i=0; i<m_nVolumeSize; i++)
				if(i<20 || i > m_nVolumeSize-20)
					fprintf(out, "%f\n", m_pVolume[i]);
			fclose(out);
		}
	}
	catch(...)
	{
		m_nVolumeSize = 0;
	}
}

#ifdef OLD_CODE
void cxVolume::ReadFile()
{
	char head_file[1024];
	char data_file[1024];
	char temp[1024];

	if (m_nStartTime == -1){
		sprintf(head_file, "%s.hdr", m_sFilename.c_str());
		sprintf(data_file, "%s.dat", m_sFilename.c_str());
	}
	else {
		cout << "Load timestep " << m_nCurTime << endl;

		ostringstream timestep;
		timestep << setw(4) << setfill('0') << m_nCurTime;

		sprintf(temp, "%s.hdr", m_sFilename.c_str());
		sprintf(head_file, temp, timestep.str().c_str(), timestep.str().c_str());

		sprintf(temp, "%s.dat", m_sFilename.c_str());
		sprintf(data_file, temp, timestep.str().c_str(), timestep.str().c_str());

	}

	if (m_sFilename.find("mixfrac") != string::npos)
		m_nVariable = DATA_MIXFRAC;

	if (m_sFilename.find("HO2") != string::npos)
		m_nVariable = DATA_HO2;

	if (m_sFilename.find("OH") != string::npos)
		m_nVariable = DATA_OH;

	if (m_sFilename.find("chi") != string::npos)
		m_nVariable = DATA_CHI;

	if (m_sFilename.find("temp") != string::npos)
		m_nVariable = DATA_TEMP;

	if (m_sFilename.find("salt") != string::npos)
		m_nVariable = DATA_SALT;     
	if (m_sFilename.find("salt_a") != string::npos)
		m_nVariable = DATA_SALT_A;  

	ReadHeader(head_file);       
	ReadData(data_file);    
}

void cxVolume::ReadHeader(char * filename)
{
	int x, y, z;
	string type;

#ifdef READ_HEADFILE    
	ifstream headfile(filename); 
	if (headfile == NULL) {
		cerr << "Can not open " << filename << endl;
		return;
	}          

	headfile >> x >> y >> z;
	headfile >> type;
	headfile.close();    

	if (type == "FLOAT" || type == "Float" || type == "float") {
		m_nVolumeType = VOLUME_TYPE_FLOAT;
	} else if (type == "SHORT" || type == "Short" || type == "short") {
		m_nVolumeType = VOLUME_TYPE_SHORT;        
	} else if (type == "BYTE" || type == "Byte" || type == "byte") {
		m_nVolumeType = VOLUME_TYPE_BYTE;        
	} else {
		//assert(0); // unknow data type
		m_nVolumeType = VOLUME_TYPE_FLOAT;
	}
#else
	x = 800;
	y = 686;
	z = 215;    
	m_nVolumeType = VOLUME_TYPE_FLOAT;
#endif

	m_nVolumeSize = x * y * z;

	SetSize(x, y, z);
	SetCenter( x/2, y/2, z/2);
}


void cxVolume::ReadData(char * filename)
{
	ifstream datafile(filename, ios_base::binary);

	if (datafile == NULL) {
		cerr << "Can not open " << filename << endl;
		return;
	}    

	if ( m_nTexVol!= 0)  
		glDeleteTextures(1, (const GLuint*)&m_nTexVol);

	if ( m_nIndTexVol!= 0)  
		glDeleteTextures(1, (const GLuint*)&m_nIndTexVol);

#ifdef SHOW_CLUSTER
	if ( m_nTexClus!= 0){
		glDeleteTextures(1, (const GLuint*)&m_nTexClus);
	}
#endif

#ifdef SV_CORRELATION
	if (m_nTexCorr != 0) {
		glDeleteTextures(1, (const GLuint*)&m_nTexCorr);
		m_nTexCorr = 0;
	}
#endif

	m_pVolume = new float[m_nVolumeSize];
	if( m_pVolume == NULL) {
		cerr << "Memory leak" << endl;
		return;
	}

	if (m_nVolumeType == VOLUME_TYPE_FLOAT) {

		datafile.read(reinterpret_cast<char *>(m_pVolume), sizeof(float) * m_nVolumeSize);

		float min, max;

#ifdef QUANTIZATION
		if (m_nVariable != DATA_MIXFRAC ) {

			if (m_nVariable == DATA_HO2) {
				min = 0;
				//max = 0.000343621;
				max = 0.000316;
			}            
			else if (m_nVariable == DATA_OH) {
				min = 0;
				max = 0.0207485;
			}            
			else if (m_nVariable == DATA_CHI) {
				//min = 0;
				//max = 12.2035;
				min = -2.0;
				//max = 0.75;
				max = 0.5;
			}
			else if (m_nVariable == DATA_TEMP) {
				min = 20.0f;
				max = 30.0f;            
			}
			else if (m_nVariable == DATA_SALT) {
				min = 30.0f;
				max = 37.0f;            
			}
			else if (m_nVariable == DATA_WT) {
				min = 30.0f;
				max = 37.0f;            
			}
			else { 

#ifdef SUPERNOVA
				min = 0.0f;
				max = 204.728 * 1.5;
#else
				min = max = m_pVolume[0];

				for (int i = 1; i < m_nVolumeSize; i++) {
					if ( min > m_pVolume[i] )
						min = m_pVolume[i];
					if ( max < m_pVolume[i])
						max = m_pVolume[i];
				}

				cout << "min " << min << " max " << max << endl;
#endif                

			}


			for (int i = 0; i < m_nVolumeSize; i++) {
				if ( m_nVariable == DATA_CHI) {
					if (m_pVolume[i] > 0)
						m_pVolume[i] = ((float)log((float)m_pVolume[i] / 10400.0)/ log((float)10) - min) / (max - min);

					if (m_pVolume[i] < 0)
						m_pVolume[i] = 0;
					if (m_pVolume[i] > 1)
						m_pVolume[i] = 1;
				}else {


					float value = (m_pVolume[i] - min) / (max - min);
					m_fMin = 0.0;
					m_fMax = 1.0;
#if defined(CLIMATE) 

					if (m_pVolume[i] < -100)
						m_pVolume[i] = 0;
					else
						m_pVolume[i] = (value < 0) ? 0.1 : value;
#else
					m_pVolume[i] = value;
#endif                                    

				}
			}

#if defined(CLIMATE)            

			/* reverse the z direction */ 
			for (int z = 0; z < m_vSize[2] / 2; z++)
				for (int y = 0; y < m_vSize[1]; y++) 
					for (int x = 0; x < m_vSize[0]; x++) {
						int org_id = x + y * m_vSize[0] + z * m_vSize[0] * m_vSize[1];
						int new_id = x + y * m_vSize[0] + (m_vSize[2] - 1 - z) * m_vSize[0] * m_vSize[1];
						float temp = m_pVolume[new_id];
						m_pVolume[new_id] = m_pVolume[org_id];            
						m_pVolume[org_id] = temp;
					}

#endif            

		}

#endif        
	} else if (m_nVolumeType == VOLUME_TYPE_SHORT) {

		unsigned short * pTempVolume = new unsigned short[m_nVolumeSize];
		assert(pTempVolume);
		datafile.read(reinterpret_cast<char *>(pTempVolume), sizeof(unsigned short) * m_nVolumeSize);

		float max = pTempVolume[0];
		float min = pTempVolume[0];

		//find min and max
		for (int i = 0; i < m_nVolumeSize; i++) {
			if ( max < pTempVolume[i])
				max = pTempVolume[i];
			if ( min > pTempVolume[i])
				min = pTempVolume[i];
		}

		max *= 0.01;
		cout << "Max " << max << endl;
		cout << "Min " << min << endl;

		for (int i = 0; i < m_nVolumeSize; i++) 
			m_pVolume[i] = ((float) pTempVolume[i] - min) / (max - min);           

		/*
		for (int i = 0; i < m_nVolumeSize; i++) 
		m_pVolume[i] = ((float) pTempVolume[i] ) / 0xffff;            
		*/

		delete [] pTempVolume;

	} else if (m_nVolumeType == VOLUME_TYPE_BYTE) {

		unsigned char * pTempVolume = new unsigned char[m_nVolumeSize];
		assert(pTempVolume);
		datafile.read(reinterpret_cast<char *>(pTempVolume), sizeof(unsigned char) * m_nVolumeSize);

		for (int i = 0; i < m_nVolumeSize; i++) {
			m_pVolume[i] = ((float)pTempVolume[i]) / 0xff;
		}

		delete [] pTempVolume;
	}

#ifdef HISTOGRAM
	int wid = m_pHistogram->w();
	int hei = m_pHistogram->h();

	int * count = new int[wid];
	assert(count);

	memset(count, 0, sizeof(int) * wid);

	int max = 0;
	for (int i = 0; i < m_nVolumeSize; i++) {
		float value = m_pVolume[i];

		int id = int(value * float(wid - 1));
		if (id > wid - 1)
			id = wid - 1;

		count[id]++;

		if (count[id] > max)
			max = count[id];

	}

	int sum = 0;
	for ( int i = 0; i < wid; i++)
		sum += count[i];

	int avg = sum / wid;

	uchar * histogram = m_pHistogram->m_pImage;
	memset(histogram, 0, wid * hei * 3);

	for (int x = 0; x < wid; x++) {
		int max_id = int(float(count[x]) / float(avg) * float(hei)); 
		if (max_id > hei - 1)
			max_id = hei - 1;

		for (int y = hei - 1; y > hei -1 - max_id; y--) {
			int id = x + y * wid;
			histogram[id * 3 + 0] = 255;
			histogram[id * 3 + 1] = 255;
			histogram[id * 3 + 2] = 255;
		}
	}    
	delete count; 
#endif

	datafile.close();

}
#endif


void cxVolume::ReadGrid(NetCFD_var* var_info)
{
	assert(var_info);
	/* allocate memory */ 
	int i = 0;
	for (int n = 0; n < var_info->rh_ndims; n++) {
		if(strcmp(var_info->dim[n]->dim_name,"time")==0)
			continue;
		int size = (int)var_info->dim[n]->length;

		if (m_pGrid[i] != NULL)
			delete [] m_pGrid[i];

		m_pGrid[i] = new float[size];
		assert(m_pGrid[i]);

		m_netcdf->get_vardata(const_cast<char *>(var_info->dim[n]->dim_name), 
			0, 1, 0, 1, 
			reinterpret_cast<char **>(&m_pGrid[i]), true);
		/* normalize */
		float min, max;

		min = m_pGrid[i][0];
		max = m_pGrid[i][size - 1];

		for (int j = 0; j < size; j++) {
			m_pGrid[i][j] = (m_pGrid[i][j] - min) / (max - min);
		}
		i++;
	}

	//rearrange m_pGrid
	float* temp = m_pGrid[2];
	m_pGrid[2] = m_pGrid[0];
	m_pGrid[0] = temp;

	for (int axis = 0; axis < 3; axis++) {        

		int size = m_vSize[axis];
		if(m_GridMap[axis]){
			//delete [] m_GridMap[axis];
			delete m_GridMap[axis];
		}

		m_GridMap[axis] = new float[size];

		for (int j = 0; j < size; j++) {
			m_GridMap[axis][j] = m_pGrid[axis][j];
		}
	}
}

void cxVolume::ReadGrid(char * filename)
{
	/* read the grid file */
	ifstream inf(filename, ios_base::binary);
	assert(inf);

	/* allocate memory */        
	for (int i = 0; i < 3; i++) {

		int size = (int)m_vSize[i];

		if (m_pGrid[i] != NULL)
			delete [] m_pGrid[i];

		m_pGrid[i] = new float[size];
		assert(m_pGrid[i]);

		inf.read(reinterpret_cast<char *>(m_pGrid[i]), sizeof(float) * size);

		/* normalize */
		float min, max;

		min = m_pGrid[i][0];
		max = m_pGrid[i][size - 1];

		for (int j = 0; j < size; j++) {
			m_pGrid[i][j] = (m_pGrid[i][j] - min) / (max - min);
		}
	}

	inf.close();    


	/*
	for (int axis = 0; axis < 3; axis++) {        

	int size = m_vSize[axis];
	m_GridMap[axis] = new float[GRID_SAMPLE]; 	
	// fill the look up table
	for (int i = 0; i < size - 1; i++) {
	int start = m_pGrid[axis][i] * GRID_SAMPLE;            
	int end = m_pGrid[axis][i+1] * GRID_SAMPLE;

	float start_v = (float) i / size;            
	float end_v = (float) (i+1) / size;
	for (int j = start; j <= end; j++) {
	float factor = ((float) (j - start)) / (end - start);
	float value = start_v * (1-factor) + end_v * (factor);

	m_GridMap[axis][j] = value;

	}
	}
	}
	*/
	for (int axis = 0; axis < 3; axis++) {        

		int size = m_vSize[axis];
		if(m_GridMap[axis]){
			//delete [] m_GridMap[axis];
			delete m_GridMap[axis];
		}

		m_GridMap[axis] = new float[size];

		for (int j = 0; j < size; j++) {
			m_GridMap[axis][j] = m_pGrid[axis][j];
		}
	}

}


void cxVolume::ReadTime(char *filename)
{
	int totaltime = m_nEndTime - m_nStartTime + 1;

	if (m_pTime == NULL) {

		m_pTime = new float[totaltime];
		assert(m_pTime);
	}

	/* read the time file */
	ifstream inf(filename);
	assert(inf);

	inf.read(reinterpret_cast<char *>(m_pTime), sizeof(float) * totaltime);

	inf.close();
}


void cxVolume::CreateDataTexPearson()
{
	//#ifdef PEARSONCPU
	if(m_nTexVolPearson==0)
	{
		glGenTextures(1, (GLuint*)&m_nTexVolPearson);
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D,m_nTexVolPearson);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// GPU data loading

		if (m_pVolume == NULL) {
			cerr << "The Volume is empty " << endl;
			return;
		}
		glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE16, (int)m_vSize[0],
			(int)m_vSize[1], (int)m_vSize[2]*m_nNumTimePearson, 0, GL_LUMINANCE, GL_FLOAT,
			//(int)m_vSize[1], (int)m_vSize[2], 0, GL_LUMINANCE, GL_FLOAT,
			m_pVolume);

		glDisable(GL_TEXTURE_3D);
	}else{

		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D, m_nTexVolPearson);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glDisable(GL_TEXTURE_3D);
	}
	//#endif //PEARSONCPU
}

void cxVolume::CreateDataTex()
{
	if(m_nTexVol==0)
	{
		glGenTextures(1, (GLuint*)&m_nTexVol);
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D,m_nTexVol);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		if (m_pVolume == NULL) {
			cerr << "The Volume is empty " << endl;
			return;
		}

		glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE16, (int)m_vSize[0],
			(int)m_vSize[1], (int)m_vSize[2], 0, GL_LUMINANCE, GL_FLOAT,
			m_pVolume);

		glDisable(GL_TEXTURE_3D);
	}else{

		glActiveTexture( GL_TEXTURE0);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D, m_nTexVol);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glDisable(GL_TEXTURE_3D);
	}
}


#ifdef SHOW_CLUSTER
void cxVolume::UpdateClusterId()
{
	if (m_nTexClus != 0) {
		glDeleteTextures(1, (const GLuint*)&m_nTexClus);
		m_nTexClus = 0;
	}
	CreateClusterTex();
	ReDraw(true);
}

void cxVolume::CreateClusterTex()
{

	if(m_nTexClus==0 && m_pClusView && m_pClusView->isInit())
	{
		int dimx = (int)m_vSize[0]/m_pClusView->m_xblock;
		int dimy = (int)m_vSize[1]/m_pClusView->m_yblock;
		int dimz = (int)m_vSize[2]/m_pClusView->m_zblock;
		int dimb =  dimx*dimy*dimz;
		int real_size = m_vSize[0]* m_vSize[1] * m_vSize[2];
		static float *ClusterMap = new float[real_size];
		int** clusdata = m_pClusView->getClusterVol();
		int cluste_item_count = 0;
		int n = 0;

		float val;
		int count = 0;
		bool validtimestep = (m_pClusView->getDim() <= m_time_in) ? false : true;
		int ind = m_time_in / m_pClusView->m_ts_chunk;

		int xblocksize = m_pClusView->m_xblock;
		int yblocksize = m_pClusView->m_yblock;
		int zblocksize = m_pClusView->m_zblock;
		for(int x1=0; x1 < m_vSize[0]-xblocksize+1; x1+=xblocksize){
			for(int y1=0; y1 < m_vSize[1]-yblocksize+1; y1+=yblocksize){
				for(int z1=0; z1 < m_vSize[2]-zblocksize+1; z1+=zblocksize){
					if (m_pClusView->currCluster() != -1 && 
						validtimestep && clusdata[ind][count] != m_pClusView->currCluster())
						val = 0.0;
					else
						val = 1.0;
					for(int x = x1; x< (x1+xblocksize); x++){
						for(int y = y1; y<(y1+yblocksize); y++){
							for(int z = z1; z<(z1+zblocksize); z++){
								int id = x + y * m_vSize[0] + z * m_vSize[0] * m_vSize[1];
								ClusterMap[id] = val;
							}
						}
					}
					count++;

				}
			}
		}
		/*
		for (int i = 0; i < dimb; i++)
		{
		if (clusdata[i] != m_pClusView->currCluster()){ 
		for(int j = 0; j < m_pClusView->m_xblock; j++){
		ClusterMap[n+j] = 0.0;
		}
		}
		else{
		for(int j = 0; j < m_pClusView->m_xblock; j++){
		ClusterMap[n+j] = 1.0;
		}
		cluste_item_count++;
		}
		n+=m_pClusView->m_xblock;
		}
		*/
		glGenTextures(1, (GLuint*)&m_nTexClus);
		glActiveTexture(GL_TEXTURE7);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D,m_nTexClus);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);    

		if (m_pVolume == NULL) {
			cerr << "The Volume is empty " << endl;
			return;
		}
		/*
		glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE16,
		(int)m_vSize[0]/m_pClusView->m_xblock,
		(int)m_vSize[1]/m_pClusView->m_yblock,
		(int)m_vSize[2]/m_pClusView->m_zblock, 0, GL_LUMINANCE, GL_FLOAT,
		ClusterMap);
		*/
		glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE16,
			(int)m_vSize[0],
			(int)m_vSize[1],
			(int)m_vSize[2], 0, GL_LUMINANCE, GL_FLOAT,
			ClusterMap);
		glDisable(GL_TEXTURE_3D);
	}else{

		glActiveTexture( GL_TEXTURE7);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D, m_nTexClus);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);    
		glDisable(GL_TEXTURE_3D);
	}
}

#endif


#ifdef SV_CORRELATION
void cxVolume::CreateCorrTex()
{

	if(m_bcorr && m_corrmap){

		glGenTextures(1, (GLuint*)&m_nTexCorr);
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D,m_nTexCorr);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);    

		if (m_pVolume == NULL) {
			cerr << "The Volume is empty " << endl;
			return;
		}

		glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE16,
			(int)m_vSize[0],
			(int)m_vSize[1],
			(int)m_vSize[2], 0, GL_LUMINANCE, GL_FLOAT,
			m_corrmap);
		glDisable(GL_TEXTURE_3D);
	}else{

		glActiveTexture( GL_TEXTURE1);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D, m_nTexCorr);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);    
		glDisable(GL_TEXTURE_3D);
	}
}
#endif


void cxVolume::CreateIndexDataTex()
{
	if(m_nIndTexVol==0)
	{
		glGenTextures(1, (GLuint*)&m_nIndTexVol);
		glActiveTexture(GL_TEXTURE6);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D,m_nIndTexVol);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		if (m_pVolume == NULL) {
			cerr << "The Volume is empty " << endl;
			return;
		}
		glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE16, (int)m_vSize[0],
			(int)m_vSize[1], (int)m_vSize[2], 0, GL_LUMINANCE, GL_FLOAT,
			m_pIndVolume);

		glDisable(GL_TEXTURE_3D);
	}else{

		glActiveTexture( GL_TEXTURE6);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D, m_nIndTexVol);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glDisable(GL_TEXTURE_3D);

	}
}


void cxVolume::GetEyePos()
{
	// calcuate the eye pos,
	GLdouble model[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model);

	// convert to the tranditional format
	double matrix[16];
	for (int i=0; i<4; i++)
		for (int j=0; j<4; j++)
			matrix[j+i*4] = model[i+j*4];

	inverse_mat(matrix, model);

	double eye[] = {0, 0, 0};
	double eye_rev[3];
	MAT_TIME_VEC(model, eye, eye_rev);     

	//translate to the texture coordinator
	m_vEye = cx3DVector(eye_rev[0] * m_fMaxSize / m_vSize[0] + 0.5, 
		eye_rev[1] * m_fMaxSize / m_vSize[1] + 0.5, 
		eye_rev[2] * m_fMaxSize / m_vSize[2] + 0.5); 

	//     m_vEye = cx3DVector((eye_rev[0] + 0.5 * m_vSize[0] / m_fMaxSize) / (m_vSize[0] / m_fMaxSize), 
	//                         (eye_rev[1] + 0.5 * m_vSize[1] / m_fMaxSize) / (m_vSize[1] / m_fMaxSize),
	//                         (eye_rev[2] + 0.5 * m_vSize[2] / m_fMaxSize) / (m_vSize[2] / m_fMaxSize)); 

#ifdef ORTHO
	double eyedir[] = {0,0,1};
	double eyedir_rev[3];
	VEC_TIME_MAT(model, eyedir, eyedir_rev);
	m_vEyeDir = cx3DVector(eyedir_rev);    

	m_vEyeDir = m_vEye - cx3DVector(0.5, 0.5, 0.5);

	m_vEyeDir.normalize();


#endif
}


void cxVolume::Draw(int nDrawMode, int nSliceAxis, int h, int w)
{
	if (m_nTexVol == 0 && m_nTexVolPearson == 0) {
		//    Read();
		return;
	}

	//if(m_nIndTexVol == 0){
	//	return;
	//}

	if (m_nVolProgram == 0 || m_bReload)
		InitProgram();

	CreateGridTex();
	if (nDrawMode == DRAW_3D_VOLUME)
		Draw3DVolume();
	else if(nDrawMode == DRAW_2D_SLICE){
		Draw2DSlice(nSliceAxis,h,w);
	}   
	m_pVisStatus->WriteToFile();    
}


void cxVolume::Draw3DVolume()
{

#if defined(CLIMATE)    
	glScalef(0.44, 1.0, 1.0);
#endif

	GetEyePos();    



	glPushMatrix();

	float scale = 1.0 / m_fMaxSize;

	glScalef(scale, scale, scale);      
	glTranslatef( -m_vCenter[0], -m_vCenter[1], -m_vCenter[2]);

	//CreateGridTex();
#ifdef POINT_CLOUD
	//if (m_bDrawPointCloud)
	//DrawPointCloud();
#else
	if (m_pVisStatus->m_bDrawVolume)
		DrawVolume();
#endif		

	//DrawGrid();

	if (m_pVisStatus->m_bDrawAxes)
		DrawAxes();

	if (m_pVisStatus->m_bDrawFrame)
		DrawFrame();
#ifdef PEARSONCPU
	if (!m_bPearsonVariable) DrawReferenceLocation();
#endif

	if(m_bvolsel)
		DrawVolSel();





#ifdef DRAWIMPORTANCE        
	DrawImportance();            
#endif

	//if (m_bDraw2DSliceBoundary)
	Draw2DSliceBoundary();

	if (m_bDrawColorMap)
		DrawColorMap();


	//if (m_bCorr)
	//	DrawCorrelation();
	glPopMatrix();
}

void cxVolume::DrawImportance()
{
	char filename[1024];

	if (m_nVariable == DATA_TEMP) {

		sprintf (filename, "/home/hfyu/work/climate/importance/temp-importance/importance_%04d.txt", m_nCurTime);

	} else if (m_nVariable == DATA_SALT) {

		sprintf (filename, "/home/hfyu/work/climate/importance/salt-importance/importance_%04d.txt", m_nCurTime);

	}

	ifstream inf;

	inf.open(filename);

	assert(inf);

	int grid_x = 12;
	int grid_y = 6;

	float size_x = m_vSize[0] / grid_x;
	float size_y = m_vSize[1] / grid_y;

	int grid_id;
	float importance;
	int x, y;    
	float low_x, low_y, top_x, top_y;

	glDisable(GL_LIGHTING);
	glLineWidth(2.0);
	glColor3f(1, 0, 0);    

	for (int i = 0; i < m_nImportance; i++) 
	{

		inf >> grid_id >> importance;        


		y = grid_id / grid_x;

		x = grid_id - grid_x * y;

		low_x = x * size_x;

		low_y = y * size_y;

		top_x = low_x + size_x;

		top_y = low_y + size_y;


		glBegin(GL_LINE_LOOP);
		glVertex3f(low_x, low_y, m_vSize[2] + 1);
		glVertex3f(top_x, low_y, m_vSize[2] + 1);
		glVertex3f(top_x, top_y, m_vSize[2] + 1);
		glVertex3f(low_x, top_y, m_vSize[2] + 1);
		glVertex3f(low_x, low_y, m_vSize[2] + 1);
		glEnd();

	}

}

/*
void cxVolume::DrawColorMap()
{
glMatrixMode(GL_PROJECTION);
glPushMatrix();
glLoadIdentity();

glMatrixMode(GL_MODELVIEW);
glPushMatrix();
glLoadIdentity();    

gl_font(FL_HELVETICA_BOLD, 12);
gl_color(FL_WHITE);

float x, y, delta;
x = 0.8;//0.3
//y = -0.9 + m_nVolumeID * 0.15;
y = -0.5; //-0.9 + m_nVolumeID * 0.22;
delta = 0.08;

switch(m_nVariable) {
case DATA_MIXFRAC : gl_draw("MIXFRAC", x, y + delta);
break;
case DATA_HO2     : gl_draw("HO2", x, y + delta);
break;                            
case DATA_OH      : gl_draw("OH", x, y + delta);
break;                            
case DATA_CHI     : gl_draw("CHI", x, y + delta);
break;
case DATA_TEMP    : gl_draw("TEMP (degC)", x - delta, y - 3*delta);
break;   
case DATA_TEMP_A    : gl_draw("TEMP_A (degC)", x - delta, y - 3*delta);
break;   
case DATA_SALT    : gl_draw("SALT (psu)", x - delta, y - 3*delta);
break;
case DATA_SALT_A    : gl_draw("SALT_A (psu)", x - delta, y - 3*delta);
break;
case DATA_WT    : gl_draw("WT (psu)", x - delta, y - 3* delta);
break;                                                    
}

char buf[1024];
float halfdelta = delta * 0.5;
if(m_fMin < 1.0)
sprintf(buf, "%.2e", m_fMin);
//sprintf(buf, "%.2f", m_fMin);    
else
sprintf(buf, "%.2f", m_fMin);
//gl_draw(buf, x, y + delta - 0.15);
gl_draw(buf, (x + halfdelta), y);

if(m_fMax < 1.0)
sprintf(buf, "%.2e", m_fMax);
//sprintf(buf, "%.8f", m_fMax);    
else
sprintf(buf, "%.2f", m_fMax);
//gl_draw(buf, x + 0.52f, y + delta - 0.15f);
gl_draw(buf, (x + halfdelta), y + 1.0 + delta - 0.15f);

// convert the NetCDF time coordinates to a calendar date 

#ifdef LINUX
utUnit unit;
int year=0, month, day, hour, minute;
float second;

if (utScan("days since 0001-01-01 00:00:00", &unit) != 0) {
printf("Error! Can not convert units\n");        
}

int cur = m_nCurTime - m_nStartTime;

utCalendar(m_pTime[cur], &unit, &year, &month, &day, &hour, &minute, &second);

//sprintf(buf, "TIME : %04d ( %02d mo | %04d yr )", m_nCurTime, m_nCurTime%12 + 1, m_nCurTime / 12 + 1);
sprintf(buf, " timestep : %04d   mm/dd/yy : %02d-%02d-%04d", cur, month, day, year);

gl_draw(buf, -1.0f, -0.95f);
#endif

glTranslatef(x, y, 0);    
glRotatef(90,0,0,1);
#ifdef SHOWOPACITY
glScalef(0.6, 0.15, 1);
#else
////glScalef(0.6, 0.06, 1);
glScalef(1, 0.06, 1);
#endif

CreateColorMapTex();

glActiveTexture(GL_TEXTURE3);
glEnable(GL_TEXTURE_2D);

glBegin(GL_QUADS);
glMultiTexCoord2f(GL_TEXTURE3, 0, 0);
glVertex2f(0,0);

glMultiTexCoord2f(GL_TEXTURE3, 1, 0);
glVertex2f(1,0);         

glMultiTexCoord2f(GL_TEXTURE3, 1, 1);
glVertex2f(1,1);

glMultiTexCoord2f(GL_TEXTURE3, 0, 1);
glVertex2f(0,1);
glEnd();

glDisable(GL_TEXTURE_2D);    

glPopMatrix();
glPopMatrix();
}
*/




void cxVolume::DrawColorMap()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();    

	gl_font(FL_HELVETICA_BOLD, 12);
	gl_color(FL_WHITE);
	//gl_color(FL_BLACK);

	float x, y, delta;
	/*
	x = -0.55;
	y = -0.96 + m_nVolumeID * 0.22;
	delta = 0.08;
	//gl_draw("PROGRESS", x, y + delta);
	gl_draw("SAMPLING SPACING", x, y + delta);
	*/
	x = 0.3;
	//y = -0.9 + m_nVolumeID * 0.15;
	y = -0.9 + m_nVolumeID * 0.22;
	delta = 0.08;

	switch(m_nVariable) {
		case DATA_MIXFRAC : gl_draw("MIXFRAC", x, y + delta);
			break;
		case DATA_HO2     : gl_draw("HO2", x, y + delta);
			break;
		case DATA_OH      : gl_draw("OH", x, y + delta);
			break;
		case DATA_CHI     : gl_draw("CHI", x, y + delta);
			break;
		case DATA_TEMP    : gl_draw("TEMPERATURE (degC)", x, y + delta);
			break;   
		case DATA_TEMP_A    : gl_draw("TEMPERATURE_A (degC)", x, y + delta);
			break;   
		case DATA_SALT    : gl_draw("SALINITY (psu)", x, y + delta);
			break;
		case DATA_SALT_A    : gl_draw("SALINITY_A (psu)", x, y + delta);
			break;
		case DATA_WT    : gl_draw("WT (psu)", x, y + delta);
			break;                                                   
			//case DATA_TEMP    : gl_draw("TEMPERATURE (degC)", x, y + delta);
		case DATA_TEMP_TEMP    : gl_draw("TEMPERATURE SELF CORRELATION", x, y + delta);
			break;
		case DATA_SALT_SALT    : gl_draw("SALINITY SELF CORRELATION", x, y + delta);
			break;
		case DATA_WT_WT    : gl_draw("WT SELF CORRELATION", x, y + delta);
			break;

			//case DATA_SALT    : gl_draw("SALINITY (psu)", x, y + delta);
		case DATA_SALT_TEMP    : //if (!m_bPearsonVariable)							  
			//gl_draw("SALINITY CORRELATION", x, y + delta);
			//	gl_draw("SALINITY & TEMPERATURE CORRELATION", x, y + delta);
			//else
			gl_draw("SALINITY AND TEMPERATURE CORRELATION", x, y + delta);
			break;
		case DATA_WT_TEMP   :	gl_draw("WT AND TEMPERATURE CORRELATION", x, y + delta);
			break;
		case DATA_WT_SALT   :	gl_draw("SALINITY AND WT CORRELATION", x, y + delta);
			break;
	}

	char buf[1024];

	//sprintf(buf, "%.2f", m_fMin);    
	sprintf(buf, "%.2f", -1.0);    
	gl_draw(buf, x - 0.02f, y + delta - 0.15f);

	sprintf(buf, "%.2f", 0.0);    
	gl_draw(buf, x + 0.27f, y + delta - 0.15f);

	//sprintf(buf, "%.2f", m_fMax);
	sprintf(buf, "%.2f", 1.0);    
	gl_draw(buf, x + 0.56f, y + delta - 0.15f);

	int offsetm = 10, offsety = 199;

	if (!m_bCorrelation)
		sprintf(buf, "TIMESTEP : %03d", m_time_in);
	else{
#ifdef CLIMATE
		//sprintf(buf, "TIMESTEP : %04d   YEAR : %03d   MONTH : %02d", m_nCurTime, (m_nCurTime/12)+1, (m_nCurTime%12)+1);			
		//sprintf(buf, "TIMESTEP : %04d   MM/YY : %02d-%04d", m_nCurTime, ((m_nCurTime+offsetm)%12)+1, offsety+((m_nCurTime+offsetm)/12)+1);
		if (m_bPearsonVariable)
			sprintf(buf, "TIMESTEPS : %04d - %04d", m_nStartTimePearson, m_nEndTimePearson);
		else {
			//float xlocation = m_pVisStatus->m_fXLocation - m_fOffsetX;
			//if (xlocation<0.0) xlocation += 1.0;
			//if (xlocation>1.0) xlocation -= 1.0;
			char xlabel[20], ylabel[20], zlabel[20];
			getLabels(xlabel, ylabel, zlabel);
			//if (m_bLocationPearsonAnimation)
			//sprintf(buf, "TIMESTEPS : %04d - %04d   REFERENCE LOCATION : (%s%s%s)", 0, 119, xlabel, ylabel, zlabel);
			//sprintf(buf, "TIMESTEPS : %04d - %04d   REFERENCE LOCATION : (%s%s%s) TEMPERATURE", 0, 119, xlabel, ylabel, zlabel);
			//sprintf(buf, "TIMESTEPS : %04d - %04d   REFERENCE LOCATION : (%s%s%s) SALINITY", m_time_in, m_nNumTimePearson, xlabel, ylabel, zlabel);
			//else
			sprintf(buf, "TIMESTEPS : %04d - %04d   REFERENCE LOCATION : (%s%s%s)", m_time_in, m_time_in+m_nNumTimePearson, xlabel, ylabel, zlabel);
			//sprintf(buf, "TIMESTEPS : %04d - %04d   REFERENCE LOCATION : (%s%s%s)", 0, 119, xlabel, ylabel, zlabel);
			//sprintf(buf, "TIMESTEPS : %04d - %04d   REFERENCE LOCATION : (%.2f, %.2f, %.2f)", m_nStartTimePearson, m_nEndTimePearson, xlocation, m_pVisStatus->m_fYLocation, m_pVisStatus->m_fZLocation);
		}
	}
#elif defined (EARTHQUAKE)
		if (m_sCltFileName!=NULL && m_nCltId!=-1) sprintf(buf, "TIMESTEP : %03d   CLUSTER ID : %02d COUNT : %04d", m_nCurTime, m_nCltId, m_nCltNumBK);
		else sprintf(buf, "TIMESTEP : %03d", m_nCurTime);
#elif defined (VORTS)
		if (m_sCltFileName!=NULL && m_nCltId!=-1) sprintf(buf, "TIMESTEP : %03d   CLUSTER ID : %02d COUNT : %04d", m_nCurTime, m_nCltId, m_nCltNumBK);
		else sprintf(buf, "TIMESTEP : %03d", m_nCurTime);
#elif defined (HURRICANE)
		//if (m_sCltFileName!=NULL && m_nCltId!=-1) sprintf(buf, "TIMESTEP : %02d   CLUSTER ID : %02d of 03 COUNT : %04d", m_nCurTime, m_nCltId, m_nCltNumBK);
		if (m_sCltFileName!=NULL && m_nCltId!=-1) sprintf(buf, "TIMESTEP : %02d   CLUSTER ID : %02d COUNT : %04d", m_nCurTime, m_nCltId, m_nCltNumBK);
		else sprintf(buf, "TIMESTEP : %02d", m_nCurTime);
#elif defined (COMBUSTION960)
		if (m_sCltFileName!=NULL && m_nCltId!=-1) sprintf(buf, "TIMESTEP : %03d   CLUSTER ID : %02d COUNT : %04d", m_nCurTime, m_nCltId, m_nCltNumBK);
		else sprintf(buf, "TIMESTEP : %02d", m_nCurTime);
#else
		sprintf(buf, "TIMESTEP : %04d", m_nCurTime);
#endif
		//gl_draw(buf, -1.0f, -0.95f);
		gl_draw(buf, -0.95f, -0.95f);

		glTranslatef(x, y, 0);    
#ifdef SHOWOPACITY
		glScalef(0.6, 0.15, 1);
#else
		glScalef(0.6, 0.06, 1);
#endif

		CreateColorMapTex();

		glActiveTexture(GL_TEXTURE3);
		glEnable(GL_TEXTURE_2D);

		glBegin(GL_QUADS);
		glMultiTexCoord2f(GL_TEXTURE3, 0, 0);
		glVertex2f(0,0);

		glMultiTexCoord2f(GL_TEXTURE3, 1, 0);
		glVertex2f(1,0);         

		glMultiTexCoord2f(GL_TEXTURE3, 1, 1);
		glVertex2f(1,1);

		glMultiTexCoord2f(GL_TEXTURE3, 0, 1);
		glVertex2f(0,1);
		glEnd();

		glDisable(GL_TEXTURE_2D);    

		glPopMatrix();
		glPopMatrix();
	}


	void cxVolume::DrawGrid()
	{
		int sizex = (int) m_vSize[0];
		int sizey = (int) m_vSize[1];
		int sizez = (int) m_vSize[2];

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity(); 


		//glPushMatrix();
		//glEnable(GL_DEPTH_TEST);
		glScalef(2.0, 2.0, 2.0);     
		glTranslatef( -0.5, -0.5, -0.5);

		glDisable(GL_LIGHTING);
		glLineWidth(1.0);
		glColor3f(1.0f,1.0f,0.0f);

		for (int i = 0; i < sizex; i++){       
			glBegin(GL_LINES);
			glVertex3f((float)m_pGrid[0][i]/(float)m_pGrid[0][sizex-1], (float)m_pGrid[1][0]/(float)m_pGrid[1][sizey-1], (float)m_pGrid[2][0]/(float)m_pGrid[2][sizez-1]);
			glVertex3f((float)m_pGrid[0][i]/(float)m_pGrid[0][sizex-1], (float)m_pGrid[1][sizey-1]/(float)m_pGrid[1][sizey-1], (float)m_pGrid[2][0]/(float)m_pGrid[2][sizez-1]);
			glEnd();
		}

		for (int i = 0; i < sizey; i++){       
			glBegin(GL_LINES);
			glVertex3f((float)m_pGrid[0][0]/(float)m_pGrid[0][sizex-1], (float)m_pGrid[1][i]/(float)m_pGrid[1][sizey-1], (float)m_pGrid[2][0]/(float)m_pGrid[2][sizez-1]);
			glVertex3f((float)m_pGrid[0][sizex -1]/(float)m_pGrid[0][sizex-1], (float)m_pGrid[1][i]/(float)m_pGrid[1][sizey-1], (float)m_pGrid[2][0]/(float)m_pGrid[2][sizez-1]);
			glEnd();
		}

		for (int i = 0; i < sizez; i++){       
			glBegin(GL_LINES);
			glVertex3f((float)m_pGrid[0][0]/(float)m_pGrid[0][sizex-1], (float)m_pGrid[1][0]/(float)m_pGrid[1][sizey-1], (float)m_pGrid[2][i]/(float)m_pGrid[2][sizez-1]);
			glVertex3f((float)m_pGrid[0][sizex -1]/(float)m_pGrid[0][sizex-1], (float)m_pGrid[1][0]/(float)m_pGrid[1][sizey-1], (float)m_pGrid[2][i]/(float)m_pGrid[2][sizez-1]);
			glEnd();
		}

		//glDisable(GL_DEPTH_TEST);
		//glPopMatrix();
		glPopMatrix();
		glPopMatrix();
	}


	void cxVolume::DrawFrame()
	{
		float el = -1;
		float ew = -1;
		float eh = -1;
		float sl = m_vSize[0] + 1;
		float sw = m_vSize[1] + 1;
		float sh = m_vSize[2] + 1;

		glPushMatrix();

		glDisable(GL_LIGHTING);
		glLineWidth(1.0);

		if (m_pVisView->m_bBlack)
			glColor3f(1.0f,1.0f,1.0f);
		else
			glColor3f(0.0f,0.0f,0.0f);

		glBegin(GL_LINE_LOOP);
		glVertex3f(sl,sw,eh);
		glVertex3f(el,sw,eh);
		glVertex3f(el,ew,eh);
		glVertex3f(sl,ew,eh);
		glVertex3f(sl,sw,eh);
		glEnd();

		glBegin(GL_LINE_STRIP);
		glVertex3f(sl,sw,eh);
		glVertex3f(sl,sw,sh);
		glVertex3f(sl,ew,sh);
		glVertex3f(sl,ew,eh);
		glEnd();

		glBegin(GL_LINE_STRIP);
		glVertex3f(el,sw,eh);
		glVertex3f(el,sw,sh);
		glVertex3f(el,ew,sh);
		glVertex3f(el,ew,eh);
		glEnd();

		glBegin(GL_LINES);
		glVertex3f(sl,ew,sh);
		glVertex3f(el,ew,sh);
		glEnd();

		glBegin(GL_LINES);
		glVertex3f(sl,sw,sh);
		glVertex3f(el,sw,sh);
		glEnd();

		glLineWidth(1.0);
		glColor3f(1.0f,0.0f,0.0f);
		glBegin(GL_LINES);

		glVertex3f(el,ew,eh);
		glVertex3f(sl,ew,eh);
		glEnd();

		glColor3f(0.0f,1.0f,0.0f);
		glBegin(GL_LINES);
		glVertex3f(el,ew,eh);
		glVertex3f(el,sw,eh);
		glEnd();

		glColor3f(0.0f,0.0f,1.0f);
		glBegin(GL_LINES);
		glVertex3f(el,ew,eh);
		glVertex3f(el,ew,sh);
		glEnd();

		glPopMatrix();
	}

	void cxVolume::DrawVolSel()
	{
		float el = m_context_x_start -1;
		float ew = m_context_y_start -1;
		float eh = m_context_z_start -1;
		float sl = m_context_x_end + 1;
		float sw = m_context_y_end + 1;
		float sh = m_context_z_end + 1;

		glPushMatrix();

		glDisable(GL_LIGHTING);
		glLineWidth(1.0);

		if (m_pVisView->m_bBlack)
			glColor3f(1.0f,1.0f,1.0f);
		else
			glColor3f(0.0f,0.0f,0.0f);
		glColor3f(1.0f,0.0f,0.0f);

		glBegin(GL_LINE_LOOP);
		glVertex3f(sl,sw,eh);
		glVertex3f(el,sw,eh);
		glVertex3f(el,ew,eh);
		glVertex3f(sl,ew,eh);
		glVertex3f(sl,sw,eh);
		glEnd();

		glBegin(GL_LINE_STRIP);
		glVertex3f(sl,sw,eh);
		glVertex3f(sl,sw,sh);
		glVertex3f(sl,ew,sh);
		glVertex3f(sl,ew,eh);
		glEnd();

		glBegin(GL_LINE_STRIP);
		glVertex3f(el,sw,eh);
		glVertex3f(el,sw,sh);
		glVertex3f(el,ew,sh);
		glVertex3f(el,ew,eh);
		glEnd();

		glBegin(GL_LINES);
		glVertex3f(sl,ew,sh);
		glVertex3f(el,ew,sh);
		glEnd();

		glBegin(GL_LINES);
		glVertex3f(sl,sw,sh);
		glVertex3f(el,sw,sh);
		glEnd();

		glLineWidth(1.0);
		glColor3f(1.0f,0.0f,0.0f);
		glBegin(GL_LINES);

		glVertex3f(el,ew,eh);
		glVertex3f(sl,ew,eh);
		glEnd();

		glColor3f(0.0f,1.0f,0.0f);
		glBegin(GL_LINES);
		glVertex3f(el,ew,eh);
		glVertex3f(el,sw,eh);
		glEnd();

		glColor3f(0.0f,0.0f,1.0f);
		glBegin(GL_LINES);
		glVertex3f(el,ew,eh);
		glVertex3f(el,ew,sh);
		glEnd();

		glPopMatrix();
	}

	void cxVolume::DrawReferenceLocation()
	{
#ifdef PEARSONCPU
		glDisable(GL_DEPTH_TEST);	

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glPushMatrix();

		glDisable(GL_LIGHTING);
		glLineWidth(2.0);

		//glColor3f(1.0f,1.0f,1.0f);

		float xlocation, ylocation, zlocation;

		if (m_bLocationPearsonAnimation)
		{
			xlocation = m_nXLocationPearson[m_nLocationPearson] / (m_vSize[0]-1);
			ylocation = m_nYLocationPearson[m_nLocationPearson] / (m_vSize[1]-1);
			zlocation = m_nZLocationPearson[m_nLocationPearson] / (m_vSize[2]-1);
		}
		else
		{
			xlocation = m_pVisStatus->m_fXLocation;
			ylocation = m_pVisStatus->m_fYLocation;
			zlocation = m_pVisStatus->m_fZLocation;
		}


		//glColor4f(0.0f, 0.0f, 0.0f, 0.9 * zlocation + 0.1);
		glColor4f(1.0f, 1.0f, 1.0f, 0.9 * zlocation + 0.1);

		xlocation = xlocation - m_fOffsetX;
		if (xlocation<0.0) xlocation += 1.0;
		if (xlocation>1.0) xlocation -= 1.0;

		float x = m_vSize[0] * xlocation;
		float y = m_vSize[1] * ylocation;
		float z = m_vSize[2] * zlocation;

		glBegin(GL_LINES);
		glVertex3f(x-6.0, y, z);
		glVertex3f(x+6.0, y, z);
		glVertex3f(x, y-2.0, z);
		glVertex3f(x, y+2.0, z);
		glVertex3f(x, y, z-1.0);
		glVertex3f(x, y, z+1.0);
		glEnd();

		glPopMatrix();

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
#endif //PEARSONCPU
	}

	float Avg(float* data, int len)
	{
		float total = 0;
		for(int i=0; i < len; i++){
			total += data[i];
		}
		return (float)(total/len);
	}
	float FindVar(float* data, int len)
	{
		if(len == 0)
			return 0;
		float temp;
		float sum = 0;
		float avg = Avg(data, len);

		for(int j = 0; j < len; j++){
			temp = (data[j] - avg) * (data[j] - avg);
			sum = sum + temp;
		}
		return sum/len;
	}

	void cxVolume::UpdatePointCloudSettings(int new_xblock, int new_yblock, int new_zblock, float new_thold)
	{
		m_pVisStatus->xblock = new_xblock;
		m_pVisStatus->yblock = new_yblock;
		m_pVisStatus->zblock = new_zblock;
		m_pVisStatus->thold = new_thold;
	}
#ifdef POINT_CLOUD
	void cxVolume::DrawPointCloud()
	{
		float el = 0;
		float ew = 0;
		float eh = 0;
		float sl = m_vSize[0];
		float sw = m_vSize[1];
		float sh = m_vSize[2];

		glPushMatrix();

		glDisable(GL_LIGHTING);
		glLineWidth(1.0);

		glPointSize(2.0); //default 1.0
		int nexty = 100;



		// faster version
		/*
		glBegin(GL_POINTS);
		for (int z = 0; z < m_vSize[2]; z+=3){
		for (int y = 0; y < m_vSize[1]; y+=1){ 
		for (int x = 0; x < m_vSize[0]; x+=6){
		int id = x + y * m_vSize[0] + z * m_vSize[0] * m_vSize[1];
		if(m_pVolume[id]!=0){
		//glColor3f(1.0f,0.0f,0.0f);
		glColor3f(m_pVolume[id],m_pVolume[id],0.0f);
		glVertex3f(((float)m_GridSpace[0][x]/(float)GRID_SAMPLE)*m_vSize[0], 
		((float)m_GridSpace[1][y]/(float)GRID_SAMPLE)*m_vSize[1],z);
		}
		}
		nexty =0;
		if(y<m_vSize[1]-2){
		for(int i=y; i<y+3;i++){
		if(m_GridSpace[1][y+1]-m_GridSpace[1][y] > GRID_SAMPLE/m_vSize[1])
		nexty++;
		else
		break;
		}
		}
		if(nexty==0){
		for(int i=y; i<y+2;i++){
		if(m_GridSpace[1][y+1]-m_GridSpace[1][y] > 0.75*GRID_SAMPLE/m_vSize[1])
		nexty++;
		else
		break;
		}
		}
		y+=nexty;
		//fprintf(stderr,"y[%d]=%f\n",y,m_GridSpace[1][z]*m_vSize[1]);
		}
		//fprintf(stderr,"z[%d]=%d\n",z,m_GridSpace[2][z]);
		}
		glEnd();
		*/

		int xblocksize=m_pVisStatus->xblock;
		int yblocksize=m_pVisStatus->yblock;
		int zblocksize=m_pVisStatus->zblock;
		float st = 0.0;
		float VAR_THRESHOLD = m_pVisStatus->thold; 
		int len = xblocksize*yblocksize*zblocksize*m_nt_pointcloud;
		int datasize = m_vSize[0]*m_vSize[1]*m_vSize[2];
		float* dataPts = new float[len];
		float** TimeData;
		TimeData = new float*[m_nt_pointcloud];
		float* temp = m_pVolume;
		for(int t=0; t<m_nt_pointcloud;t++){
			TimeData[t] = temp;
			temp += datasize;
		}

		glBegin(GL_POINTS);
		for(int z1=0; z1 < m_vSize[2]-zblocksize+1; z1+=zblocksize){
			for(int y1=0; y1 < m_vSize[1]-yblocksize+1; y1+=yblocksize){
				for(int x1=0; x1 < m_vSize[0]-xblocksize+1; x1+=xblocksize){
					int count=0;
					for(int t=0; t<m_nt_pointcloud;t++){
						for(int z = z1; z< (z1+zblocksize); z++){
							for(int y = y1; y<(y1+yblocksize); y++){
								for(int x = x1; x<(x1+xblocksize); x++){
									int id = x + y * m_vSize[0] + z * m_vSize[0] * m_vSize[1];
									dataPts[count] = TimeData[t][id];//m_pVolume[id];
									count++;
								}
							}
						}
					}

					if(len == 1)
						st = dataPts[0];
					else{
						st = FindVar(dataPts, len);
						//fprintf(stderr,"st=%f\n",st);
					}
					
					if(st>VAR_THRESHOLD)
					{
						int xcoord=x1+xblocksize/2+1;
						int ycoord=y1+yblocksize/2+1;
						int zcoord=z1+zblocksize/2+1;
						if(xcoord>=m_vSize[0]) xcoord=m_vSize[0]-1;
						if(ycoord>=m_vSize[1]) ycoord=m_vSize[1]-1;
						if(zcoord>=m_vSize[2]) zcoord=m_vSize[2]-1;

						int id = xcoord + ycoord * m_vSize[0] + zcoord * m_vSize[0] * m_vSize[1];
						if(m_pVolume[id]!=0){
							glColor3f(m_pVolume[id],m_pVolume[id],0.0f);
							glVertex3f(((float)m_GridSpace[0][xcoord]/(float)GRID_SAMPLE)*m_vSize[0], 
								((float)m_GridSpace[1][ycoord]/(float)GRID_SAMPLE)*m_vSize[1],zcoord);
						}
						//fprintf(stderr,"st=%f\n",st);
					}
					
				}
			}
		}
		glEnd();
		delete [] dataPts;
		delete [] TimeData;
		glPopMatrix();

	}
#endif
	void cxVolume::DrawAxes()
	{

		float pf = 10.0;
		// arrays for lighting
		GLfloat light_position0[ ] = {1.0f * pf, 1.0f * pf, 1.0f * pf, 0.0};
		GLfloat light_specular0[ ] =  {1.0, 1.0, 1.0, 1.0};
		GLfloat light_ambient0[ ] =  {1.0, 1.0, 1.0, 0.5};
		GLfloat light_diffuse0[ ] =  {1.0, 1.0, 1.0, 1.0};

		// For Lighting
		glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse0 );
		glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular0 );
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient0 );
		glLightfv(GL_LIGHT0, GL_POSITION, light_position0 );
		glEnable( GL_LIGHT0 );

		float x = m_vSize[0];
		float y = m_vSize[1];
		float z = m_vSize[2];

		float zoom = x / 5.0;

		glPushMatrix();

		glTranslatef(m_vCenter[0], m_vCenter[1], m_vCenter[2]);

		//glDisable(GL_LIGHTING);

		glLineWidth(1.0);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		//RED - X
		glBegin(GL_LINES);
		glColor3f(1.0,0.0,0.0);
		glVertex3f(0, 0, 0);
		glVertex3f(x,0,0);
		glEnd();

		//GREEN - Y
		glBegin(GL_LINES);
		glColor3f(0.0,1.0,0.0);
		glVertex3f(0, 0, 0);
		glVertex3f(0,y,0);
		glEnd();

		//BLUE - Z
		glBegin(GL_LINES);
		glColor3f(0.0,0.0,1.0);
		glVertex3f(0, 0, 0);
		glVertex3f(0,0,z);
		glEnd();    
		glDisable(GL_CULL_FACE);

		glEnable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		GLfloat light0_ambient[] =  {1.0f, 1.0f, 1.0f, 1.0f};
		glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
		glPushMatrix();
		GLfloat mat_ambient0[] = { 0.5, 0.0, 0.0, 1.0 };
		GLfloat mat_diffuse0[] = { 0.5, 0.0, 0.0, 1.0 };        
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient0);
		glMaterialfv(GL_FRONT, GL_DIFFUSE,mat_diffuse0);
		glTranslatef(x,0,0);
		glRotatef(90,0,1,0);
		glScalef(zoom, zoom, zoom);         
		//glutSolidCone(0.1, 0.3,8,8);
		glPopMatrix();

		glPushMatrix();
		GLfloat mat_ambient1[] = { 0.0, 0.5, 0.0, 1.0 };
		GLfloat mat_diffuse1[] = { 0.0, 0.5, 0.0, 1.0 };
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient1);
		glMaterialfv(GL_FRONT, GL_DIFFUSE,mat_diffuse1);
		glTranslatef(0,y,0);
		glRotatef(90,-1,0,0);
		glScalef(zoom, zoom, zoom);              
		//glutSolidCone(0.1, 0.3,8,8);
		glPopMatrix();

		glPushMatrix();
		GLfloat mat_ambient2[] = { 0.0, 0.0, 0.5, 1.0 };
		GLfloat mat_diffuse2[] = { 0.0, 0.0, 0.5, 1.0 };
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient2);
		glMaterialfv(GL_FRONT, GL_DIFFUSE,mat_diffuse2);
		glTranslatef(0,0,z);
		glScalef(zoom, zoom, zoom);               
		//glutSolidCone(0.1, 0.3,8,8);
		glPopMatrix();  

		glDisable(GL_LIGHTING);

		glPopMatrix();
	}

	void cxVolume::TurnOffTextureAndPrograms()
	{
		for(int i=0;i<8;i++)
		{
			glActiveTexture( GL_TEXTURE0+i);
			glDisable(GL_TEXTURE_1D);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_3D);
			glDisable(GL_TEXTURE_RECTANGLE_NV);
		}

		// turn off fragment program
		glDisable(GL_FRAGMENT_PROGRAM_ARB);

		// turn off GL Shading language
		glUseProgram(0); 
	}


	void cxVolume::InitProgram()
	{
		GLchar *VertexShaderSource, *FragmentShaderSource;
		readShaderSource("myVOL", &VertexShaderSource, &FragmentShaderSource);
		int success = installShaders(VertexShaderSource, FragmentShaderSource, (GLuint*)&(m_nVolProgram));        
		if ( !success)
		{
			fprintf(stderr, "Error: Can not install shaders.\n");   
		}
		free(VertexShaderSource);
		free(FragmentShaderSource);

		m_bReload = false;
	}

	void cxVolume::DrawCube()
	{
		float minx = m_pVisStatus->m_vRangeMin[0];
		float miny = m_pVisStatus->m_vRangeMin[1];
		float minz = m_pVisStatus->m_vRangeMin[2];

		float maxx = m_pVisStatus->m_vRangeMax[0];
		float maxy = m_pVisStatus->m_vRangeMax[1];
		float maxz = m_pVisStatus->m_vRangeMax[2]; 

		float c[8][3] = {{minx,miny,minz},{maxx,miny,minz},{minx,maxy,minz},{maxx,maxy,minz},
		{minx,miny,maxz},{maxx,miny,maxz},{minx,maxy,maxz},{maxx,maxy,maxz}};

		minx = minx * m_vSize[0];
		miny = miny * m_vSize[1];
		minz = minz * m_vSize[2];

		maxx = maxx * m_vSize[0];
		maxy = maxy * m_vSize[1];
		maxz = maxz * m_vSize[2];

		float v[8][3] = {{minx,miny,minz},{maxx,miny,minz},{minx,maxy,minz},{maxx,maxy,minz},
		{minx,miny,maxz},{maxx,miny,maxz},{minx,maxy,maxz},{maxx,maxy,maxz}};


		glBegin(GL_QUADS);
		glColor3fv(c[0]); glVertex3fv(v[0]);
		glColor3fv(c[1]); glVertex3fv(v[1]);
		glColor3fv(c[3]); glVertex3fv(v[3]);
		glColor3fv(c[2]); glVertex3fv(v[2]);

		glColor3fv(c[1]); glVertex3fv(v[1]);
		glColor3fv(c[5]); glVertex3fv(v[5]);
		glColor3fv(c[7]); glVertex3fv(v[7]);
		glColor3fv(c[3]); glVertex3fv(v[3]);

		glColor3fv(c[4]); glVertex3fv(v[4]);
		glColor3fv(c[6]); glVertex3fv(v[6]);
		glColor3fv(c[7]); glVertex3fv(v[7]);
		glColor3fv(c[5]); glVertex3fv(v[5]);

		glColor3fv(c[0]); glVertex3fv(v[0]);
		glColor3fv(c[2]); glVertex3fv(v[2]);
		glColor3fv(c[6]); glVertex3fv(v[6]);
		glColor3fv(c[4]); glVertex3fv(v[4]);

		glColor3fv(c[0]); glVertex3fv(v[0]);
		glColor3fv(c[4]); glVertex3fv(v[4]);
		glColor3fv(c[5]); glVertex3fv(v[5]);
		glColor3fv(c[1]); glVertex3fv(v[1]);

		glColor3fv(c[2]); glVertex3fv(v[2]);
		glColor3fv(c[3]); glVertex3fv(v[3]);
		glColor3fv(c[7]); glVertex3fv(v[7]);
		glColor3fv(c[6]); glVertex3fv(v[6]);
		glEnd();
	}

	void cxVolume::CreateRayDirTex()
	{    
		if (m_nTexBack == 0)
			glGenTextures(1, (GLuint*)&m_nTexBack);

		int width = m_pVisView->w();
		int height = m_pVisView->h();



		//Back face    
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		DrawCube();
		glActiveTexture( GL_TEXTURE1 );    
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_nTexBack);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);    
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB , 0, 0, width, height, 0);    

		//     int precision;    
		//     glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &precision);    
		//     cout << "precision " << precision << endl;

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void cxVolume::CreateTfTex()
	{
		if (m_nTexTF == 0)
			glGenTextures(1, (GLuint*)&m_nTexTF);


		glActiveTextureARB( GL_TEXTURE2_ARB );
		glEnable(GL_TEXTURE_1D);
		glBindTexture(GL_TEXTURE_1D, m_nTexTF);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);    

		static float * spacingAdjustedTF = new float[TF_SIZE*4];

		for(int i = 0;i < TF_SIZE; i++)
		{
			float alpha = m_pVisStatus->m_Colors[i].a();

			if (m_pVisView->m_bOperating)
				alpha = 1 - pow((1-alpha), 0.01f / m_fNormalSpacing );
			else
				alpha = 1 - pow((1-alpha), m_pVisStatus->m_fSampleSpacing / m_fNormalSpacing );

			spacingAdjustedTF[i*4+0] = m_pVisStatus->m_Colors[i].r() * alpha;
			spacingAdjustedTF[i*4+1] = m_pVisStatus->m_Colors[i].g() * alpha;
			spacingAdjustedTF[i*4+2] = m_pVisStatus->m_Colors[i].b() * alpha;
			spacingAdjustedTF[i*4+3] = alpha;
		}

		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA_FLOAT16_ATI, TF_SIZE, 0, GL_RGBA, GL_FLOAT, spacingAdjustedTF);

		glDisable(GL_TEXTURE_1D);

	}
	/*
	void cxVolume::CreateColorMapTex()
	{
	if (m_nTexColorMap == 0)
	glGenTextures(1, (GLuint*)&m_nTexColorMap);

	glActiveTexture(GL_TEXTURE3);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_nTexColorMap);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);        
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);       

	int factor = 4;

	int width = TF_SIZE / factor;

	int height = width * 2;

	static float * TF = new float[width * height * 3];

	memset(TF, 0, sizeof(float) * width * height * 3);

	// color 
	for (int x = 0; x < width; x++) {
	float r = m_pVisStatus->m_Colors[x * factor].r();
	float g = m_pVisStatus->m_Colors[x * factor].g();
	float b = m_pVisStatus->m_Colors[x * factor].b();
	float a = m_pVisStatus->m_Colors[x * factor].a();

	#ifdef SHOWOPACITY
	#define OPACITY_HIGH (2.0/3.0)
	#else
	#define OPACITY_HIGH 0
	#endif

	for (int y = height * OPACITY_HIGH; y < height - 1; y++) {
	int id = x + y * width;

	TF[id * 3 + 0] = r;
	TF[id * 3 + 1] = g;
	TF[id * 3 + 2] = b;
	}


	for (int y = 0; y < height * OPACITY_HIGH * a; y++) {
	int id = x + y * width;

	TF[id * 3 + 0] = 1;
	TF[id * 3 + 1] = 1;
	TF[id * 3 + 2] = 1;

	}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA_FLOAT16_ATI, width, height, 0, GL_RGB, GL_FLOAT, TF);

	glDisable(GL_TEXTURE_2D);
	}
	*/

	void cxVolume::CreateColorMapTex()
	{
		if (m_nTexColorMap == 0)
			glGenTextures(1, (GLuint*)&m_nTexColorMap);

		glActiveTexture(GL_TEXTURE3);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_nTexColorMap);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);        
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);       

		int factor = 4;

		int width = TF_SIZE / factor;

		int height = width * 2;

		static float * TF = new float[width * height * 3];

		memset(TF, 0, sizeof(float) * width * height * 3);

		// color 
		for (int x = 0; x < width; x++) {
			float r = m_pVisStatus->m_Colors[x * factor].r();
			float g = m_pVisStatus->m_Colors[x * factor].g();
			float b = m_pVisStatus->m_Colors[x * factor].b();
			float a = m_pVisStatus->m_Colors[x * factor].a();

#ifdef SHOWOPACITY
#define OPACITY_HIGH (2.0/3.0)
#else
#define OPACITY_HIGH 0
#endif

			for (int y = height * OPACITY_HIGH; y < height - 1; y++) {
				int id = x + y * width;

				TF[id * 3 + 0] = r;
				TF[id * 3 + 1] = g;
				TF[id * 3 + 2] = b;
			}


			for (int y = 0; y < height * OPACITY_HIGH * a; y++) {
				int id = x + y * width;

				TF[id * 3 + 0] = 1;
				TF[id * 3 + 1] = 1;
				TF[id * 3 + 2] = 1;

			}
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA_FLOAT16_ATI, width, height, 0, GL_RGB, GL_FLOAT, TF);

		glDisable(GL_TEXTURE_2D);
	}

	void cxVolume::CreateGridTex()
	{
		static float *GridMap = new float[GRID_SAMPLE];
		float prevGridSpace;
		// create texture for each axis 
		for (int axis = 0; axis < 3; axis++) {        
			int size = m_vSize[axis];
#ifdef POINT_CLOUD
			m_GridSpace[axis] = new int[size];
			prevGridSpace = 0.0;
#endif
			int start=0;
			int end =0;
			// fill the look up table
			for (int i = 0; i < size - 1; i++) {
				if(axis <= 1){
					start = m_pGrid[axis][i] * GRID_SAMPLE;            
					end = m_pGrid[axis][i+1] * GRID_SAMPLE;
				}else{
					start = ((float)i/size) * GRID_SAMPLE;            
					end = ((float)(i+1)/size) * GRID_SAMPLE;
					//printf("axis=%d, start=%d, end=%d, i=%d\n",axis, start, end,i);
				}

				float start_v = (float) i / size;            
				float end_v = (float) (i+1) / size;

				for (int j = start; j <= end; j++) {

					float factor = ((float) (j - start)) / (end - start);
					float value = start_v * (1-factor) + end_v * (factor);

					GridMap[j] = value;
					//printf("size - 1= %d, axis=%d, i=%d, GridMap[%d]=%f\n",size - 1,axis,i,j,GridMap[j]);

				}

				//m_GridSpace[axis][i]=end-start+1;
#ifdef POINT_CLOUD
				m_GridSpace[axis][i] = end - start + 1 + prevGridSpace;
				prevGridSpace = m_GridSpace[axis][i];
#endif
				//printf("start=%d, end=%d, size=%d, axis=%d, i=%d\n",start,end, size,axis,i);
			}
#ifdef POINT_CLOUD
			m_GridSpace[axis][size-1] = GRID_SAMPLE;
#endif
			// create the texture
			if (m_nTexGrid[axis] == 0)
				glGenTextures(1, (GLuint*)&(m_nTexGrid[axis]));


			glActiveTextureARB( GL_TEXTURE3+axis );
			glEnable(GL_TEXTURE_1D);
			glBindTexture(GL_TEXTURE_1D, m_nTexGrid[axis]);

			glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);    

			glTexImage1D(GL_TEXTURE_1D, 0, GL_LUMINANCE16, GRID_SAMPLE, 0, GL_LUMINANCE, GL_FLOAT, GridMap);

			glDisable(GL_TEXTURE_1D);        
		}
	}


	void cxVolume::DrawVolume()
	{
		glDisable(GL_DEPTH_TEST);

		//CreateRayDirTex();
		CreateTfTex();
		if(m_bCorrelation)
			CreateDataTexPearson();
		else{
			CreateDataTex();
			//CreateIndexDataTex();
		}

#ifdef SHOW_CLUSTER
		CreateClusterTex();
#endif

#ifdef SV_CORRELATION
		CreateCorrTex();
#endif
		//CreateGridTex();

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA );


		glUseProgram(m_nVolProgram);    
		glUniform1i(glGetUniformLocation(m_nVolProgram, "grid"), 1);

		glUniform1i(glGetUniformLocation(m_nVolProgram, "volumeTex"), 0);    
		//glUniform1i(glGetUniformLocation(m_nVolProgram, "backTex"), 1);
		glUniform1i(glGetUniformLocation(m_nVolProgram, "tfTex"), 2);
		glUniform1i(glGetUniformLocation(m_nVolProgram, "gridTexX"), 3);
		glUniform1i(glGetUniformLocation(m_nVolProgram, "gridTexY"), 4);
		glUniform1i(glGetUniformLocation(m_nVolProgram, "gridTexZ"), 5);
		//glUniform1i(glGetUniformLocation(m_nVolProgram, "volumeIndTex"),6);
#ifdef SV_CORRELATION
		glUniform1i(glGetUniformLocation(m_nVolProgram, "mapCorr"), 1);
#endif

#ifdef SHOW_CLUSTER

		glUniform1i(glGetUniformLocation(m_nVolProgram, "volumeClusTex"),7);

		if(m_pClusView->isInit()){
			glUniform1i(glGetUniformLocation(m_nVolProgram, "clusterId"), m_pClusView->currCluster());
			glUniform1i(glGetUniformLocation(m_nVolProgram, "xblock"), m_pClusView->m_xblock);
			glUniform1i(glGetUniformLocation(m_nVolProgram, "yblock"), m_pClusView->m_yblock);
			glUniform1i(glGetUniformLocation(m_nVolProgram, "zblock"), m_pClusView->m_zblock);
		}
		else{  //dummy variable indicating that clustering is daisabled
			glUniform1i(glGetUniformLocation(m_nVolProgram, "clusterId"), 100); 
			glUniform1i(glGetUniformLocation(m_nVolProgram, "xblock"), 1);
			glUniform1i(glGetUniformLocation(m_nVolProgram, "yblock"), 1);
			glUniform1i(glGetUniformLocation(m_nVolProgram, "zblock"), 1);
		}

#endif


		if (m_bCorrelation)
			glUniform1i(glGetUniformLocation(m_nVolProgram, "pearsonCorrelationActive"), 1);
		else
			glUniform1i(glGetUniformLocation(m_nVolProgram, "pearsonCorrelationActive"), 0);

		if (m_bPearsonVariable)
			glUniform1i(glGetUniformLocation(m_nVolProgram, "pearsonVariable"), 1);
		else
			glUniform1i(glGetUniformLocation(m_nVolProgram, "pearsonVariable"), 0);


		glUniform1f(glGetUniformLocation(m_nVolProgram, "referenceX"), m_corrx/*m_pVisStatus->m_fXLocation*/);
		glUniform1f(glGetUniformLocation(m_nVolProgram, "referenceY"), m_corry/*m_pVisStatus->m_fYLocation*/);
		glUniform1f(glGetUniformLocation(m_nVolProgram, "referenceZ"), m_corrz/*m_pVisStatus->m_fZLocation*/);

		glUniform1f(glGetUniformLocation(m_nVolProgram, "offsetX"), m_fOffsetX);

		//glUniform1f(glGetUniformLocation(m_nVolProgram, "screenWidth"), width);
		//glUniform1f(glGetUniformLocation(m_nVolProgram, "screenHeight"), height);

		if (m_pVisView->m_bOperating)
			glUniform1f(glGetUniformLocation(m_nVolProgram, "sampleSpacing"),  0.01);
		else
			glUniform1f(glGetUniformLocation(m_nVolProgram, "sampleSpacing"),  m_pVisStatus->m_fSampleSpacing);

		glUniform4f(glGetUniformLocation(m_nVolProgram, "lightPar"), 
			m_pVisStatus->m_fLightPar[0],  m_pVisStatus->m_fLightPar[1], 
			m_pVisStatus->m_fLightPar[2],  m_pVisStatus->m_fLightPar[3]);
		glUniform3f(glGetUniformLocation(m_nVolProgram, "eyePos"), m_vEye[0], m_vEye[1], m_vEye[2]);

		float centralDifferenceSpacing[3];    
		for(int i = 0; i < 3; i++)
			centralDifferenceSpacing[i] = 1.0 / m_vSize[i];

		glUniform3f(glGetUniformLocation(m_nVolProgram, "centralDifferenceSpacing"), 
			centralDifferenceSpacing[0],centralDifferenceSpacing[1],centralDifferenceSpacing[2]);    

		glUniform1i(glGetUniformLocation(m_nVolProgram, "drawSlice"), 0);

#ifdef ORTHO    
		glUniform3f(glGetUniformLocation(m_nVolProgram, "eyeDir"), m_vEyeDir[0], m_vEyeDir[1], m_vEyeDir[2]);
		glUniform1i(glGetUniformLocation(m_nVolProgram, "bOrtho"), 1);
#else
		glUniform3f(glGetUniformLocation(m_nVolProgram, "eyeDir"), 0,0,0);
		glUniform1i(glGetUniformLocation(m_nVolProgram, "bOrtho"), 0);
#endif    

		glUniform1f(glGetUniformLocation(m_nVolProgram, "shiftDis"),  m_pVisStatus->m_fShiftDis);

		float minx = m_pVisStatus->m_vRangeMin[0];
		float miny = m_pVisStatus->m_vRangeMin[1];
		float minz = m_pVisStatus->m_vRangeMin[2];

		float maxx = m_pVisStatus->m_vRangeMax[0];
		float maxy = m_pVisStatus->m_vRangeMax[1];
		float maxz = m_pVisStatus->m_vRangeMax[2]; 

		float t[8][3] = {{minx,miny,minz},{maxx,miny,minz},{minx,maxy,minz},{maxx,maxy,minz},
		{minx,miny,maxz},{maxx,miny,maxz},{minx,maxy,maxz},{maxx,maxy,maxz}};

		minx = minx * m_vSize[0];
		miny = miny * m_vSize[1];
		minz = minz * m_vSize[2];

		maxx = maxx * m_vSize[0];    
		maxy = maxy * m_vSize[1];
		maxz = maxz * m_vSize[2];

		float v[8][3] = {{minx,miny,minz},{maxx,miny,minz},{minx,maxy,minz},{maxx,maxy,minz},
		{minx,miny,maxz},{maxx,miny,maxz},{minx,maxy,maxz},{maxx,maxy,maxz}};


		//Front face
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		glBegin(GL_QUADS);
		glTexCoord3fv(t[0]); glVertex3fv(v[0]);
		glTexCoord3fv(t[1]); glVertex3fv(v[1]);
		glTexCoord3fv(t[3]); glVertex3fv(v[3]);
		glTexCoord3fv(t[2]); glVertex3fv(v[2]);

		glTexCoord3fv(t[1]); glVertex3fv(v[1]);
		glTexCoord3fv(t[5]); glVertex3fv(v[5]);
		glTexCoord3fv(t[7]); glVertex3fv(v[7]);
		glTexCoord3fv(t[3]); glVertex3fv(v[3]);

		glTexCoord3fv(t[4]); glVertex3fv(v[4]);
		glTexCoord3fv(t[6]); glVertex3fv(v[6]);
		glTexCoord3fv(t[7]); glVertex3fv(v[7]);
		glTexCoord3fv(t[5]); glVertex3fv(v[5]);

		glTexCoord3fv(t[0]); glVertex3fv(v[0]);
		glTexCoord3fv(t[2]); glVertex3fv(v[2]);
		glTexCoord3fv(t[6]); glVertex3fv(v[6]);
		glTexCoord3fv(t[4]); glVertex3fv(v[4]);

		glTexCoord3fv(t[0]); glVertex3fv(v[0]);
		glTexCoord3fv(t[4]); glVertex3fv(v[4]);
		glTexCoord3fv(t[5]); glVertex3fv(v[5]);
		glTexCoord3fv(t[1]); glVertex3fv(v[1]);

		glTexCoord3fv(t[2]); glVertex3fv(v[2]);
		glTexCoord3fv(t[3]); glVertex3fv(v[3]);
		glTexCoord3fv(t[7]); glVertex3fv(v[7]);
		glTexCoord3fv(t[6]); glVertex3fv(v[6]);
		glEnd();      

		glDisable(GL_CULL_FACE);

		TurnOffTextureAndPrograms();

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}


	void cxVolume::Draw2DSlice(int nSliceAxis, int h, int w)
	{
		CreateTfTex();
		if(m_bCorrelation)
			CreateDataTexPearson();
		else{	
			CreateDataTex();
			//CreateIndexDataTex();
		}
#ifdef SHOW_CLUSTER
		CreateClusterTex();
#endif

#ifdef SV_CORRELATION
		CreateCorrTex();
#endif
		//CreateGridTex();

		//float width = m_pVisView->w();
		//float height = m_pVisView->h();
		float slicewidth = m_pSliceView[nSliceAxis - SLICE_X_AXIS]->w();
		float sliceheight = m_pSliceView[nSliceAxis - SLICE_X_AXIS]->h();

		int buffersize = slicewidth * sliceheight;
		float * buffer = new float[buffersize];

		glUseProgram(m_nVolProgram);    
		glUniform1i(glGetUniformLocation(m_nVolProgram, "grid"), 1);
		glUniform1i(glGetUniformLocation(m_nVolProgram, "volumeTex"), 0);    
		//glUniform1i(glGetUniformLocation(m_nVolProgram, "backTex"), 1);   
		glUniform1i(glGetUniformLocation(m_nVolProgram, "tfTex"), 2);
		glUniform1i(glGetUniformLocation(m_nVolProgram, "gridTexX"), 3);
		glUniform1i(glGetUniformLocation(m_nVolProgram, "gridTexY"), 4);
		glUniform1i(glGetUniformLocation(m_nVolProgram, "gridTexZ"), 5);

//#ifdef SV_CORRELATION
//		glUniform1i(glGetUniformLocation(m_nVolProgram, "mapCorr"), 1);
//#endif

#ifdef SHOW_CLUSTER

		glUniform1i(glGetUniformLocation(m_nVolProgram, "volumeClusTex"),7);

		if(m_pClusView->isInit()){
			glUniform1f(glGetUniformLocation(m_nVolProgram, "clusterId"), m_pClusView->currCluster());
			glUniform1f(glGetUniformLocation(m_nVolProgram, "xblock"), m_pClusView->m_xblock);
			glUniform1f(glGetUniformLocation(m_nVolProgram, "yblock"), m_pClusView->m_yblock);
			glUniform1f(glGetUniformLocation(m_nVolProgram, "zblock"), m_pClusView->m_zblock);
		}
		else{  //dummy variable indicating that clustering is daisabled
			glUniform1f(glGetUniformLocation(m_nVolProgram, "clusterId"), 100); 
			glUniform1f(glGetUniformLocation(m_nVolProgram, "xblock"), 1);
			glUniform1f(glGetUniformLocation(m_nVolProgram, "yblock"), 1);
			glUniform1f(glGetUniformLocation(m_nVolProgram, "zblock"), 1);
		}

#endif
		if (m_bCorrelation)
			glUniform1i(glGetUniformLocation(m_nVolProgram, "pearsonCorrelationActive"), 1);
		else
			glUniform1i(glGetUniformLocation(m_nVolProgram, "pearsonCorrelationActive"), 0);

		if (m_bPearsonVariable)
			glUniform1i(glGetUniformLocation(m_nVolProgram, "pearsonVariable"), 1);
		else
			glUniform1i(glGetUniformLocation(m_nVolProgram, "pearsonVariable"), 0);


		glUniform1f(glGetUniformLocation(m_nVolProgram, "referenceX"), m_corrx/*m_pVisStatus->m_fXLocation*/);
		glUniform1f(glGetUniformLocation(m_nVolProgram, "referenceY"), m_corry/*m_pVisStatus->m_fYLocation*/);
		glUniform1f(glGetUniformLocation(m_nVolProgram, "referenceZ"), m_corrz/*m_pVisStatus->m_fZLocation*/);

		glUniform1f(glGetUniformLocation(m_nVolProgram, "offsetX"), m_fOffsetX);

		//glUniform1f(glGetUniformLocation(m_nVolProgram, "screenWidth"), width);
		//glUniform1f(glGetUniformLocation(m_nVolProgram, "screenHeight"), height);
		glUniform1f(glGetUniformLocation(m_nVolProgram, "sampleSpacing"), m_pVisStatus->m_fSampleSpacing);
		glUniform4f(glGetUniformLocation(m_nVolProgram, "lightPar"), 
			m_pVisStatus->m_fLightPar[0], m_pVisStatus->m_fLightPar[1], 
			m_pVisStatus->m_fLightPar[2], m_pVisStatus->m_fLightPar[3]);
		glUniform3f(glGetUniformLocation(m_nVolProgram, "eyePos"), m_vEye[0], m_vEye[1], m_vEye[2]);

		float centralDifferenceSpacing[3];    
		for(int i = 0; i < 3; i++)
			centralDifferenceSpacing[i] = 1.0 / m_vSize[i];

		glUniform3f(glGetUniformLocation(m_nVolProgram, "centralDifferenceSpacing"), 
			centralDifferenceSpacing[0],centralDifferenceSpacing[1],centralDifferenceSpacing[2]);    

		glUniform1i(glGetUniformLocation(m_nVolProgram, "drawSlice"), 2);
		float x, y, z, max;


		if (nSliceAxis == SLICE_X_AXIS) {
			max = m_vSize[1] > m_vSize[2] ? m_vSize[1] : m_vSize[2];        
			y = 0.75;//m_vSize[1] / max;
			z = 0.75;//m_vSize[2] / max;

			//float x_pos = 0.5 + m_pVisStatus->m_fSlicePosX * 0.5;
			int x_slice = floor((m_pVisStatus->m_fSlicePosX + 1.0)*0.5 * (m_vSize[0] - 1));
			float x_pos = x_slice / (m_vSize[0]);

			glBegin(GL_QUADS);
			glTexCoord3f(x_pos, 0, 0);
			glVertex2f(-y, -z);
			glTexCoord3f(x_pos, 1, 0);
			glVertex2f(y, -z);
			glTexCoord3f(x_pos, 1, 1);
			glVertex2f(y, z);
			glTexCoord3f(x_pos, 0, 1);
			glVertex2f(-y, z);
			glEnd();

		}

		if (nSliceAxis == SLICE_Y_AXIS) {
			max = m_vSize[0] > m_vSize[2] ? m_vSize[0] : m_vSize[2];        
			x = 0.75; //m_vSize[0] / max;
			z = 0.75; //m_vSize[2] / max;

			float y_pos = 0.5 + m_pVisStatus->m_fSlicePosY * 0.5;
			glBegin(GL_QUADS);
			glTexCoord3f(0, y_pos, 0);
			glVertex2f(-x, -z);
			glTexCoord3f(1, y_pos, 0);
			glVertex2f(x, -z);
			glTexCoord3f(1, y_pos, 1);
			glVertex2f(x, z);
			glTexCoord3f(0, y_pos, 1);
			glVertex2f(-x, z);
			glEnd();

		}


		if (nSliceAxis == SLICE_Z_AXIS) {
			max = m_vSize[0] > m_vSize[1] ? m_vSize[0] : m_vSize[1];        
			x = 0.75; //m_vSize[0] / max;
			y = 0.75; //m_vSize[1] / max;

			float z_pos = 0.5 + m_pVisStatus->m_fSlicePosZ * 0.5;
			glBegin(GL_QUADS);
			glTexCoord3f(0, 0, z_pos);
			glVertex2f(-x, -y);
			glTexCoord3f(1, 0, z_pos);
			glVertex2f(x, -y);
			glTexCoord3f(1, 1, z_pos);
			glVertex2f(x, y);
			glTexCoord3f(0, 1, z_pos);
			glVertex2f(-x, y);
			glEnd();
		}

		glFinish();
		glReadPixels(0, 0, slicewidth, sliceheight, GL_RED, GL_FLOAT, buffer); 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if(m_pCoord) m_pCoord->SetSelectedData(buffer, sliceheight, slicewidth);

		/*
		for (int i = 0; i < width * height; i++) {
		if (buffer[i] > 0)
		cout << "$$$ " << buffer[i] << endl;
		}
		*/

		if(IsEnabledColorContours()){	
			glUniform1i(glGetUniformLocation(m_nVolProgram, "drawSlice"), 1);



			if (nSliceAxis == SLICE_X_AXIS) {
				max = m_vSize[1] > m_vSize[2] ? m_vSize[1] : m_vSize[2];        
				y = 0.75;//m_vSize[1] / max;
				z = 0.75;//m_vSize[2] / max;

				//float x_pos = 0.5 + m_pVisStatus->m_fSlicePosX * 0.5;
				int x_slice = floor((m_pVisStatus->m_fSlicePosX + 1.0)*0.5 * (m_vSize[0] - 1));
				float x_pos = x_slice / (m_vSize[0]);

				glBegin(GL_QUADS);
				glTexCoord3f(x_pos, 0, 0);
				glVertex2f(-y, -z);
				glTexCoord3f(x_pos, 1, 0);
				glVertex2f(y, -z);
				glTexCoord3f(x_pos, 1, 1);
				glVertex2f(y, z);
				glTexCoord3f(x_pos, 0, 1);
				glVertex2f(-y, z);
				glEnd();

			}

			if (nSliceAxis == SLICE_Y_AXIS) {
				max = m_vSize[0] > m_vSize[2] ? m_vSize[0] : m_vSize[2];        
				x = 0.75; //m_vSize[0] / max;
				z = 0.75; //m_vSize[2] / max;

				float y_pos = 0.5 + m_pVisStatus->m_fSlicePosY * 0.5;
				glBegin(GL_QUADS);
				glTexCoord3f(0, y_pos, 0);
				glVertex2f(-x, -z);
				glTexCoord3f(1, y_pos, 0);
				glVertex2f(x, -z);
				glTexCoord3f(1, y_pos, 1);
				glVertex2f(x, z);
				glTexCoord3f(0, y_pos, 1);
				glVertex2f(-x, z);
				glEnd();

			}


			if (nSliceAxis == SLICE_Z_AXIS) {
				max = m_vSize[0] > m_vSize[1] ? m_vSize[0] : m_vSize[1];        
				x = 0.75; //m_vSize[0] / max;
				y = 0.75; //m_vSize[1] / max;

				float z_pos = 0.5 + m_pVisStatus->m_fSlicePosZ * 0.5;
				glBegin(GL_QUADS);
				glTexCoord3f(0, 0, z_pos);
				glVertex2f(-x, -y);
				glTexCoord3f(1, 0, z_pos);
				glVertex2f(x, -y);
				glTexCoord3f(1, 1, z_pos);
				glVertex2f(x, y);
				glTexCoord3f(0, 1, z_pos);
				glVertex2f(-x, y);
				glEnd();
			}

		}

		TurnOffTextureAndPrograms();

		if(m_bContours && m_ms){
			//m_ms->compute_for_slice(m_vSize[0], m_vSize[1], m_vSize[2], m_pVolume, SLICE_X_AXIS, m_pVisStatus->m_fSlicePosX);
			m_ms->compute_for_texture(buffer, sliceheight, slicewidth);
		}


		//glFinish();
		//glReadPixels(0, 0, width, height, GL_RED, GL_FLOAT, buffer); 

		delete buffer;

	}

	void cxVolume::Draw2DSliceBoundary()
	{
		glPushMatrix();

		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glLineWidth(2.0);

		glColor3f(1.0, 1.0, 0.0);

		float x = m_vSize[0];
		float y = m_vSize[1];
		float z = m_vSize[2];

		if (m_nSliceAxis == SLICE_X_AXIS) {
			x = (m_pVisStatus->m_fSlicePosX + 1 ) * 0.5 * m_vSize[0];

			glBegin(GL_LINE_LOOP);
			glVertex3f(x, 0, 0);
			glVertex3f(x, y, 0);
			glVertex3f(x, y, z);
			glVertex3f(x, 0, z);
			glEnd();
		}

		if (m_nSliceAxis == SLICE_Y_AXIS) {
			y = (m_pVisStatus->m_fSlicePosY + 1 ) * 0.5 * m_vSize[1];

			glBegin(GL_LINE_LOOP);
			glVertex3f(0, y, 0);
			glVertex3f(x, y, 0);
			glVertex3f(x, y, z);
			glVertex3f(0, y, z);
			glEnd();
		}

		if (m_nSliceAxis == SLICE_Z_AXIS) {
			z = (m_pVisStatus->m_fSlicePosZ + 1 ) * 0.5 * m_vSize[2];

			glBegin(GL_LINE_LOOP);
			glVertex3f(0, 0, z);
			glVertex3f(x, 0, z);
			glVertex3f(x, y, z);
			glVertex3f(0, y, z);
			glEnd();
		}


		glPopMatrix();
		glEnable(GL_DEPTH_TEST);
	}

	void cxVolume::SetCorrCenterPoint(int axis, float x, float y)
	{
		assert(x<=1.0 && y <= 1.0 && x>=0 && y >= 0);
		if (axis == SLICE_X_AXIS){
			if(!m_bCorrFreeze){
				m_corrx = (m_pVisStatus->m_fSlicePosX + 1 ) * 0.5;// * m_vSize[0];
				m_corry = x;// * m_vSize[1];
				m_corrz = 1 - y;// * m_vSize[2];
			}
		}
		else if (axis == SLICE_Y_AXIS){
			if(!m_bCorrFreeze){
				m_corry = (m_pVisStatus->m_fSlicePosY + 1 )* 0.5;// * m_vSize[1];
				m_corrx = x;// * m_vSize[0];
				m_corrz = 1 - y;// * m_vSize[2];

				m_corrx += m_fOffsetX;
				if(m_corrx < 0) m_corrx += 1.0;
				else if(m_corrx > 1.0) m_corrx -= 1.0; 
			}
		}   
		else if (axis == SLICE_Z_AXIS){
			if(!m_bCorrFreeze){
				m_corrz = (m_pVisStatus->m_fSlicePosZ + 1 )* 0.5;// * m_vSize[2];
				m_corrx = x;// * m_vSize[0];
				m_corry = y;// * m_vSize[1];

				m_corrx += m_fOffsetX;
				if(m_corrx < 0) m_corrx += 1.0;
				else if(m_corrx > 1.0) m_corrx -= 1.0; 
			}
		}


		//fprintf(stderr,"X=%f, Y=%f, Z=%f\n",m_corrx,m_corry,m_corrz);
		ReDraw(true);
	}
	/*
	int x_start = 0;
	int x_end = m_vSize[0];
	int y_start = 0;
	int y_end = m_vSize[1];
	int z_start = 0;
	int z_end = m_vSize[2];
	int total_size = m_vSize[0]*m_vSize[1]*m_vSize[2];  // do not forget to divide by the size of the block
	// no need to do that right now because 1 block = 1 voxel

	if(m_corrdata){
	for(int i=0; i < total_size; i++)
	delete [] m_corrdata[i];
	delete [] m_corrdata;
	}

	m_corrdata = new float*[total_size];
	for(int i=0; i < total_size; i++)
	m_corrdata[i] = new float[time_count];


	if(m_corrdata){
	if(!m_netcdf)
	return;
	NetCFD_var* var_info = m_netcdf->get_varinfo(m_curnetcdf_var);
	Filter* fltr = new Filter();
	fltr->m_dim = 4;
	NetCDFUtils* datautils = new NetCDFUtils(m_netcdf, fltr);
	datautils->Get_subsetTACs(	x_start, x_end, 
	y_start, y_end, 
	z_start, z_end,
	var_info->var_name, 
	time_in, time_count, 
	1,1,1,
	m_corrdata);
	}

	if(m_corrmap)
	delete [] m_corrmap;
	m_corrmap = new float[total_size];


	//int count = 0;
	float val;
	int xblocksize = 1;
	int yblocksize = 1;
	int zblocksize = 1;
	FILE *out = fopen("d:\\jeffs\\write_debug.txt", "w");
	int id_center = m_corrx + m_corry * m_vSize[0] + m_corrz * m_vSize[0] * m_vSize[1];
	for(int x1=0; x1 < m_vSize[0]-xblocksize+1; x1+=xblocksize){
	for(int y1=0; y1 < m_vSize[1]-yblocksize+1; y1+=yblocksize){
	for(int z1=0; z1 < m_vSize[2]-zblocksize+1; z1+=zblocksize){

	for(int x = x1; x< (x1+xblocksize); x++){
	for(int y = y1; y<(y1+yblocksize); y++){
	for(int z = z1; z<(z1+zblocksize); z++){
	int id = x + y * m_vSize[0] + z * m_vSize[0] * m_vSize[1];
	if(	x >= x_start && x < x_end && 
	y >= y_start && y < y_end &&
	z >= z_start && z < z_end){
	CalcSVCorrelation(m_corrdata[id_center], m_corrdata[id], time_count, val);
	fprintf(out,"%f\n",val);
	val = (val + 1.0)/2.0;
	m_corrmap[id] = val;
	}
	else
	m_corrmap[id] = 0.0;
	}
	}
	}

	}
	}
	}
	fclose(out);

	#ifdef SV_CORRELATION
	if (m_nTexCorr != 0) {
	glDeleteTextures(1, (const GLuint*)&m_nTexCorr);
	m_nTexCorr = 0;
	}
	CreateCorrTex();
	#endif


	}
	*/

	int cxVolume::GetSizeSlice(int axis, char dim)
	{
		int len = 0;

		if (axis == SLICE_X_AXIS) {
			if(dim == 'x')
				len = m_vSize[2];
			else //(dim == 'y')
				len = m_vSize[1];
		}
		else if (axis == SLICE_Y_AXIS) {
			if(dim == 'x')
				len = m_vSize[0];
			else //(dim == 'y')
				len = m_vSize[2];
		}   
		else if (axis == SLICE_Z_AXIS) {
			if(dim == 'x')
				len = m_vSize[0];
			else //(dim == 'y')
				len = m_vSize[1];
		}
		return len;
	}

#define X_SHIFT 54
	// draw selected rectangle on top of the 2D slice
	void cxVolume::SetSelected2DRegion(int axis, float x1, float y1, float x2, float y2)
	{
		int temp = 0;
		if(x1 > x2) {
			temp = x1;
			x1 = x2;
			x2 = temp;
		}
		if(y1 > y2) {
			temp = y1;
			y1 = y2;
			y2 = temp;
		}

		if (axis == SLICE_X_AXIS) {
			if(m_vSize[0] == 0){
				m_selectx1 = 0;
				m_selectx2 = 0;
			}
			else{
				m_selectx1 = (m_pVisStatus->m_fSlicePosX + 1 ) * 0.5 * (m_vSize[0]-1);
				m_selectx2 = (m_pVisStatus->m_fSlicePosX + 1 ) * 0.5 * (m_vSize[0]-1);

			}

			if(m_vSize[1] == 0){
				m_selecty1 = 0;
				m_selecty2 = 0;
			}
			else{
				m_selecty1 = y1 * (m_vSize[1]-1);
				m_selecty2 = y2 * (m_vSize[1]-1);
			}

			if(m_vSize[2] == 0){
				m_selectz1 = 0;
				m_selectz2 = 0;
			}
			else{
				m_selectz1 = x1 * (m_vSize[2]-1);
				m_selectz2 = x2 * (m_vSize[2]-1);
			}

			//printf("m_selecty1=%d, m_selecty2=%d, m_selectz1=%d, m_selectz2=%d\n",m_selecty1,m_selecty2,m_selectz1,m_selectz2);
		}

		else if (axis == SLICE_Y_AXIS) {
			if(m_vSize[1] == 0){
				m_selecty1 = 0;
				m_selecty2 = 0;
			}
			else{
				m_selecty1 = (m_pVisStatus->m_fSlicePosY + 1 ) * 0.5 * (m_vSize[1]-1);
				m_selecty2 = (m_pVisStatus->m_fSlicePosY + 1 ) * 0.5 * (m_vSize[1]-1);
			}

			if(m_vSize[0] == 0){
				m_selectx1 = 0;
				m_selectx2 = 0;
			}
			else{
				m_selectx1 = x1 * (m_vSize[0]-1);
				m_selectx2 = x2 * (m_vSize[0]-1);
			}

			if(m_vSize[2] == 0){
				m_selectz1 = 0;
				m_selectz2 = 0;
			}
			else{
				m_selectz1 = y1 * (m_vSize[2]-1);
				m_selectz2 = y2 * (m_vSize[2]-1);
			}
			//printf("m_selectx1=%d, m_selectx2=%d, m_selectz1=%d, m_selectz2=%d\n",m_selectx1,m_selectx2,m_selectz1,m_selectz2);
		}   
		else if (axis == SLICE_Z_AXIS) {
			if(m_vSize[2] == 0){
				m_selectz1 = 0;
				m_selectz2 = 0;
			}
			else{
				m_selectz1 = (m_pVisStatus->m_fSlicePosZ + 1 ) * 0.5 * (m_vSize[2]-1);
				m_selectz2 = (m_pVisStatus->m_fSlicePosZ + 1 ) * 0.5 * (m_vSize[2]-1);
			}

			if(m_vSize[0] == 0){
				m_selectx1 = 0;
				m_selectx2 = 0;
			}
			else{
				m_selectx1 = x1 * (m_vSize[0]-1);// - X_SHIFT;
				//if(m_selectx1 < 0){
				//	m_selectx1 = 0;
				//}
				m_selectx2 = x2 * (m_vSize[0]-1);// - X_SHIFT;
				//if(m_selectx2 < 0){
				//	m_selectx2 = 0;
				//}

			}

			if(m_vSize[1] == 0){
				m_selecty1 = 0;
				m_selecty2 = 0;
			}
			else{
				m_selecty1 = y1 * (m_vSize[1]-1);
				m_selecty2 = y2 * (m_vSize[1]-1);
			}
			//printf("m_selectx1=%d, m_selectx2=%d, m_selecty1=%d, m_selecty2=%d\n",m_selectx1,m_selectx2,m_selecty1,m_selecty2);
		}
		swapvalues(m_selectx1,m_selectx2);
		swapvalues(m_selecty1,m_selecty2);
		swapvalues(m_selectz1,m_selectz2);
		m_selection = true;
	}

	bool cxVolume::GetSelected2DRegion(int* x1, int* x2, int* y1, int* y2, int* z1, int* z2)
	{
		if(m_selection){
			*x1 = m_selectx1;
			*x2 = m_selectx2;
			*y1 = m_selecty1;
			*y2 = m_selecty2;
			*z1 = m_selectz1;
			*z2 = m_selectz2;
			return true;
		}
		else
			return false;
	}

	void cxVolume::EnableSelection(bool selection) 
	{
		m_selection = selection;
		if(m_pCoord) m_pCoord->redraw();
	}

	bool cxVolume::IsSelected2DRegion()
	{ 
		return m_selection;
	}

	void cxVolume::Discard2DRegion()
	{
		m_selection = false;
	}

	void cxVolume::SetTimePeriod(int time_in, int time_out)
	{
		m_time_in = time_in;
		m_time_out = time_out;
		//if(m_ms)
		//	delete m_ms;
		if(!m_bCorrelation){
			Read(m_curnetcdf_var);
		}else{
			assert(m_time_in < 1200-m_nNumTimePearson);
			Read(m_bCorrItem1,m_bCorrItem2,true);
		}
		if(m_pCoord)
			m_pCoord->SetInitialTimeStep(m_time_in);
		if(m_bContours){
			m_ms->update(m_contour_value, m_contour_num_saved, m_time_in);
			//m_ms->compute(m_vSize[0], m_vSize[1], m_vSize[2], m_pVolume);  
		}
	}

	void cxVolume::SetCurVar(char* varname)
	{ 
		strcpy(m_curnetcdf_var, varname);
	}

	void cxVolume::SetYstop(int size) 
	{ 
		m_stopIndY  = size + m_startIndY; 
		m_IndYsize = size;
	}
	void cxVolume::SetXstop(int size) 
	{ 
		m_stopIndX  = size + m_startIndX; 
		m_IndXsize = size;
	} 

	void cxVolume::SetYstart(int start) 
	{ 
		m_startIndY = start;
		m_stopIndY = m_IndYsize + start; 
	}

	void cxVolume::SetXstart(int start) 
	{ 
		m_startIndX = start;
		m_stopIndX = m_IndXsize + start; 
	}

	void cxVolume::SaveTAC_vol()
	{
		//SaveTACSubSet(  m_selectx1,m_selectx2,m_selecty1,m_selecty2,m_selectz1,m_selectz2, m_TAC_start, m_TAC_finish);
	}


#define X_BLOCK_SIZE	 3
#define Y_BLOCK_SIZE	 3
#define Z_BLOCK_SIZE	 3
	void cxVolume::SaveGTAC_vol()
	{
		if(!m_netcdf)
			return;
		NetCFD_var* var_info = m_netcdf->get_varinfo(m_curnetcdf_var);
		Filter* fltr = new Filter();
		fltr->m_dim = 4;
		NetCDFUtils* datautils = new NetCDFUtils(m_netcdf, fltr);
		/*
		if (axis == SLICE_X_AXIS) {
		if(m_selectx1 == m_selectx2)
		m_selectx2++;
		}
		else if (axis == SLICE_Y_AXIS) {
		if(m_selecty1 == m_selecty2)
		m_selecty2++;	
		}   
		else if (axis == SLICE_Z_AXIS) {
		if(m_selectz1 == m_selectz2)
		m_selectz2++;
		}
		*/
		char filename[MAX_PATH] = "c://jeffs/temp/test.dat";
		datautils->Save_volumeTACs(	0, m_vSize[0]-1, 
			0, m_vSize[1]-1,
			0, m_vSize[2]-1,
			var_info->var_name, 
			//m_TAC_start, m_TAC_finish,
			0, 30,/*m_timelen,*/ 
			X_BLOCK_SIZE, Y_BLOCK_SIZE, Z_BLOCK_SIZE, filename);


		int nparams = 11;
		char** params;
		params = new char*[12];
		for(int i=0; i < nparams; i++)
			params[i] = new char[128];
		strcpy(params[0],"");
		strcpy(params[1],"-t");			// algorithm def: 0
		strcpy(params[2],"0");
		strcpy(params[3],"-tch");		// number of timestamps used in calculating KMeans def: 0 (All timestamps)
		strcpy(params[4],"10");
		strcpy(params[5],"-tac");		// saved tac filename
		strcpy(params[6],filename);
		strcpy(params[7],"-k");			// number of cluster centers def: 4
		strcpy(params[8],"3");
		strcpy(params[9],"-s");		    // number of stages def: 1000
		strcpy(params[10],"300");
		//main_cluster(nparams, params);

		for(int i=0; i < nparams; i++)
			delete [] params[i];
		delete [] params;

		delete datautils;
		delete fltr;


	}

	void cxVolume::SaveCustomTAC_vol(char* filename, int start, int count)
	{
		if(!m_netcdf)
			return;
		NetCFD_var* var_info = m_netcdf->get_varinfo(m_curnetcdf_var);
		Filter* fltr = new Filter();
		fltr->m_dim = 4;
		NetCDFUtils* datautils = new NetCDFUtils(m_netcdf, fltr);

		datautils->Save_volumeTACs(	0, m_vSize[0]-1, 
			0, m_vSize[1]-1,
			0, m_vSize[2]-1,
			var_info->var_name, 
			start, count,
			X_BLOCK_SIZE, Y_BLOCK_SIZE, Z_BLOCK_SIZE,
			filename);
		delete datautils;
		delete fltr;
	}
	/*
	void cxVolume::SaveTACSubSet(int x_start, int x_end, 
	int y_start, int y_end, 
	int z_start, int z_end,
	int time_in, int time_count)
	{
	if(!m_netcdf && !IsSelected2DRegion())
	return;
	NetCFD_var* var_info = m_netcdf->get_varinfo(m_curnetcdf_var);
	Filter* fltr = new Filter();
	fltr->m_dim = 4;
	NetCDFUtils* datautils = new NetCDFUtils(m_netcdf, fltr);
	time_count--;
	if(time_count <= 0) time_count = 1;

	datautils->SaveSubset(	x_start, x_end, 
	y_start, y_end, 
	z_start, z_end,
	var_info->var_name, 
	time_in, time_count);
	delete datautils;
	delete fltr;
	}
	*/

	void cxVolume::CalcSVCorrelation(float* x, float* y, int N, float& correlation)
	{

		//The following algorithm (in pseudocode) will calculate Pearson correlation with good numerical stability
		/*
		float sum_sq_x = 0;
		float sum_sq_y = 0;
		float sum_coproduct = 0;
		float mean_x = x[1];
		float mean_y = y[1];
		for(int i = 1; i < N; i++){
		float sweep = (i - 1.0) / i;
		float delta_x = x[i] - mean_x;
		float delta_y = y[i] - mean_y;
		sum_sq_x += delta_x * delta_x * sweep;
		sum_sq_y += delta_y * delta_y * sweep;
		sum_coproduct += delta_x * delta_y * sweep;
		mean_x += delta_x / i;
		mean_y += delta_y / i;
		}
		float pop_sd_x = sqrt( sum_sq_x / N );
		float pop_sd_y = sqrt( sum_sq_y / N );
		float cov_x_y = sum_coproduct / N;

		if(pop_sd_x * pop_sd_y == 0.0)
		correlation = 0;
		else
		correlation = cov_x_y / (pop_sd_x * pop_sd_y);
		*/

		float sum_sq_x = 0;
		float sum_sq_y = 0;
		float mean_x = 0;
		float mean_y = 0;
		correlation = 0;
		int i;

		for (i=0; i<N; i++) {
			mean_x += x[i];
			mean_y += y[i];
		}
		mean_x /= N;
		mean_y /= N;

		for (i=0; i<N; i++) {
			sum_sq_x += (x[i] - mean_x) * (x[i] - mean_x);
			sum_sq_y += (y[i] - mean_y) * (y[i] - mean_y);
		}
		sum_sq_x = sqrt(sum_sq_x / N);
		sum_sq_y = sqrt(sum_sq_y / N);

		for (i=0; i<N; i++) {
			if (sum_sq_x * sum_sq_y != 0.0) 
				correlation += ((x[i] - mean_x) / sum_sq_x) * ((y[i] - mean_y) / sum_sq_y);
		}
		correlation /= N;

		if (correlation < -0.5) correlation = -1;
		else if (correlation < 0.5) correlation = 0;
		else correlation = 1;

	}


	void cxVolume::getLabels(char xlabel[], char ylabel[], char zlabel[])
	{

		float xlocation, ylocation, zlocation;
		xlocation = m_corrx;
		ylocation = m_corry;
		zlocation = m_corrz;

		xlocation = xlocation - m_fOffsetX + 0.06;
		if (xlocation<0.0) xlocation += 1.0;
		if (xlocation>1.0) xlocation -= 1.0;

		int xlab;

		if (xlocation<=0.5) {
			xlab = int(360 * xlocation);
			if (xlab==0) sprintf(xlabel, "%d", 0);
			else if (xlab==180) sprintf(xlabel, "%d", 0);
			else sprintf(xlabel, "%dE", xlab);
		}
		else {
			xlab = int(-360 * xlocation + 360);
			if (xlab==0) sprintf(xlabel, "%d", 0);
			else if (xlab==180) sprintf(xlabel, "%d", 0);
			else sprintf(xlabel, "%dW", xlab);
		}

		if (xlocation==0.0) 
			sprintf(xlabel, "%d", 0);
		else if (xlocation==0.5) 
			sprintf(xlabel, "%d", 180);

		int yvalue, sign;
		if (ylocation==0.5) 
			yvalue = 0;
		else if (ylocation<0.5) 
		{
			yvalue = int(-40 * ylocation + 20);
			sign = 0;
		}
		else 
		{
			yvalue = int(40 * ylocation - 20);
			sign = 1;
		}
		if (yvalue == 0)
			sprintf(ylabel, ", %d", yvalue);
		else if (sign == 0)
			sprintf(ylabel, ", %dS", yvalue);
		else
			sprintf(ylabel, ", %dN", yvalue);

		sprintf(zlabel, ", %dM", int(-300 * zlocation + 300));
	}

	NetCDF* cxVolume::openNetCDF_file(char* filename)
    { 
		m_netcdf = new NetCDF(filename); 
		return m_netcdf;
	}