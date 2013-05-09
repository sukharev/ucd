/*
 *  clusterview2d.cpp
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
#include <time.h>
#include "kmlsample.h"

#define NODE_HEIGHT 15
#define NODE_WIDTH  20
#define COLOR_CHOOSER_WIDTH  180
#define COLOR_CHOOSER_HEIGHT 60
#define MARGIN 20 //15
#define BUTTON_HEIGHT 25
#define BUTTON_WIDTH  150

#define MAX_NUM_CLUSTERS  6;

//bool readTACFileHeader(ifstream& inf, int *p_xsize, int *p_ysize, int *p_zsize, int *dim);
bool readTACFileHeader(ifstream& inf, int *p_xsize, 
					   int *p_ysize, int *p_zsize, 
					   int *dim, int* xblock, int *yblock, int *zblock);
bool readTACData(ifstream& inf, float** dataPts, int dim, int nPts, float& min, float& max);
bool readClusterHeader(ifstream& inf, int& num_clusters, int& ts_chunk);
//bool readClusterData(ifstream& inf, int* dataPts, int nPts, int nClusters);
bool readClusterData(ifstream& inf, int* dataPts, int nPts, int nClusters, int ts_chunk, float** ctrs);
bool readClusterNum(ifstream& inf, int& nClusters);
bool readClusterCentroid(ifstream& inf, int dim, int nClusters, float** centroid = NULL);


int** AllocateDArray(int num_items, int item_size);
void DeallocateDArray(int** arr, int num_items, int item_size);

float*** AllocateTArray(int num_arr, int num_items, int item_size);
void DeallocateTArray(float*** tarr, int num_arr, int num_items, int item_size);

ClusterView2D::ClusterView2D(int x, int y, int w, int h, int inResolution, cxVolume * pVolume) : Fl_Gl_Window(x, y, w, h-COLOR_CHOOSER_HEIGHT){

	//mode(FL_ALPHA | FL_DEPTH | FL_DOUBLE | FL_RGB8 );
	//	Fl::add_idle(&idle_cp, (void*)this);
	opacityDisp = 0;
	histogram1D = 0;
	m_dataPts = NULL;
	m_cluster = NULL;
	m_ctrs = NULL;
	m_pVolume = pVolume;
	
	m_percentHeight = (float)COLOR_CHOOSER_HEIGHT / (float) h;
	m_percentWidth = (float)COLOR_CHOOSER_WIDTH / (float) w;
	ClusterView2D *ihateptrs = this; // this is necessary because &this doesnt work.
		
	//m_buttonA = new Fl_Button(x+MARGIN+COLOR_CHOOSER_WIDTH, y+(h-COLOR_CHOOSER_HEIGHT)+MARGIN, BUTTON_WIDTH,BUTTON_HEIGHT, "Reset");
	//m_buttonA->callback((Fl_Callback*)cb_buttonA, this);
	//m_buttonA->resizable(0);
	

	Fl_Group* o = new Fl_Group(	2,
								y+(h-COLOR_CHOOSER_HEIGHT)+MARGIN,
								COLOR_CHOOSER_WIDTH*3,
								COLOR_CHOOSER_HEIGHT*4,"K-Means settings");
    o->box(FL_ENGRAVED_FRAME);
    o->align(FL_ALIGN_TOP_LEFT);

	m_pInputA = new Fl_Input(	o->x()+3,
								o->y()+MARGIN,
								BUTTON_WIDTH*2 + 3*2,
								BUTTON_HEIGHT,"Filename");
    m_pInputA->value("");
	m_pInputA->align(FL_ALIGN_RIGHT);
	
	m_pButtonChooser = new Fl_Button(	(m_pInputA->x()+m_pInputA->w()),
										o->y()+ MARGIN, 
										BUTTON_WIDTH, BUTTON_HEIGHT,"Browse..");
    m_pButtonChooser->callback((Fl_Callback *)cb_ButtonChooser, this);
	
	m_pFileChooser = NULL;
	
	m_inp_tch = new Fl_Input(o->x() + 3, m_pInputA->y()+ BUTTON_HEIGHT*2, BUTTON_WIDTH, BUTTON_HEIGHT, "timesteps intervals");
	m_inp_tch->align(FL_ALIGN_TOP_LEFT);
	m_inp_tch->value("12");
	
	m_inp_t = new Fl_Input(o->x() + 3, m_pInputA->y()+ BUTTON_HEIGHT*3 + MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT, "algorithm");
    m_inp_t->align(FL_ALIGN_TOP_LEFT);
	m_inp_t->value("0");
	
	m_inp_k = new Fl_Input(m_inp_tch->x()+m_inp_tch->w() + 3, m_pInputA->y()+ BUTTON_HEIGHT*2, BUTTON_WIDTH, BUTTON_HEIGHT, "clusters");
	m_inp_k->align(FL_ALIGN_TOP_LEFT);
	m_inp_k->value("3");
	
	m_inp_s = new Fl_Input(m_inp_t->x()+m_inp_t->w() + 3, m_pInputA->y()+ BUTTON_HEIGHT*3 + MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT, "stages");
	m_inp_s->align(FL_ALIGN_TOP_LEFT);
	m_inp_s->value("300");
	
	m_inp_start_time = new Fl_Input(m_inp_k->x()+m_inp_k->w() + 3, m_pInputA->y()+ BUTTON_HEIGHT*2, 
									BUTTON_WIDTH, BUTTON_HEIGHT, "start timestep");
	m_inp_start_time->align(FL_ALIGN_TOP_LEFT);
	m_inp_start_time->value("0");

	m_inp_total_time = new Fl_Input(m_inp_s->x()+m_inp_s->w() + 3, m_pInputA->y()+ BUTTON_HEIGHT*3 + 
									MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT, "number of timesteps");
	m_inp_total_time->align(FL_ALIGN_TOP_LEFT);
	m_inp_total_time->value("120");


	Fl_Button* bt_save = new Fl_Button(	m_inp_tch->x()+m_inp_tch->w() + 3,
										m_pInputA->y()+ BUTTON_HEIGHT*5 + MARGIN*3, 
										BUTTON_WIDTH, BUTTON_HEIGHT,"Create new cluster");
	bt_save->callback((Fl_Callback *)cb_ButtonSave, this);

	Fl_Button* bt_load = new Fl_Button(	bt_save->x()+bt_save->w()+3,
										m_pInputA->y()+ BUTTON_HEIGHT*5 + MARGIN*3, 
										BUTTON_WIDTH, BUTTON_HEIGHT,"Load cluster");
	bt_load->callback((Fl_Callback *)cb_ButtonLoad, this);	

	
	o->end();
	m_pLB = new Fl_Light_Button(o->x() + o->w() + 3,
			 	    (y+(h-COLOR_CHOOSER_HEIGHT)+MARGIN), 
				    BUTTON_WIDTH,BUTTON_HEIGHT, "Show all clusters");
    //m_pLB->labelsize(TEXTSIZE);
    m_pLB->callback((Fl_Callback *)cb_LB, this);
	m_pLB->set();
	m_bshowall = true; 

	m_pButtonNext = new Fl_Button(	o->x() + o->w() + 3,
									(y+(h-COLOR_CHOOSER_HEIGHT)+MARGIN + BUTTON_HEIGHT + 3),
									BUTTON_WIDTH, BUTTON_HEIGHT,"Next Cluster");
    m_pButtonNext->callback((Fl_Callback *)cb_ButtonNext, this);

#ifdef CHANGES
	m_pButtonClear = new Fl_Light_Button(	o->x() + o->w() + 3,
									(y+(h-COLOR_CHOOSER_HEIGHT)+MARGIN + BUTTON_HEIGHT*2 + 6),
									BUTTON_WIDTH, BUTTON_HEIGHT,"Clear Cluster");
    m_pButtonClear->callback((Fl_Callback *)cb_ButtonClear, this);
	m_pButtonClear->set();
	m_bClear = true;
#endif	
/*
	int tch     = 0;        // use all timesteps in calculating KMeans (use only 1 chunk) -tch
int alg     = 0;        // use Lloyd by default  -t option
int	k		= 4;		// number of centers     -k option
int	dim		= 2;		// dimension            
int	stages		= 1000;		// number of stages  -s option

	strcpy(params[0],"");
	strcpy(params[1],"-t");
	strcpy(params[2],"3");
	strcpy(params[3],"-tch");
	strcpy(params[4],"12");
	strcpy(params[5],"-tac");
	strcpy(params[6],filename);
	strcpy(params[7],"-k");
	strcpy(params[8],"3");
	strcpy(params[9],"-s");
	strcpy(params[10],"300");
	main_cluster(nparams, params);
	*/

	m_total_size = 0;
	m_dim = 0;
	m_selcluster = 0;
	m_bInit = false;
	m_nClusters = 0;

	m_xblock = 1;
	m_yblock = 1;
	m_zblock = 1;

	m_datamin = -100;
	m_datamax = 100;
#ifdef SHOW_CLUSTER
	pVolume->SetClusterView(this);
#endif

	srand((unsigned)time( NULL ));
}

#ifdef CHANGES
void ClusterView2D::clean()
{
	m_bClear = true;
	m_pButtonClear->set();
	for(int i=0; i < m_total_size; i++)
		delete [] m_dataPts[i];
	if(m_dataPts)
		delete [] m_dataPts;
	m_dataPts=NULL;
	//if(m_clusterPts)
	//	delete [] m_clusterPts;
	if(m_dim>0 && m_ts_chunk>0)
	{
		int arr_len = (m_dim %m_ts_chunk == 0) ? (m_dim / m_ts_chunk) : (m_dim / m_ts_chunk)+1;
		if(m_cluster) DeallocateDArray(m_cluster, arr_len, m_total_size);
		if(m_ctrs) DeallocateTArray(m_ctrs, arr_len,m_nClusters,m_ts_chunk);
	}
	m_cluster = NULL;
	m_ctrs = NULL;
	m_total_size = 0;
}
#endif

ClusterView2D::~ClusterView2D(){
	for(int i=0; i < m_total_size; i++)
		delete [] m_dataPts[i];
	if(m_dataPts)
		delete [] m_dataPts;
	//if(m_clusterPts)
	//	delete [] m_clusterPts;
	int arr_len = (m_dim %m_ts_chunk == 0) ? (m_dim / m_ts_chunk) : (m_dim / m_ts_chunk)+1;
	DeallocateDArray(m_cluster, arr_len, m_total_size);
	DeallocateTArray(m_ctrs, arr_len,m_nClusters,m_ts_chunk);
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
	float x,y;
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
	
	//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);    // Use The Good Calculations
	//glEnable(GL_LINE_SMOOTH);            // Enable Anti-Aliasing
	glLineWidth(1);
    glShadeModel(GL_SMOOTH);  // Because each line is the same color.

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	int cl_ind;
#ifdef CHANGES
	if(!m_bClear){
#endif
	for(int i = 0; i < m_total_size; i++){
		float* data = m_dataPts[i];
		//glBegin(GL_LINE_STRIP);
		cl_ind = 0;
		glBegin(GL_LINE_STRIP);
		float alpha = (float)(rand()%100) / 1000.0;
		//int clusterid = m_cluster[cl_ind][i];
		while(cl_ind*m_ts_chunk < m_dim){
			int clusterid = m_cluster[cl_ind][i];
			if(m_bshowall || clusterid == m_selcluster){

				
				ColorCluster(clusterid,alpha);
				for (int j = (m_ts_chunk * cl_ind); j < (m_ts_chunk * (cl_ind+1)); j++){
					glBegin(GL_LINES);
						x = XMap(j);///(m_percentWidth*(float)w());//COLOR_CHOOSER_WIDTH;
						y = (/*0.1*(m_percentHeight*(float)h()) + */YMap((data[j]-m_datamin)/(m_datamax-m_datamin)));///(m_percentHeight*(float)h());//COLOR_CHOOSER_HEIGHT;
						glVertex2f( x,y);
					
						if(j + 1 != m_dim){
							x = XMap(j+1);///(m_percentWidth*(float)w());//COLOR_CHOOSER_WIDTH;
							y = (/*0.1*(m_percentHeight*(float)h()) + */YMap((data[j+1]-m_datamin)/(m_datamax-m_datamin)));///(m_percentHeight*(float)h());//COLOR_CHOOSER_HEIGHT;
							glVertex2f( x,y);
						}
					glEnd();
					//glFlush();
				}
				
			}
			cl_ind++;
		}
		//glEnd();
		//glFlush();

	}

	glLineWidth(2.0);
	glColor3f(0.0f,0.0f,0.0f);
	cl_ind = 0;
	
	for (int j = 0; j < m_nClusters; j++){
		if(m_bshowall || m_selcluster == j){
			glBegin(GL_LINE_STRIP);
			cl_ind = 0;
			while(cl_ind*m_ts_chunk < m_dim){
				int ind = m_ts_chunk * cl_ind;
				for (int d = cl_ind*m_ts_chunk; d < (cl_ind+1)*m_ts_chunk; d++) {
					//glBegin(GL_LINES);
					float x = XMap(d);///(m_percentWidth*(float)w());//COLOR_CHOOSER_WIDTH;
					int index = d % m_ts_chunk;
					float y = (/*0.1*(m_percentHeight*(float)h()) + */YMap((m_ctrs[cl_ind][j][index]-m_datamin)/(m_datamax-m_datamin)));///(m_percentHeight*(float)h());//COLOR_CHOOSER_HEIGHT;
					glVertex2f( x,y);
					/*
					if(d+1 < m_dim){
						x = XMap(d+1)/(m_percentWidth*(float)w());//COLOR_CHOOSER_WIDTH;
						index = (d+1) % m_ts_chunk;
						y = (0.1*(m_percentHeight*(float)h()) + YMap((m_ctrs[cl_ind][j][index]-m_datamin)/(m_datamax-m_datamin)))/(m_percentHeight*(float)h());//COLOR_CHOOSER_HEIGHT;
						glVertex2f( x,y);
					}
					*/
					ind++;
					//glEnd();
				}
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
	int xsize, ysize, zsize;
	int nClusters = 0;
	int arr_len;
	int count = 0, total = 0;
	char filename_cluster[1024];
	int i=0;
	m_dataStream.open(filename, ios::in | ios::binary);
	if (!m_dataStream) {
		cerr << "Cannot open tac data file " << filename << endl;
		return false;
	}
		
	
	strcpy(filename_cluster, filename);
	strcat(filename_cluster, ".cluster");
	m_clusterStream.open(filename_cluster, ios::in | ios::binary);
	if (!m_clusterStream) {
		cerr << "Cannot open cluster file " << filename_cluster << endl ;
		goto CLEANUP;
	}

	if(!readTACFileHeader(m_dataStream, &xsize, &ysize, &zsize, &m_dim, &m_xblock, &m_yblock, &m_zblock)){
		goto CLEANUP;
	}

	m_total_size = xsize*ysize*zsize;
	m_dataPts = new float*[m_total_size];
	for(i=0; i < m_total_size; i++)
		m_dataPts[i] = new float[m_dim];

	if(!readTACData(m_dataStream, m_dataPts, m_dim, m_total_size, m_datamin, m_datamax)){
		goto CLEANUP;
	}

	//m_dataStream.close();

	//m_clusterPts = new int[m_total_size];
	if(!readClusterHeader(m_clusterStream, m_nClusters, m_ts_chunk)){
		goto CLEANUP;
	}
	
	arr_len = (m_dim %m_ts_chunk == 0) ? (m_dim / m_ts_chunk) : (m_dim / m_ts_chunk)+1;
	m_cluster = AllocateDArray(arr_len, m_total_size);
	m_ctrs = AllocateTArray(arr_len,m_nClusters,m_ts_chunk);
	count = 0;
	total = 0;
	while(total < m_dim){
		//fprintf(stderr,"time chunk index %d\n",count);
		if(!readClusterData(m_clusterStream, m_cluster[count], m_total_size, m_nClusters, m_ts_chunk, m_ctrs[count])){
		//if(!readClusterData(m_clusterStream, m_cluster[count], m_total_size, m_nClusters)){
			goto CLEANUP;
		}

		count++;
		total += m_ts_chunk;
	}

	m_clusterPts = m_cluster[0];
	

	/*
	if(!m_clusterStream.eof()){
		float** centroid = NULL;
		readClusterNum(m_clusterStream, m_nClusters);
		//readClusterCentroid(m_clusterStream, m_dim, nClusters, centroid);
	}
	*/
	
CLEANUP:
	if(m_dataStream)
		m_dataStream.close();
	if(m_clusterStream)
		m_clusterStream.close();
	return true;
}

void ClusterView2D::ColorCluster(int clusterid, float a){

	int id = clusterid%MAX_NUM_CLUSTERS;
/*
	float r=(rand()%100)/100.0;
	float g=(rand()%100)/100.0;
	float b=(rand()%100)/100.0;
    glColor4f(r, g, b, 0.01);
*/
	switch(id){
		case 0: // orange
			glColor4f(1.0, 0.5, 0.0, a);
			break;
		case 1:  //green
			glColor4f(0.0, 0.75, 0.0, a);
			// brown
			//glColor4f(0.15, 0.2, 0.05, 0.4);
			break;
		case 2: // red
			glColor4f(0.75, 0.0, 0.0, a);
			break;
		case 3:  //yellow
			glColor4f(0.75, 0.75, 0.0, a);
			break;
		case 4: //
			glColor4f(1.0, 0.75, 0.75, 0.4);
			break;
		case 5:
			glColor4f(0.5, 0.5, 0.5, 0.6);
			break;
		default:
			glColor4f(0.5, 0.5, 0.5, 1);

	}

}

inline float ClusterView2D::YMap(float yc){ 
	return /*(m_percentHeight * h() * */(1.0) * yc;
}

inline float ClusterView2D::XMap(float i) { 
	return /*m_percentWidth * w() **/ ((float)i / (float)m_dim); 
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
	//tf->read(tf->m_pFileChooser->value());
	//tf->m_bInit = true;

	//if(tf->m_pFileChooser)
	//	delete tf->m_pFileChooser;
	tf->redraw();
}

void ClusterView2D::cb_ButtonLoad(Fl_Button* o, void* v) {
  ((ClusterView2D*)(o->parent()->user_data()))->cb_ButtonLoad_i(o,v);
}

inline void ClusterView2D::cb_ButtonLoad_i(Fl_Button* o, void* v)
{
	ClusterView2D* tf = reinterpret_cast<ClusterView2D*>(v);
	
	tf->clean();
	//if(!tf->m_pFileChooser)
		
	//tf->m_pFileChooser = new Fl_File_Chooser("", "", Fl_File_Chooser::SINGLE, "");
    Fl_File_Chooser* pFileChooser =  new Fl_File_Chooser("", "", Fl_File_Chooser::SINGLE, "");
	//tf->m_pFileChooser->show();
	pFileChooser->show();

    // Block until user picks something.
    //     (The other way to do this is to use a callback())
    //
    //while(tf->m_pFileChooser->shown()) {
	while(pFileChooser->shown()) {
        Fl::wait();
    }

    // Print the results
    //if ( tf->m_pFileChooser->value() == NULL ) {
	if ( pFileChooser->value() == NULL ) {
        printf("(User hit 'Cancel')\n");
        return;
    }

	printf("DIRECTORY: '%s'\n", pFileChooser->directory());
    printf("    VALUE: '%s'\n", pFileChooser->value());
	const char* temp= pFileChooser->value();
	tf->m_pInputA->value(pFileChooser->value());
	tf->read(pFileChooser->value());
	tf->m_bInit = true;
	
	
	if(pFileChooser)
		delete pFileChooser;
	tf->redraw();
}

void ClusterView2D::cb_ButtonSave(Fl_Button* o, void* v) {
  ((ClusterView2D*)(o->parent()->user_data()))->cb_ButtonSave_i(o,v);
}

inline void ClusterView2D::cb_ButtonSave_i(Fl_Button* o, void* v)
{
	ClusterView2D* tf = reinterpret_cast<ClusterView2D*>(v);
	if((strcmp(tf->m_inp_tch->value(),"")==0) ||
	  (strcmp(tf->m_inp_t->value(),"")==0) ||
	  (strcmp(tf->m_inp_k->value(),"")==0) ||
	  (strcmp(tf->m_inp_s->value(),"")==0) ||
	  (strcmp(tf->m_pInputA->value(),"")==0))
		return;
	tf->clean();

	if(tf->m_pVolume){
		tf->m_pVolume->SaveCustomTAC_vol((char*)tf->m_pInputA->value(), 
										atoi(tf->m_inp_start_time->value()), 
										atoi(tf->m_inp_total_time->value()));
		int nparams = 11;
		char** params;
		params = new char*[12];
		for(int i=0; i < nparams; i++)
			params[i] = new char[128];
		strcpy(params[0],"");
		strcpy(params[1],"-t");			// algorithm def: 0
		strcpy(params[2],tf->m_inp_t->value());
		strcpy(params[3],"-tch");		// number of timestamps used in calculating KMeans def: 0 (All timestamps)
		strcpy(params[4],tf->m_inp_tch->value());
		strcpy(params[5],"-tac");		// saved tac filename
		strcpy(params[6],tf->m_pInputA->value());
		strcpy(params[7],"-k");			// number of cluster centers def: 4
		strcpy(params[8],tf->m_inp_k->value());
		strcpy(params[9],"-s");		    // number of stages def: 1000
		strcpy(params[10],tf->m_inp_s->value());
		main_cluster(nparams, params);

		for(int i=0; i < nparams; i++)
			delete [] params[i];
		delete [] params;
		/*
		tf->m_pInputA->value(tf->m_pFileChooser->value());
		*/
		tf->read(tf->m_pInputA->value());
		tf->m_bInit = true;
	
	
		//if(tf->m_pFileChooser)
		//	delete tf->m_pFileChooser;
		tf->redraw();
	}

	//if(tf->m_pFileChooser)
	//	delete tf->m_pFileChooser;
}

void ClusterView2D::cb_LB_i(Fl_Light_Button* o, void* v)
{
	ClusterView2D* tf = reinterpret_cast<ClusterView2D*>(v);
	if(tf->isInit()){
	tf->m_bshowall = !tf->m_bshowall;

#ifdef SHOW_CLUSTER
	tf->m_pVolume->UpdateClusterId();
#endif
	tf->m_pVolume->ReDraw();
	tf->redraw();
	}
}
int ClusterView2D::currCluster() 
{
	if(this->m_bshowall || this->m_bClear){
		return -1;
	}
	return m_selcluster;
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
		tf->m_pVolume->ReDraw();
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
	if(tf->m_bClear)
		tf->m_pButtonClear->set();
#ifdef SHOW_CLUSTER
	tf->m_pVolume->UpdateClusterId();
#endif
	tf->redraw();
	tf->m_pVolume->ReDraw();
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
    xsize = xend - xstart+1;
    ysize = yend - ystart+1;
    zsize = zend - zstart+1;
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

bool readTACData(ifstream& inf, float** dataPts, int dim, int nPts, float& min, float& max)
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

	inf.read(reinterpret_cast<char *>(&value), sizeof(float));
    min = value;

	inf.read(reinterpret_cast<char *>(&value), sizeof(float));
    max = value;
	return true;
}

bool readClusterHeader(ifstream& inf, int& num_clusters, int& ts_chunk)
{
	if (!inf)
        return false;
	inf.read(reinterpret_cast<char *>(&num_clusters), sizeof(int));
	inf.read(reinterpret_cast<char *>(&ts_chunk), sizeof(int));
	return true;
}

bool readClusterData(ifstream& inf, int* dataPts, int nPts, int nClusters, int ts_chunk, float** ctrs)
{
	if (!inf)
        return false;
	int value;
	float fvalue;
    for(int i=0; i < nPts; i++){
		//assert(inf);
		//if(!inf.eof()){
			inf.read(reinterpret_cast<char *>(&value), sizeof(int));
			//fprintf(stderr,"dataPts[%d]=%d\n",i,value);
			dataPts[i] = value;
			
		//}
	}

	//fprintf(stderr,"next_cluster\n");
	for (int j = 0; j < nClusters; j++){
		for (int d = 0; d < ts_chunk; d++) {
			inf.read(reinterpret_cast<char *>(&fvalue), sizeof(float));
			ctrs[j][d] = fvalue;
			//fprintf(stderr,"ctrs[%d][%d]=%f\n",j,d,ctrs[j][d]);
		}
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
	int** arr = new int*[num_items];
	for(int i=0; i < num_items; i++){
		arr[i] = new int[item_size];
	}
	return arr;
}

void DeallocateDArray(int** arr, int num_items, int item_size)
{
	if(!arr)
		return;
	for(int i=0; i < num_items; i++){
		delete arr[i];
	}
	delete [] arr;
}


float*** AllocateTArray(int num_arr, int num_items, int item_size)
{
	float*** tarr = new float**[num_arr];
	for(int j=0; j < num_arr; j++){
		float** darr = new float*[num_items];
		for(int i=0; i < num_items; i++){
			darr[i] = new float[item_size];
		}
		tarr[j] = darr;
	}
	return tarr;
}

void DeallocateTArray(float*** tarr, int num_arr, int num_items, int item_size)
{
	if(!tarr)
		return;
	
	for(int j=0; j < num_arr; j++){
		for(int i=0; i < num_items; i++){
			delete tarr[j][i];
		}
		delete [] tarr[j];
	}
	delete [] tarr;
}