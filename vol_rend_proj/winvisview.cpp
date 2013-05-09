//
// C++ Implementation: WinVisView
//
// Description: 
//
//
// Author: Hongfeng Yu <hfyu@ucdavis.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "GL/glew.h"
#include "winvisview.h"
#include "winvismain.h"
#include "cxvolume.h"
#include <GL/glu.h>
#include <FL/glut.H>
#include "trackball.c"
#include <fstream>
//#include <sys/times.h>
//#include <unistd.h>
#include "shader.h"
 

typedef int   int32_t;
typedef short int16_t; 
#define ROTATION_TIME  0.01
 
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WinVisView::WinVisView(int x,int y,int w,int h,const char *l):Fl_Gl_Window(x,y,w,h,l)
{
	m_pVolume = NULL;
    m_pImage = NULL;
    m_nFrameNum = 0;	
    
    m_bSaveImage = false;
    m_nImageCount  = 0;
    
    m_nVolProgram = 0; 
    m_bReload = true;
    
    m_bBlack = true;
    
    m_bRotation = false;
    
    m_bCut = false;
    
    m_bPlayOn = false;
    
    m_bEnd = false;
    
    Reset();
	LoadView();
    
	this->callback((Fl_Callback *)cb_wClose);
}

WinVisView::~WinVisView()
{

}

void WinVisView::Reset()
{
	trackball(m_curquat, 0.0, 0.0, 0.0, 0.0);

	m_scale = 1;
	m_deltax = 0;
	m_deltay = 0;
	
	m_bOperating = false;
    m_bDrawing = false;   
}

void WinVisView::cb_wClose(Fl_Gl_Window*, void*)
{
	exit(0);
}

void WinVisView::draw()
{
	if(!valid())
	{
		glViewport(0,0,w(),h());    
        
		// Turn on Z buffer
		glEnable( GL_DEPTH_TEST );

		// Normalize normals
		glEnable(GL_NORMALIZE);

		//smooth shading
		glShadeModel(GL_SMOOTH);
		//glEnable(GL_SHININESS);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
            // Problem: glewInit failed, something is seriously wrong.
            fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        }
        
         // verify FBOs are supported (otherwise we get FuBar'd Objects)
        /* 
        if (!glutExtensionSupported ("GL_EXT_framebuffer_object") )
        {
            cerr << "FBO extension unsupported" << endl;
            //exit (1);
        }
        */
	}

#ifdef DRAW_LARGEIMAGE
//    int final_image_x = 2048;
//    int final_image_y = 2048;  
    
    int final_image_x = 8*1024;
    int final_image_y = 8*1024;  
    
    int tile_image_x = w();
    int tile_image_y = h();
    int tile_x = final_image_x / tile_image_x;
    int tile_y = final_image_y / tile_image_y;
  
    float aspect_ratio = float(final_image_x) / float(final_image_y);
    float width_per_node = 2.0 * aspect_ratio / (float)tile_x;
    float height_per_node = 2.0 / (float)tile_y;
    
    static unsigned char* final_image_ptr = new unsigned char[final_image_x * final_image_y * 3];
     
    for ( int row = 0; row < tile_y; row++)
    for ( int col = 0; col < tile_x; col++) 
    {
        cerr << "Draw " << row <<  " " << col << endl;
#endif    
	
    // Set background color
    //if (m_bBlack)   
       glClearColor( 0, 0, 0, 1 );
    //else
    //   glClearColor( 1, 1, 1, 1 );
    
    // Clear the buffers
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    
#ifdef DRAW_LARGEIMAGE
    glFrustum( -1*aspect_ratio+width_per_node*col, -1*aspect_ratio+width_per_node*(col+1), 
               -1+height_per_node*(row), -1+height_per_node*(row+1), 2.0, 100 );
#else                   
    #if defined(CLIMATE)
        #if defined(ORTHO)
            glOrtho( -2, 2, -1, 1, 2.0, 100 );
        #else
            glFrustum( -2, 2, -1, 1, 2.0, 100 );
        #endif
    #else
        glFrustum( -1, 1, -1, 1, 2.0, 100 );
    #endif
#endif    

    	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    
	glScalef(m_scale,m_scale, 1);
    glTranslatef(0,0, -5.0);
    
	// rotate scale and translate 	
	glTranslatef(m_deltax, m_deltay, 0);
    	float m[4][4];   
	build_rotmatrix(m, m_curquat);
	glMultMatrixf(&(m[0][0]));
	
    static float angle = 0;    
#ifdef ROTATION
    #ifndef CLIMATE
        
        if (m_bRotation) {
            glRotatef(angle, 1, 0, 0);
            angle += 1;
                
            if (angle > 360) {
                angle = 0;
                m_bRotation = false;
            }
            
            cout << "angle " << angle << endl;
        }
    #else        
        static float dir = 1;
        
        if (m_bRotation) {
            glRotatef(angle, -1, 0, 0);
            angle += dir;
                
            if (angle > 135) {
                dir *= -1;            
            }
            
            if (angle == -1) {
                m_bRotation = false;
            }
            cout << "angle " << angle << endl;
        }
    #endif
#endif

#ifdef CUT    
    
    if (m_bCut){
    
        glRotatef(angle, -1, 0, 0);
        
        if (angle < 45 && m_pVolume->m_pVisStatus->m_vRangeMin[1] == 0) {
            angle++;
        }
        else if (angle == 45 && m_pVolume->m_pVisStatus->m_vRangeMin[1] < 0.5) {
            m_pVolume->m_pVisStatus->m_vRangeMin[1] -= -0.01;
        }
        else if (angle < 90 && m_pVolume->m_pVisStatus->m_vRangeMin[1] >= 0.5) {
            angle++;
        }        
        else {
            m_bCut = false;
        }
        
    }
    
#endif


#ifdef TIMING
    clock_t start,end;
    start = clock();
#endif
        
        
        
#ifdef MULTI_VARIABLES
    ShowMultiObject();
#else
	ShowObject();
#endif
    
    glFinish(); 
    
#ifdef TIMING       
    end = clock();
    double fps =   (double)(CLOCKS_PER_SEC) / (end - start);    
    cerr << "fps " << fps << endl;
#endif
            
    SaveView();
    
#ifdef WIN_SLICE_EXTRA    
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
        
    m_nImagewidth = viewport[2]; 
    m_nImageheight = viewport[3];
    long buffersize = m_nImagewidth*m_nImageheight*3;
	if (m_pImage){
        delete [] m_pImage;//delete [] m_pImage;
		m_pImage = NULL;
	}
	m_pImage = new unsigned char[buffersize];    
    glReadPixels(0, 0, viewport[2], viewport[3], GL_RGB, GL_UNSIGNED_BYTE, m_pImage); 
#endif
    
#ifdef SAVE_SINGLE_IMAGE    
    if (m_bSaveImage)
    {
        SaveImage();   
        m_bSaveImage = false;
    }
#endif

#ifdef ROTATION
    if (m_bRotation)
    {
        SaveImage();
    }
#endif

#ifdef CUT
    if (m_bCut)
    {
        SaveImage();
    }
#endif

    if (m_bPlayOn)
    {
        SaveImage();
        
        if (m_bEnd)
            m_bPlayOn = false;
    }

#ifdef DRAW_LARGEIMAGE
    m_SaveImage.readBuffer();
    
    unsigned char * pImage = m_SaveImage.getBuffer();
    
    for (int y = 0; y < tile_image_y; y++)
        for (int x = 0; x < tile_image_x; x++) {
            int src = x + y * tile_image_x;
            int des = x + col * tile_image_x + ((tile_image_y - y - 1) + row * tile_image_y) * final_image_x;
            final_image_ptr[des * 3 + 0] = pImage[src * 3 + 0];
            final_image_ptr[des * 3 + 1] = pImage[src * 3 + 1];
            final_image_ptr[des * 3 + 2] = pImage[src * 3 + 2];
        }//end for
     
     }//Draw tiles
    
    cout << "Save save_final.tga" << endl;
    
    SaveTga("save_final.tga", final_image_ptr, final_image_x, final_image_y);
    
#endif
    m_nFrameNum++;
}

void WinVisView::SaveImage()
{
        char filename[1024];
        sprintf(filename, "image/save_%05d", m_nImageCount);
        cout << "Save " << filename << endl;
        string file_str(filename);
        m_SaveImage.readBuffer();
        m_SaveImage.saveImage(file_str, IMAGE_TGA);        
        m_nImageCount++;    
}

void  WinVisView::SaveTga(char *filename, const unsigned char *image, int image_x, int image_y)
{

    static int32_t tgaimagesize = 0;
    static unsigned char *tgaimage = NULL;

    A3D_TGA_HEADER     tgaHeader;
    
    int imagesize;

    int16_t  i, j;
    FILE  *fp;
    
    assert(image);
    

    // set the tga header
    memset(&tgaHeader, 0, sizeof(A3D_TGA_HEADER));
    tgaHeader.nID = 0x0;
    tgaHeader.nColorMapType = 0x0;
    tgaHeader.nImageType = 0x2;
    tgaHeader.nColorMapOrigin = 0x0;
    tgaHeader.nColorMapLength = 0x0;
    tgaHeader.nColorMapWidth = 0x0;
    tgaHeader.nXOrigin = 0x0;
    tgaHeader.nYOrigin = 0x0;
    tgaHeader.nImageWidth = image_x;
    tgaHeader.nImageHeight = image_y;
    tgaHeader.nFlags = 0x18;

    // fill the image
    imagesize = image_x * image_y;

    if ((tgaimage == NULL) || (imagesize != tgaimagesize)) {

        if (tgaimage != NULL) {
            free(tgaimage);
        }

        tgaimage = (unsigned char *)
                calloc(sizeof(unsigned char), imagesize * 3);

        assert(tgaimage);
        
        tgaimagesize = imagesize;
    }

    memset(tgaimage, 0, sizeof(unsigned char) * imagesize * 3);


    for (i = image_y - 1; i >= 0; i--)
        for (j = 0; j < image_x; j++) {
        int n = i * image_x + j;
    
        tgaimage[n * 3]     = (unsigned char)(image[n * 3 + 2]);
        tgaimage[n * 3 + 1] = (unsigned char)(image[n * 3 + 1]);
        tgaimage[n * 3 + 2] = (unsigned char)(image[n * 3 + 0]);
    }


        fp = fopen(filename, "w+");
        assert(fp);
        
        if ((fwrite(&tgaHeader, sizeof(A3D_TGA_HEADER), 1, fp) != 1) ||
             (fwrite(tgaimage, sizeof(unsigned char) * imagesize * 3, 1, fp) != 1)){
            cerr << "Error writing tga file" << endl;
        }

        fclose(fp);

    return;
}

#ifndef MULTI_VARIABLES
void WinVisView::ShowObject()
{
	if(m_pVolume)
		m_pVolume->Draw();  
        
    if (m_bPlayOn) {
        m_pVolume->Forward();
        m_pVolume->ClearVolTex();            
    }
}
#endif


#ifdef MULTI_VARIABLES
void WinVisView::ShowMultiObject()
{
    if (m_vVolume.size() == 0)
        return;
        
    int volnum = m_vVolume.size();
    
    // read volume
    for (int i = 0; i < volnum; i++) {
        
        if (m_vTexVol[i] == 0) {
            m_vVolume[i]->ClearVolume();
            
            m_vVolume[i]->ReadFile();       
            
            CreateDataTex(i);
                        
            m_vVolume[i]->ClearVolume();
        }
    }           
    
    //initialize program
    if (m_nVolProgram == 0 || m_bReload)
        InitProgram();
                   
#if defined(CLIMATE)    
    glScalef(0.44, 1.0, 1.0);
#endif
                   
    m_vVolume[0]->GetEyePos();
    
    glPushMatrix();

    float scale = 1.0 / m_vVolume[0]->m_fMaxSize;
    glScalef(scale, scale, scale);      
    glTranslatef( -1 * m_vVolume[0]->m_vCenter[0], 
                  -1 * m_vVolume[0]->m_vCenter[1], 
                  -1 * m_vVolume[0]->m_vCenter[2]);
                  
    // read
    DrawVolume();
    
    if (m_vVolume[0]->m_pVisStatus->m_bDrawAxes)
        m_vVolume[0]->DrawAxes();
    
    if (m_vVolume[0]->m_pVisStatus->m_bDrawFrame)
        m_vVolume[0]->DrawFrame();    
        
    for (int i = 0; i < volnum; i++) {
        if (m_vVolume[i]->m_bDrawColorMap)
            m_vVolume[i]->DrawColorMap();        
    }
        
    // Forward
    if (m_bPlayOn) {
        for (int i = 0; i < volnum; i++) {   
            m_vVolume[i]->Forward();
            glDeleteTextures(1, (const GLuint*)&m_vTexVol[i]);
            m_vTexVol[i] = 0;
        }
    }

    
    glPopMatrix();
}

void WinVisView::InitProgram()
{
    GLchar *VertexShaderSource, *FragmentShaderSource;
    readShaderSource("myMultiVOL", &VertexShaderSource, &FragmentShaderSource);
    int success = installShaders(VertexShaderSource, FragmentShaderSource, (GLuint*)&(m_nVolProgram));        
    if ( !success)
    {
        fprintf(stderr, "Error: Can not install shaders.\n");   
    }
    free(VertexShaderSource);
    free(FragmentShaderSource);
    
    m_bReload = false;
}

void WinVisView::CreateTfTex(int vol)
{
    if (m_vTexTF[vol] == 0){
        GLuint nTexTF;
        glGenTextures(1, (GLuint*)&nTexTF);
        m_vTexTF[vol] = nTexTF;
    }
        
    glActiveTexture( GL_TEXTURE4 + vol );
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, m_vTexTF[vol]);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);    
    
    static float * spacingAdjustedTF = new float[TF_SIZE*4];
    
    for(int i = 0;i < TF_SIZE; i++)
    {
        float alpha = m_vVolume[vol]->m_pVisStatus->m_Colors[i].a();
        alpha = 1 - pow((1-alpha), m_vVolume[0]->m_pVisStatus->m_fSampleSpacing / m_vVolume[0]->m_fNormalSpacing ); 

        spacingAdjustedTF[i*4+0] = m_vVolume[vol]->m_pVisStatus->m_Colors[i].r() * alpha;
        spacingAdjustedTF[i*4+1] = m_vVolume[vol]->m_pVisStatus->m_Colors[i].g() * alpha;
        spacingAdjustedTF[i*4+2] = m_vVolume[vol]->m_pVisStatus->m_Colors[i].b() * alpha;
        spacingAdjustedTF[i*4+3] = alpha;
    }
    
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA_FLOAT16_ATI, TF_SIZE, 0, GL_RGBA, GL_FLOAT, spacingAdjustedTF);
            
    glDisable(GL_TEXTURE_1D);

}

void WinVisView::CreateDataTex(int vol)
{
    int volnum = m_vVolume.size();
    
    if(m_vTexVol[vol]==0)
    {
        GLuint nTexVol;
        glGenTextures(1, (GLuint*)&nTexVol);
        m_vTexVol[vol] = nTexVol;
        
        glActiveTexture(GL_TEXTURE4 + volnum + vol);
        glEnable(GL_TEXTURE_3D);
        glBindTexture(GL_TEXTURE_3D, m_vTexVol[vol]);
        glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        
        if (m_vVolume[vol]->m_pVolume == NULL) {
            cerr << "The Volume is empty" << endl;
            return;
        }
        
        glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE16, 
                     (int)m_vVolume[vol]->m_vSize[0],
                     (int)m_vVolume[vol]->m_vSize[1], 
                     (int)m_vVolume[vol]->m_vSize[2], 
                     0, GL_LUMINANCE, GL_FLOAT,
                     m_vVolume[vol]->m_pVolume);
                     
        glDisable(GL_TEXTURE_3D);
    }else{
    
        glActiveTexture( GL_TEXTURE4 + volnum + vol);
        glEnable(GL_TEXTURE_3D);
        glBindTexture(GL_TEXTURE_3D, m_vTexVol[vol]);
        glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glDisable(GL_TEXTURE_3D);
    }
}

void WinVisView::DrawVolume()
{
    int volnum = m_vVolume.size();
    
    for (int i = 0; i < volnum; i++) {
        CreateTfTex(i);
        CreateDataTex(i);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
    

    glUseProgram(m_nVolProgram);    
    
    if (m_vVolume[0]->m_pVisStatus->m_bDrawVolume)
        glUniform1i(glGetUniformLocation(m_nVolProgram, "drawvol0"), 
                1);
    else
        glUniform1i(glGetUniformLocation(m_nVolProgram, "drawvol0"), 
                0);
                
    if (m_vVolume[1]->m_pVisStatus->m_bDrawVolume)
        glUniform1i(glGetUniformLocation(m_nVolProgram, "drawvol1"), 
                1);
    else
        glUniform1i(glGetUniformLocation(m_nVolProgram, "drawvol1"), 
                0);
    
    glUniform1i(glGetUniformLocation(m_nVolProgram, "tfTex0"), 
                4);
    
    glUniform1i(glGetUniformLocation(m_nVolProgram, "tfTex1"), 
                4 + 1);
                                
    glUniform1i(glGetUniformLocation(m_nVolProgram, "volumeTex0"), 
                4 + volnum);    
                    
    glUniform1i(glGetUniformLocation(m_nVolProgram, "volumeTex1"), 
                4 + volnum + 1);    
                
    glUniform4f(glGetUniformLocation(m_nVolProgram, "lightPar0"), 
                m_vVolume[0]->m_pVisStatus->m_fLightPar[0],  
                m_vVolume[0]->m_pVisStatus->m_fLightPar[1], 
                m_vVolume[0]->m_pVisStatus->m_fLightPar[2],
                m_vVolume[0]->m_pVisStatus->m_fLightPar[3]);
    
    glUniform4f(glGetUniformLocation(m_nVolProgram, "lightPar1"), 
                m_vVolume[1]->m_pVisStatus->m_fLightPar[0],  
                m_vVolume[1]->m_pVisStatus->m_fLightPar[1], 
                m_vVolume[1]->m_pVisStatus->m_fLightPar[2],
                m_vVolume[1]->m_pVisStatus->m_fLightPar[3]);
    
    glUniform1f(glGetUniformLocation(m_nVolProgram, "sampleSpacing"),  
                m_vVolume[0]->m_pVisStatus->m_fSampleSpacing);
                
                
    glUniform3f(glGetUniformLocation(m_nVolProgram, "eyePos"), 
                m_vVolume[0]->m_vEye[0], 
                m_vVolume[0]->m_vEye[1], 
                m_vVolume[0]->m_vEye[2]);
        
    float centralDifferenceSpacing[3];    
    for(int i = 0; i < 3; i++)
        centralDifferenceSpacing[i] = 1.0 / m_vVolume[0]->m_vSize[i];

    glUniform3f(glGetUniformLocation(m_nVolProgram, "centralDifferenceSpacing"), 
                centralDifferenceSpacing[0],centralDifferenceSpacing[1],centralDifferenceSpacing[2]);    
    


    float minx = m_vVolume[0]->m_pVisStatus->m_vRangeMin[0];
    float miny = m_vVolume[0]->m_pVisStatus->m_vRangeMin[1];
    float minz = m_vVolume[0]->m_pVisStatus->m_vRangeMin[2];

    float maxx = m_vVolume[0]->m_pVisStatus->m_vRangeMax[0];
    float maxy = m_vVolume[0]->m_pVisStatus->m_vRangeMax[1];
    float maxz = m_vVolume[0]->m_pVisStatus->m_vRangeMax[2]; 
    
    float t[8][3] = {{minx,miny,minz},{maxx,miny,minz},{minx,maxy,minz},{maxx,maxy,minz},
                     {minx,miny,maxz},{maxx,miny,maxz},{minx,maxy,maxz},{maxx,maxy,maxz}};

    minx = minx * m_vVolume[0]->m_vSize[0];
    miny = miny * m_vVolume[0]->m_vSize[1];
    minz = minz * m_vVolume[0]->m_vSize[2];

    maxx = maxx * m_vVolume[0]->m_vSize[0];
    maxy = maxy * m_vVolume[0]->m_vSize[1];
    maxz = maxz * m_vVolume[0]->m_vSize[2];

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
    
    m_vVolume[0]->TurnOffTextureAndPrograms();
    
    glDisable(GL_BLEND);
    
    
}

#endif


int WinVisView::handle(int event)
{
	switch(event) {
	case FL_DRAG:
		m_bOperating = true;
		if(Fl::event_button1() && !Fl::event_button2() && !Fl::event_button3()) //Drag with left botton to rotate
			Rotate();
		else if(!Fl::event_button1() && (Fl::event_button2() || Fl::event_button3())) //Drag with mid button or right botton to scale
			Scale();
		else if(Fl::event_button1() && (Fl::event_button2() || Fl::event_button3())) //Drag with more than two button to move
			Translate();

		return 1;
	case FL_PUSH:
		m_mousex = Fl::event_x();
		m_mousey = Fl::event_y();
		return 1;
	case FL_RELEASE:
		m_bOperating = false;
		redraw();
		return 1;
	case FL_KEYBOARD:
		if(Fl::event_key()==FL_Left)
		{
			redraw();
		}		
        if(Fl::event_key()=='r')
        {
#ifdef MULTI_VARIABLES
            m_bReload = true;
#else
            m_pVolume->m_bReload = true;
#endif       
            redraw();
        }   
        if(Fl::event_key()=='b')
        {
            m_bBlack = !m_bBlack;
            redraw();
        }   
        if(Fl::event_key()=='s')
        {
            m_bSaveImage = true;            
            redraw();
        } 
        if(Fl::event_key()=='r')
        {
            m_bRotation = !m_bRotation;
            
            if (m_bRotation)
                Fl::add_timeout(ROTATION_TIME, cb_wRedraw, this);
            else
                Fl::remove_timeout(cb_wRedraw, this);
            redraw();
        }
        if(Fl::event_key()=='c')
        {
            m_bCut = !m_bCut;
            
            if (m_bCut)
                Fl::add_timeout(ROTATION_TIME, cb_wRedraw, this);
            else
                Fl::remove_timeout(cb_wRedraw, this);
            redraw();
        }
        if(Fl::event_key()=='p')
        {
            m_bPlayOn = !m_bPlayOn;
            
            if (m_bPlayOn)
                Fl::add_timeout(ROTATION_TIME, cb_wRedraw, this);
            else
                Fl::remove_timeout(cb_wRedraw, this);
            redraw();
        }
		return 1;
	default:
		// pass other events to the base class...
		return Fl_Gl_Window::handle(event);
	}
}

void WinVisView::Rotate()
{
	int x = Fl::event_x();
	int y = Fl::event_y();

	float lastquat[4];
	if (m_mousex != x || m_mousey != y) 
	{
		trackball(lastquat,
			(2.0*m_mousex - w()) / w(),
			(h() - 2.0*m_mousey) / h(),
			(2.0*x - w()) / w(),
			(h() - 2.0*y) / h());		
		
		add_quats(lastquat, m_curquat, m_curquat);
		m_mousex = x;
		m_mousey = y;
		redraw();
	}	
}

void WinVisView:: Translate()
{
	int x = Fl::event_x();
	int y = Fl::event_y();

  	if (m_mousex != x || m_mousey != y) 
  	{
		m_deltax += ((float)(x-m_mousex )) * 0.005;
		m_deltay +=((float) (m_mousey-y)) * 0.005;

		m_mousex = x;
		m_mousey = y;

		redraw();
	}	
		
}

void WinVisView::Scale()
{
	int x = Fl::event_x();
	int y = Fl::event_y();

  	if (m_mousex != x || m_mousey != y) 
  	{
		m_scale  += ((float)(m_mousey - y))/h() * 1.3 * m_scale;
		if(m_scale <= 0)
			m_scale = 0.00000001f;
		m_mousex = x;
		m_mousey = y;
		
		redraw();
	}		
}

void WinVisView::SaveView()
{
#ifdef MULTI_VARIABLES    
    memcpy(m_vVolume[0]->m_pVisStatus->m_curquat, m_curquat, sizeof(float) * 4);
    m_vVolume[0]->m_pVisStatus->m_scale = m_scale;
    m_vVolume[0]->m_pVisStatus->m_deltax = m_deltax;
    m_vVolume[0]->m_pVisStatus->m_deltay = m_deltay;
    m_vVolume[0]->m_pVisStatus->WriteToFile();

    memcpy(m_vVolume[1]->m_pVisStatus->m_curquat, m_curquat, sizeof(float) * 4);
    m_vVolume[1]->m_pVisStatus->m_scale = m_scale;
    m_vVolume[1]->m_pVisStatus->m_deltax = m_deltax;
    m_vVolume[1]->m_pVisStatus->m_deltay = m_deltay;
    m_vVolume[1]->m_pVisStatus->WriteToFile();

    ofstream outf("ctrlview.cfg");
    assert(outf);
    
    outf.write(reinterpret_cast<char *>(m_curquat), sizeof(float)*4);
    outf.write(reinterpret_cast<char *>(&m_scale), sizeof(float));
    outf.write(reinterpret_cast<char *>(&m_deltax), sizeof(float));
    outf.write(reinterpret_cast<char *>(&m_deltay), sizeof(float));
    
    outf.close();
    
#else
    memcpy(m_pVolume->m_pVisStatus->m_curquat, m_curquat, sizeof(float) * 4);
    m_pVolume->m_pVisStatus->m_scale = m_scale;
    m_pVolume->m_pVisStatus->m_deltax = m_deltax;
    m_pVolume->m_pVisStatus->m_deltay = m_deltay;
    
    ofstream outf("ctrlview.cfg");
    assert(outf);
    
    outf.write(reinterpret_cast<char *>(m_curquat), sizeof(float)*4);
    outf.write(reinterpret_cast<char *>(&m_scale), sizeof(float));
    outf.write(reinterpret_cast<char *>(&m_deltax), sizeof(float));
    outf.write(reinterpret_cast<char *>(&m_deltay), sizeof(float));
    
    outf.close();
#endif
}

void WinVisView::LoadView()
{   
    ifstream inf("ctrlview.cfg");
    
    if (inf) {
        inf.read(reinterpret_cast<char *>(m_curquat), sizeof(float)*4);
        inf.read(reinterpret_cast<char *>(&m_scale), sizeof(float));
        inf.read(reinterpret_cast<char *>(&m_deltax), sizeof(float));
        inf.read(reinterpret_cast<char *>(&m_deltay), sizeof(float));
        inf.close();
    }
}

void cb_wRedraw(void * pData)
{
    if (pData == NULL)
        return;
        
    /* get the contrl point*/
    WinVisView* pView = reinterpret_cast<WinVisView*>(pData);        
    
    pView->redraw();
    
    Fl::repeat_timeout(ROTATION_TIME, cb_wRedraw, pData);

}



