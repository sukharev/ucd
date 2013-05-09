/*
 *  clusterview2d.cpp
 *  amrview
 *
 *  Created by Ryan Armstrong on 2/15/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */


#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#endif


//#include "controlWin.h"
#include "clusterview2d.h"
#include "cxvolume.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include <assert.h>
#include <stdio.h>


#define NODE_HEIGHT 15
#define NODE_WIDTH  20
#define COLOR_CHOOSER_WIDTH  190
#define COLOR_CHOOSER_HEIGHT 100
#define MARGIN 15
#define BUTTON_HEIGHT 25
#define BUTTON_WIDTH  100

#define MAX_NUM_CLUSTERS  6;

//bool readTACFileHeader(ifstream& inf, int *p_xsize, int *p_ysize, int *p_zsize, int *dim);
bool readTACFileHeader(ifstream& inf, int *p_xsize, 
					   int *p_ysize, int *p_zsize, 
					   int *dim, int* xblock, int *yblock, int *zblock);
bool readTACData(ifstream& inf, float** dataPts, int dim, int nPts);
bool readClusterHeader(ifstream& inf, int& ts_chunk);
bool readClusterData(ifstream& inf, int* dataPts, int nPts);
bool readClusterNum(ifstream& inf, int& nClusters);
bool readClusterCentroid(ifstream& inf, int dim, int nClusters, float** centroid = NULL);


int** AllocateDArray(int num_items, int item_size);
void DeallocateDArray(int num_items, int item_size);


ClusterView2D::ClusterView2D(int x, int y, int w, int h, int inResolution, cxVolume * pVolume) : Fl_Gl_Window(x, y, w, h-COLOR_CHOOSER_HEIGHT){

	//mode(FL_ALPHA | FL_DEPTH | FL_DOUBLE | FL_RGB8 );
	//	Fl::add_idle(&idle_cp, (void*)this);
	opacityDisp = 0;
	histogram1D = 0;
	
	m_pVolume = pVolume;
	
	m_percentHeight = (float)COLOR_CHOOSER_HEIGHT / (float) h;
	m_percentWidth = (float)COLOR_CHOOSER_WIDTH / (float) w;
	ClusterView2D *ihateptrs = this; // this is necessary because &this doesnt work.
		
	//m_buttonA = new Fl_Button(x+MARGIN+COLOR_CHOOSER_WIDTH, y+(h-COLOR_CHOOSER_HEIGHT)+MARGIN, BUTTON_WIDTH,BUTTON_HEIGHT, "Reset");
	//m_buttonA->callback((Fl_Callback*)cb_buttonA, this);
	//m_buttonA->resizable(0);

	m_pInputA = new Fl_Input(0,y+(h-COLOR_CHOOSER_HEIGHT)+MARGIN,420,BUTTON_HEIGHT);
    m_pInputA->value("");
	m_pButtonChooser = new Fl_Button(	(m_pInputA->x()+m_pInputA->w()),
										(y+(h-COLOR_CHOOSER_HEIGHT)+MARGIN), 
										BUTTON_WIDTH, BUTTON_HEIGHT,"Browse..");
    m_pButtonChooser->callback((Fl_Callback *)cb_ButtonChooser, this);
	m_pFileChooser = NULL;
	
	m_pLB = new Fl_Light_Button(w - BUTTON_WIDTH/2*3 - 3,
			 	    (y+(h-COLOR_CHOOSER_HEIGHT)+MARGIN), 
				    BUTTON_WIDTH/2*3,BUTTON_HEIGHT, "Show all clusters");
    //m_pLB->labelsize(TEXTSIZE);
    m_pLB->callback((Fl_Callback *)cb_LB, this);
	m_pLB->set();
	m_bshowall = true; 

	m_pButtonNext = new Fl_Button(	w - BUTTON_WIDTH/2*3 - 3,
									(y+(h-COLOR_CHOOSER_HEIGHT)+MARGIN + BUTTON_HEIGHT + 3),
									BUTTON_WIDTH/2*3, BUTTON_HEIGHT,"Next Cluster");
    m_pButtonNext->callback((Fl_Callback *)cb_ButtonNext, this);

#ifdef CHANGES
	m_pButtonClear = new Fl_Light_Button(	w - BUTTON_WIDTH/2*3 - 3,
									(y+(h-COLOR_CHOOSER_HEIGHT)+MARGIN + BUTTON_HEIGHT*2 + 6),
									BUTTON_WIDTH/2*3, BUTTON_HEIGHT,"Clear Cluster");
    m_pButtonClear->callback((Fl_Callback *)cb_ButtonClear, this);
	m_pButtonClear->set();
	m_bClear = true;
#endif

	m_total_size = 0;
	m_dim = 0;
	m_selcluster = 0;
	m_bInit = false;
	m_nClusters = 0;

	m_xblock = 1;
	m_yblock = 1;
	m_zblock = 1;

#ifdef SHOW_CLUSTER
	pVolume->SetClusterView(this);
#endif
}

#ifdef CHANGES
void ClusterView2D::clean()
{
	for(int i=0; i < m_total_size; i++)
		delete [] m_dataPts[i];
	if(m_dataPts)
		delete [] m_dataPts;
	//if(m_clusterPts)
	//	delete [] m_clusterPts;
	int arr_len = (m_dim %m_ts_chunk == 0) ? (m_dim / m_ts_chunk) : (m_dim / m_ts_chunk)+1;
	DeallocateDArray(arr_len, m_total_size);

	delete m_pButtonChooser;
	delete m_pInputA;

}
#endif

ClusterView2D::~ClusterView2D(){
	for(int i=0; i < m_total_size; i++)
		delete [] m_dataPts[i];
	if(m_dataPts)
		delete [] m_dataPts;
	//if(m_clusterPts)
	//	delete [] m_clusterPts;
	int arrlen = (m_dim %m_ts_chunk == 0) ? (m_dim / m_ts_chunk) : (m_dim / m_ts_chunk)+1;
	DeallocateDArray(arr_len, m_total_size);
	
	delete m_pButtonChooser;
	delete m_pInputA;
}




void ClusterView2D::drawBackground(){
	int nextNode = 0;
	int nextIndex = 0;
	float x1, x2;
	// draw color box

}

void ClusterView2D::draw() {
/*
	glClearColor( 0, 0, 0, 1 );
	glClear( GL_COLOR_BUFFER_BIT );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glOrtho( 0, 1, 0, 1, -1, 1 );

	glShadeModel( GL_SMOOTH );
	glBegin( GL_QUADS );
		glColor3f( 1, 0, 0 );
		glVertex3f( 0, 0, 0 );
		glColor3f( 0, 1, 0 );
		glVertex3f( 0, 1, 0 );
		glColor3f( 0, 0, 1 );
		glVertex3f( 1, 1, 1 );
		glColor3f( 1, 1, 1 );
		glVertex3f( 1, 0, 0 );
	glEnd();
	glFlush();
*/

	glViewport(0, 0, w(), h());
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0,1.0,1.0,1.0);
/*
	glPushMatrix();
	glColor4f(1.0, 0.0, 0.0, 1);
	glBegin(GL_LINE_LOOP);
		glVertex3f( -1, -1, 0 );
		glVertex3f( -1, 0, 0 );
		glVertex3f( 0, 0, 0 );
		glVertex3f( 0, -1, 0 );
	glEnd();
	glPopMatrix();
*/
	glPushMatrix();
	glTranslatef(-1, -1, 0);
	glScalef(2, 2, 1);
	

	glLineWidth(1);
    glShadeModel(GL_SMOOTH);  // Because each line is the same color.

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

#ifdef CHANGES
	if(!m_bClear){
#endif
	for(int i = 0; i < m_total_size; i++){
		float* data = m_dataPts[i];
		//int clusterid = m_clusterPts[i];
		if(m_bshowall || clusterid == m_selcluster){
			glBegin(GL_LINE_STRIP);
			//ColorCluster(clusterid);
			int cl_ind = 0;
			for (int j = 0; j < m_dim; j++){
				int clusterid = m_cluster[cl_ind][i];
				ColorCluster(clusterid);

				float x = XMap(j)/(m_percentWidth*(float)w());//COLOR_CHOOSER_WIDTH;
				float y = (0.1*(m_percentHeight*(float)h()) + YMap(data[j]))/(m_percentHeight*(float)h());//COLOR_CHOOSER_HEIGHT;
				glVertex2f( x,y);

				if(j != 0 && (j % m_ts_chunk == 0))
					cl_ind++;
			}
			glEnd();
			glFlush();
		}
	}
#ifdef CHANGES
	}
#endif	

	glDisable(GL_BLEND);
    glLineWidth(7);
    
    glShadeModel(GL_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
	glPopMatrix();

	glFlush();

	
}

/*
int ClusterView2D::handle(int event) {
	
	return 1;
}
*/

bool ClusterView2D::read(const char* filename)
{
	m_dataStream.open(filename, ios::in | ios::binary);
	if (!m_dataStream) {
		cerr << "Cannot open tac data file " << filename << endl;
		return false;
	}
		
	char filename_cluster[1024];
	strcpy(filename_cluster, filename);
	strcat(filename_cluster, ".cluster");
	m_clusterStream.open(filename_cluster, ios::in | ios::binary);
	if (!m_clusterStream) {
		cerr << "Cannot open cluster file " << filename_cluster << endl ;
		goto CLEANUP;
	}

	int xsize, ysize, zsize;
	if(!readTACFileHeader(m_dataStream, &xsize, &ysize, &zsize, &m_dim, &m_xblock, &m_yblock, &m_zblock)){
		goto CLEANUP;
	}

	m_total_size = xsize*ysize*zsize;
	m_dataPts = new float*[m_total_size];
	for(int i=0; i < m_total_size; i++)
		m_dataPts[i] = new float[m_dim];

	if(!readTACData(m_dataStream, m_dataPts, m_dim, m_total_size)){
		goto CLEANUP;
	}

	//m_dataStream.close();

	//m_clusterPts = new int[m_total_size];
	int nClusters = 0;
	
	if(!readClusterHeader(m_clusterStream, m_ts_chunk)){
		goto CLEANUP;
	}
	
	int arrlen = (m_dim %m_ts_chunk == 0) ? (m_dim / m_ts_chunk) : (m_dim / m_ts_chunk)+1;
	m_cluster = AllocateDArray(arr_len, m_total_size);
	int count = 0;
	int total = 0;
	while(total < m_dim){
		if(!readClusterData(m_clusterStream, m_cluster[count], m_total_size)){
			goto CLEANUP;
		}
		count++;
		total += m_ts_chunk;
	}
	m_clusterPts = m_cluster[0];
	
	if(!m_clusterStream.eof()){
		float** centroid = NULL;
		readClusterNum(m_clusterStream, m_nClusters);
		//readClusterCentroid(m_clusterStream, m_dim, nClusters, centroid);
	}
	
CLEANUP:
	if(m_dataStream)
		m_dataStream.close();
	if(m_clusterStream)
		m_clusterStream.close();
	return true;
}

void ClusterView2D::ColorCluster(int clusterid){

	int id = clusterid%MAX_NUM_CLUSTERS;
/*
	float r=(rand()%100)/100.0;
	float g=(rand()%100)/100.0;
	float b=(rand()%100)/100.0;
    glColor4f(r, g, b, 0.01);
*/
	switch(id){
		case 0: // black
			glColor4f(0.0, 0.0, 0.0, 0.04);
			break;
		case 1:  //green
			glColor4f(0.0, 1.0, 0.0, 0.04);
			// brown
			//glColor4f(0.15, 0.2, 0.05, 0.4);
			break;
		case 2: // blue
			glColor4f(0.0, 0.0, 1.0, 0.04);
			break;
		case 3:  //red
			glColor4f(1.0, 0.0, 0.0, 0.04);
			break;
		case 4: // yellow
			glColor4f(1.0, 1.0, 0.0, 0.06);
			break;
		case 5:
			glColor4f(0.5, 0.5, 0.5, 0.6);
			break;
		default:
			glColor4f(0.5, 0.5, 0.5, 1);

	}

}

inline float ClusterView2D::YMap(float yc){ 
	return (m_percentHeight * h() * 0.8) * yc;
}

inline float ClusterView2D::XMap(float i) { 
	return m_percentWidth * w() * ((float)i / (float)m_dim); 
}



// UI CALLBACKS
void ClusterView2D::cb_ButtonA(Fl_Button* o, void* v) {
  ((ClusterView2D*)(o->parent()->user_data()))->cb_ButtonA_i(o,v);
}

inline void ClusterView2D::cb_ButtonA_i(Fl_Button* o, void* v)
{
	ClusterView2D* tf = reinterpret_cast<ClusterView2D*>(v);
	tf->redraw();
}

void ClusterView2D::cb_ButtonChooser(Fl_Button* o, void* v) {
  ((ClusterView2D*)(o->parent()->user_data()))->cb_ButtonChooser_i(o,v);
}

inline void ClusterView2D::cb_ButtonChooser_i(Fl_Button* o, void* v)
{
	ClusterView2D* tf = reinterpret_cast<ClusterView2D*>(v);
	
	if(!tf->m_pFileChooser)
		tf->m_pFileChooser = new Fl_File_Chooser("", "", Fl_File_Chooser::SINGLE, "");
    tf->m_pFileChooser->show();

    // Block until user picks something.
    //     (The other way to do this is to use a callback())
    //
    while(tf->m_pFileChooser->shown()) {
        Fl::wait();
    }

    // Print the results
    if ( tf->m_pFileChooser->value() == NULL ) {
        printf("(User hit 'Cancel')\n");
        return;
    }

	printf("DIRECTORY: '%s'\n", tf->m_pFileChooser->directory());
    printf("    VALUE: '%s'\n", tf->m_pFileChooser->value());
	const char* temp= tf->m_pFileChooser->value();
	tf->m_pInputA->value(tf->m_pFileChooser->value());
	tf->read(tf->m_pFileChooser->value());
	tf->m_bInit = true;
	tf->redraw();
}

void ClusterView2D::cb_LB_i(Fl_Light_Button* o, void* v)
{
	ClusterView2D* tf = reinterpret_cast<ClusterView2D*>(v);
	if(tf->isInit()){
	tf->m_bshowall = !tf->m_bshowall;
	tf->redraw();
	}
}

void ClusterView2D::cb_LB(Fl_Light_Button* o, void* v)
{
	((ClusterView2D*)(o->parent()->user_data()))->cb_LB_i(o,v);
}

inline void ClusterView2D::cb_ButtonNext_i(Fl_Button* o, void* v)
{
	ClusterView2D* tf = reinterpret_cast<ClusterView2D*>(v);
	if(tf->isInit()){
		tf->m_selcluster++;
		tf->m_selcluster = tf->m_selcluster%tf->m_nClusters;
#ifdef SHOW_CLUSTER
		tf->m_pVolume->UpdateClusterId();
#endif
		tf->redraw();
	}
}

void ClusterView2D::cb_ButtonNext(Fl_Button* o, void* v)
{
	((ClusterView2D*)(o->parent()->user_data()))->cb_ButtonNext_i(o,v);
}


#ifdef CHANGES
inline void ClusterView2D::cb_ButtonClear_i(Fl_Light_Button* o, void* v)
{
	ClusterView2D* tf = reinterpret_cast<ClusterView2D*>(v);
	tf->m_bClear = !tf->m_bClear;
	tf->redraw();
}

void ClusterView2D::cb_ButtonClear(Fl_Light_Button* o, void* v)
{
	((ClusterView2D*)(o->parent()->user_data()))->cb_ButtonClear_i(o,v);
}
#endif


//----------------------------------------------------------------------
//
//	readTACFileHeader - read header of TAC data file.
//----------------------------------------------------------------------

bool readTACFileHeader(ifstream& inf, int *p_xsize, 
					   int *p_ysize, int *p_zsize, 
					   int *dim, int* xblock, int *yblock, int *zblock)
{

    //ifstream inf(filename, ios::binary);

    if (!inf)
        return false;

    int xstart, xend, ystart, yend, zstart, zend;
    int zsize, ysize, xsize;
    int total_size, time_out, time_in;
    int time_step;

	int xblocksize, yblocksize, zblocksize;
    inf.read(reinterpret_cast<char *>(&xstart), sizeof(int));
    inf.read(reinterpret_cast<char *>(&xend), sizeof(int));
    inf.read(reinterpret_cast<char *>(&ystart), sizeof(int));
    inf.read(reinterpret_cast<char *>(&yend), sizeof(int));
    inf.read(reinterpret_cast<char *>(&zstart), sizeof(int));
    inf.read(reinterpret_cast<char *>(&zend), sizeof(int));
    inf.read(reinterpret_cast<char *>(&time_in), sizeof(int));
    inf.read(reinterpret_cast<char *>(&time_out), sizeof(int));
	inf.read(reinterpret_cast<char *>(&xblocksize), sizeof(int));
    inf.read(reinterpret_cast<char *>(&yblocksize), sizeof(int));
    inf.read(reinterpret_cast<char *>(&zblocksize), sizeof(int));
    xsize = xend - xstart;
    ysize = yend - ystart;
    zsize = zend - zstart;
	assert(xsize >= 1 && ysize >= 1 && zsize >= 1 && time_out >= 1);

    
    //time_out = time_out + time_in - 1;
    time_in = 1;

	xsize = xsize / xblocksize;
	ysize = ysize / yblocksize;
	zsize = zsize / zblocksize;
	total_size = xsize * ysize * zsize;
	*p_xsize = xsize;
	*p_ysize = ysize;
	*p_zsize = zsize;
    *dim = time_out;

	*xblock = xblocksize;
	*yblock = yblocksize;
	*zblock = zblocksize;
    return true;
}

bool readTACData(ifstream& inf, float** dataPts, int dim, int nPts)
{
	if (!inf)
        return false;

    float value;
    int time_step;
	int i = 0;
    while(i < dim){
	    //inf.read(reinterpret_cast<char *>(&time_step), sizeof(int));
		for(int j=0; j < nPts; j++){
			inf.read(reinterpret_cast<char *>(&value), sizeof(float));
			dataPts[j][i] = value;
		}
		i++;
    }
    return true;
}

bool readClusterHeader(ifstream& inf, int& ts_chunk)
{
	if (!inf)
        return false;
	
	inf.read(reinterpret_cast<char *>(&ts_chunk), sizeof(int));
	return true;
}

bool readClusterData(ifstream& inf, int* dataPts, int nPts)
{
	if (!inf)
        return false;
	int value;
    for(int i=0; i < nPts; i++){
		inf.read(reinterpret_cast<char *>(&value), sizeof(int));
		dataPts[i] = value;
	}
	return true;
}

bool readClusterNum(ifstream& inf, int& nClusters)
{
	inf.read(reinterpret_cast<char *>(&nClusters), sizeof(int));
	return true;
}

bool readClusterCentroid(ifstream& inf, int dim, int nClusters, float** centroid)
{
	if(centroid){
		for (int j = 0; j < nClusters; j++){
			for (int d = 0; d < dim; d++) {
				inf.read(reinterpret_cast<char *>(&centroid[j][d]), sizeof(float));
			}
		}
	}
	return true;
}


/*
bool writeClusterData(ofstream& outf, KMfilterCenters& ctrs, const KMdata&    dataPts)
{
    if (!outf)
        return false;

    KMctrIdxArray closeCtr = new KMctrIdx[dataPts.getNPts()];
    double* sqDist = new double[dataPts.getNPts()];
    ctrs.getAssignments(closeCtr, sqDist);

    for(int i=0; i < dataPts.getNPts(); i++){
        outf.write(reinterpret_cast<char *>(&closeCtr[i]), sizeof(int));
    }

    // write out number of clusters
    outf.write(reinterpret_cast<char *>(&k), sizeof(int));
    for (int j = 0; j < k; j++){
        for (int d = 0; d < ctrs.getDim(); d++) {
            outf.write(reinterpret_cast<char *>(&ctrs[j][d]), sizeof(float));
        }
    }
    delete [] closeCtr;
    delete [] sqDist;
   
    return true;
}
*/


int** AllocateDArray(int num_items, int item_size)
{
	int arr = new int*[num_items];
	for(int i=0; i < num_items; i++){
		arr[i] = new int[item_size];
	}
	return arr;
}

void DeallocateDArray(int num_items, int item_size)
{
	for(int i=0; i < num_items; i++){
		delete arr[i];
	}
	delete [] arr;
}