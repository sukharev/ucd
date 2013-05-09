//#define MERCATOR_TEST
//#define MAIN
//#define LATLON_PROJ
#define _ATMOSPHERE
#ifdef _ATMOSPHERE
/**
 * Precomputed Atmospheric Scattering
 * Copyright (c) 2008 INRIA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Author: Eric Bruneton
 */

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>

//#include <GL/glew.h>
//#include <glee.h>
//#ifdef __gl_h_
//#undef __gl_h_
//#endif

#include "GLTools.h"
//#include <GL/glut.h>
//#include <GL/glext.h>
#include "tiffio.h"

#include "vec3.h"
#include "mat4.h"
#include "Main.h"
#include "utils.h"
using namespace std;


// ----------------------------------------------------------------------------
// TOOLS
// ----------------------------------------------------------------------------
void displayTexture();

void loadTIFF(char *name, unsigned char *tex)
{
    tstrip_t strip = 0;
    tsize_t off = 0;
    tsize_t n = 0;
    TIFF* tf = TIFFOpen(name, "r");
    while ((n = TIFFReadEncodedStrip(tf, strip, tex + off, (tsize_t) -1)) > 0) {
    	strip += 1;
        off += n;
    };
    TIFFClose(tf);
}

string* loadFile(const string &fileName)
{
    string* result = new string();
    ifstream file(fileName.c_str());
    if (!file) {
        std::cerr << "Cannot open file " << fileName << endl;
        throw exception();
    }
    string line;
    while (getline(file, line)) {
        *result += line;
        *result += '\n';
    }
    file.close();
    return result;
}

void printShaderLog(int shaderId)
{
    int logLength;
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        char *log = new char[logLength];
        glGetShaderInfoLog(shaderId, logLength, &logLength, log);
        cout << string(log);
        delete[] log;
    }
}

unsigned int loadProgram(const vector<string> &files)
{
    unsigned int programId = glCreateProgram();
    unsigned int vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    int n = files.size();
    string **strs = new string*[n];
    const char** lines = new const char*[n + 1];
    cout << "loading program " << files[n - 1] << "..." << endl;
    bool geo = false;
    for (int i = 0; i < n; ++i) {
        string* s = loadFile(files[i]);
        strs[i] = s;
        lines[i + 1] = s->c_str();
        if (strstr(lines[i + 1], "_GEOMETRY_") != NULL) {
            geo = true;
        }
    }

    lines[0] = "#define _VERTEX_\n";
    glShaderSource(vertexShaderId, n + 1, lines, NULL);
    glCompileShader(vertexShaderId);
    printShaderLog(vertexShaderId);

    if (geo) {
        unsigned geometryShaderId = glCreateShader(GL_GEOMETRY_SHADER_EXT);
        glAttachShader(programId, geometryShaderId);
        lines[0] = "#define _GEOMETRY_\n";
        glShaderSource(geometryShaderId, n + 1, lines, NULL);
        glCompileShader(geometryShaderId);
        printShaderLog(geometryShaderId);
        glProgramParameteriEXT(programId, GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
        glProgramParameteriEXT(programId, GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
        glProgramParameteriEXT(programId, GL_GEOMETRY_VERTICES_OUT_EXT, 3);
    }

    lines[0] = "#define _FRAGMENT_\n";
    glShaderSource(fragmentShaderId, n + 1, lines, NULL);
    glCompileShader(fragmentShaderId);
    printShaderLog(fragmentShaderId);

    glLinkProgram(programId);

    for (int i = 0; i < n; ++i) {
        delete strs[i];
    }
    delete[] strs;
    delete[] lines;

    return programId;
}

void drawQuad()
{
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(-1.0, -1.0);
    glVertex2f(+1.0, -1.0);
    glVertex2f(-1.0, +1.0);
    glVertex2f(+1.0, +1.0);
    glEnd();
}

// ----------------------------------------------------------------------------
// PRECOMPUTATIONS
// ----------------------------------------------------------------------------

const int reflectanceUnit = 0;
const int transmittanceUnit = 1;
const int irradianceUnit = 2;
const int inscatterUnit = 3;
const int deltaEUnit = 4;
const int deltaSRUnit = 5;
const int deltaSMUnit = 6;
const int deltaJUnit = 7;
const int drawUnit = 8;

unsigned int reflectanceTexture;//unit 0, ground reflectance texture
unsigned int transmittanceTexture;//unit 1, T table
unsigned int irradianceTexture;//unit 2, E table
unsigned int inscatterTexture;//unit 3, S table
unsigned int deltaETexture;//unit 4, deltaE table
unsigned int deltaSRTexture;//unit 5, deltaS table (Rayleigh part)
unsigned int deltaSMTexture;//unit 6, deltaS table (Mie part)
unsigned int deltaJTexture;//unit 7, deltaJ table
unsigned int drawTexture;//unit 8, deltaJ table

unsigned int transmittanceProg;
unsigned int irradiance1Prog;
unsigned int inscatter1Prog;
unsigned int copyIrradianceProg;
unsigned int copyInscatter1Prog;
unsigned int jProg;
unsigned int irradianceNProg;
unsigned int inscatterNProg;
unsigned int copyInscatterNProg;

unsigned int fbo;

unsigned int drawTxtProg;
unsigned int drawProg;

void setLayer(unsigned int prog, int layer)
{
    double r = layer / (RES_R - 1.0);
    r = r * r;
    r = sqrt(Rg * Rg + r * (Rt * Rt - Rg * Rg)) + (layer == 0 ? 0.01 : (layer == RES_R - 1 ? -0.001 : 0.0));
    double dmin = Rt - r;
    double dmax = sqrt(r * r - Rg * Rg) + sqrt(Rt * Rt - Rg * Rg);
    double dminp = r - Rg;
    double dmaxp = sqrt(r * r - Rg * Rg);
    glUniform1f(glGetUniformLocation(prog, "r"), float(r));
    glUniform4f(glGetUniformLocation(prog, "dhdH"), float(dmin), float(dmax), float(dminp), float(dmaxp));
    glUniform1i(glGetUniformLocation(prog, "layer"), layer);
}

void loadData()
{
    cout << "loading Earth texture..." << endl;
    glActiveTexture(GL_TEXTURE0 + reflectanceUnit);
    glGenTextures(1, &reflectanceTexture);
    glBindTexture(GL_TEXTURE_2D, reflectanceTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    unsigned char *tex = new unsigned char[2500 * 1250 * 4];
    //unsigned char *tex = new unsigned char[5400 * 2700 * 4];
	//unsigned char *tex = new unsigned char[2642 * 1488 * 4];
	loadTIFF("C:\\SVN\\Dev\\inria\\PrecomputedAtmosphericScattering\\earth.tiff", tex);
	//loadTIFF("earth_b.tiff", tex);
	//loadTIFF("test.tiff", tex);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2500, 1250, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 5400, 2700, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2642, 1488, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
    glGenerateMipmapEXT(GL_TEXTURE_2D);
    delete[] tex;
}

#define MAX_CHAR 512
void LoadTexture3D()
{
	int size = 3;
	float *dataRGBA=new float[size*RES_R*RES_MU_S * RES_NU *RES_MU];

#ifdef TEST_INSCATTER
	float val_r = 1.0;
	float val_g = 0.0;
	float val_b = 0.0;
	
	float step = 1.0/(float)RES_R;
	for (int i=0; i<RES_R; i++)
	{
		for(int j=0; j < RES_MU_S * RES_NU; j++){
			for(int k=0; k < RES_MU; k++){
				dataRGBA[i*RES_MU_S * RES_NU*RES_MU*size + j*RES_MU*size+k*size]=val_r;
				dataRGBA[i*RES_MU_S * RES_NU*RES_MU*size + j*RES_MU*size+k*size+1]=val_g;
				dataRGBA[i*RES_MU_S * RES_NU*RES_MU*size + j*RES_MU*size+k*size+2]=val_b;
				dataRGBA[i*RES_MU_S * RES_NU*RES_MU*size + j*RES_MU*size+k*size+3]=1.0;
			}
		}
		val_r = val_r - step;
		val_g = val_g + step;
	}
#else
	for (int i=0; i<RES_R; i++)
	{
		char filename[MAX_CHAR];
#ifdef MODTRAN_PPM
	    sprintf(filename, "C:\\jeffs\\mod53ucdavis\\pyradi\\pyradi-read-only\\ppm\\pas_python_%d.ppm", i);
		//sprintf(filename, "C:\\jeffs\\mod53ucdavis\\pyradi\\pyradi-read-only\\ppm\\pas_python_%d.ppm", i);
#else
		sprintf(filename, "C:\\temp\\pas\\pas_%d.ppm", i);
#endif		
		PPMImage * img = readPPM(filename);
		for(int j=0; j < RES_MU; j++){
			for(int k=0; k < RES_MU_S * RES_NU; k++){
				int ind = j*RES_MU_S*RES_NU + k;
				//if(i != RES_R - 1){
					dataRGBA[i*RES_MU_S * RES_NU*RES_MU*size + j*RES_MU_S * RES_NU*size+k*size]=(float)(img->data[ind].red) / (float)255;
					dataRGBA[i*RES_MU_S * RES_NU*RES_MU*size + j*RES_MU_S * RES_NU*size+k*size+1]=(float)(img->data[ind].green)/(float)255;
					dataRGBA[i*RES_MU_S * RES_NU*RES_MU*size + j*RES_MU_S * RES_NU*size+k*size+2]=(float)(img->data[ind].blue)/(float)255;
				//}
				//else{
				//	dataRGBA[i*RES_MU_S * RES_NU*RES_MU*size + j*RES_MU_S * RES_NU*size+k*size]=0.0;
				//	dataRGBA[i*RES_MU_S * RES_NU*RES_MU*size + j*RES_MU_S * RES_NU*size+k*size+1]=0.0;
				//	dataRGBA[i*RES_MU_S * RES_NU*RES_MU*size + j*RES_MU_S * RES_NU*size+k*size+2]=0.0;
				//}
				//dataRGBA[i*RES_MU_S * RES_NU*RES_MU*size + j*RES_MU*size+k*size+3]=1.0;
			}
		}
	}
#endif


	glActiveTexture(GL_TEXTURE0 + inscatterUnit);
    glGenTextures(1, &inscatterTexture);
    glBindTexture(GL_TEXTURE_3D, inscatterTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F_ARB, RES_MU_S * RES_NU, RES_MU, RES_R, 0, GL_RGB, GL_FLOAT, dataRGBA);
}



void precompute(bool bLoadInScatter = false)
{
	opengl_version();

    glActiveTexture(GL_TEXTURE0 + transmittanceUnit);
    glGenTextures(1, &transmittanceTexture);
    glBindTexture(GL_TEXTURE_2D, transmittanceTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F_ARB, TRANSMITTANCE_W, TRANSMITTANCE_H, 0, GL_RGB, GL_FLOAT, NULL);

    glActiveTexture(GL_TEXTURE0 + irradianceUnit);
    glGenTextures(1, &irradianceTexture);
    glBindTexture(GL_TEXTURE_2D, irradianceTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F_ARB, SKY_W, SKY_H, 0, GL_RGB, GL_FLOAT, NULL);

#ifdef BUILD_INSCATTER
    glActiveTexture(GL_TEXTURE0 + inscatterUnit);
    glGenTextures(1, &inscatterTexture);
    glBindTexture(GL_TEXTURE_3D, inscatterTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F_ARB, RES_MU_S * RES_NU, RES_MU, RES_R, 0, GL_RGB, GL_FLOAT, NULL);
#endif

	glActiveTexture(GL_TEXTURE0 + drawUnit);
    glGenTextures(1, &drawTexture);
    glBindTexture(GL_TEXTURE_2D, drawTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F_ARB, 764, 764, 0, GL_RGB, GL_FLOAT, NULL);


	glActiveTexture(GL_TEXTURE0 + deltaEUnit);
    glGenTextures(1, &deltaETexture);
    glBindTexture(GL_TEXTURE_2D, deltaETexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F_ARB, SKY_W, SKY_H, 0, GL_RGB, GL_FLOAT, NULL);

    glActiveTexture(GL_TEXTURE0 + deltaSRUnit);
    glGenTextures(1, &deltaSRTexture);
    glBindTexture(GL_TEXTURE_3D, deltaSRTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F_ARB, RES_MU_S * RES_NU, RES_MU, RES_R, 0, GL_RGB, GL_FLOAT, NULL);

    glActiveTexture(GL_TEXTURE0 + deltaSMUnit);
    glGenTextures(1, &deltaSMTexture);
    glBindTexture(GL_TEXTURE_3D, deltaSMTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F_ARB, RES_MU_S * RES_NU, RES_MU, RES_R, 0, GL_RGB, GL_FLOAT, NULL);

    glActiveTexture(GL_TEXTURE0 + deltaJUnit);
    glGenTextures(1, &deltaJTexture);
    glBindTexture(GL_TEXTURE_3D, deltaJTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F_ARB, RES_MU_S * RES_NU, RES_MU, RES_R, 0, GL_RGB, GL_FLOAT, NULL);

    vector<string> files;
    files.push_back("Main.h");
    files.push_back("common.glsl");
    files.push_back("transmittance.glsl");
    transmittanceProg = loadProgram(files);

    files.clear();
    files.push_back("Main.h");
    files.push_back("common.glsl");
    files.push_back("irradiance1.glsl");
    irradiance1Prog = loadProgram(files);

    files.clear();
    files.push_back("Main.h");
    files.push_back("common.glsl");
    files.push_back("inscatter1.glsl");
    inscatter1Prog = loadProgram(files);

    files.clear();
    files.push_back("Main.h");
    files.push_back("common.glsl");
    files.push_back("copyIrradiance.glsl");
    copyIrradianceProg = loadProgram(files);

    files.clear();
    files.push_back("Main.h");
    files.push_back("common.glsl");
    files.push_back("copyInscatter1.glsl");
    copyInscatter1Prog = loadProgram(files);

    files.clear();
    files.push_back("Main.h");
    files.push_back("common.glsl");
    files.push_back("inscatterS.glsl");
    jProg = loadProgram(files);

    files.clear();
    files.push_back("Main.h");
    files.push_back("common.glsl");
    files.push_back("irradianceN.glsl");
    irradianceNProg = loadProgram(files);

    files.clear();
    files.push_back("Main.h");
    files.push_back("common.glsl");
    files.push_back("inscatterN.glsl");
    inscatterNProg = loadProgram(files);

    files.clear();
    files.push_back("Main.h");
    files.push_back("common.glsl");
    files.push_back("copyInscatterN.glsl");
    copyInscatterNProg = loadProgram(files);

	files.clear();
    files.push_back("Main.h");
    files.push_back("common.glsl");
    files.push_back("earth1.glsl");
    drawTxtProg = loadProgram(files);
		

    files.clear();
    files.push_back("Main.h");
    files.push_back("common.glsl");
    files.push_back("earth.glsl");
    drawProg = loadProgram(files);
    glUseProgram(drawProg);
    glUniform1i(glGetUniformLocation(drawProg, "reflectanceSampler"), reflectanceUnit);
    glUniform1i(glGetUniformLocation(drawProg, "transmittanceSampler"), transmittanceUnit);
    glUniform1i(glGetUniformLocation(drawProg, "irradianceSampler"), irradianceUnit);
    glUniform1i(glGetUniformLocation(drawProg, "inscatterSampler"), inscatterUnit);
	glUniform1i(glGetUniformLocation(drawProg, "drawSampler"), drawUnit);
    cout << "precomputations..." << endl;

    glGenFramebuffersEXT(1, &fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

    // computes transmittance texture T (line 1 in algorithm 4.1)
    glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, transmittanceTexture, 0);
    glViewport(0, 0, TRANSMITTANCE_W, TRANSMITTANCE_H);
    glUseProgram(transmittanceProg);
    drawQuad();
	saveTextureToXYZ((GLenum)GL_COLOR_ATTACHMENT0_EXT, TRANSMITTANCE_W, TRANSMITTANCE_H, 3, "c:\\temp\\pas\\transmittance", GL_RGB, true);

    // computes irradiance texture deltaE (line 2 in algorithm 4.1)
    glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, deltaETexture, 0);
    glViewport(0, 0, SKY_W, SKY_H);
    glUseProgram(irradiance1Prog);
    glUniform1i(glGetUniformLocation(irradiance1Prog, "transmittanceSampler"), transmittanceUnit);
    drawQuad();

    // computes single scattering texture deltaS (line 3 in algorithm 4.1)
    // Rayleigh and Mie separated in deltaSR + deltaSM
    glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, deltaSRTexture, 0);
    glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, deltaSMTexture, 0);
    unsigned int bufs[2] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
    glDrawBuffers(2, bufs);
    glViewport(0, 0, RES_MU_S * RES_NU, RES_MU);
    glUseProgram(inscatter1Prog);
    glUniform1i(glGetUniformLocation(inscatter1Prog, "transmittanceSampler"), transmittanceUnit);
    for (int layer = 0; layer < RES_R; ++layer) {
        setLayer(inscatter1Prog, layer);
        drawQuad();
    }
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, 0, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

    // copies deltaE into irradiance texture E (line 4 in algorithm 4.1)
    glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, irradianceTexture, 0);
    glViewport(0, 0, SKY_W, SKY_H);
    glUseProgram(copyIrradianceProg);
    glUniform1f(glGetUniformLocation(copyIrradianceProg, "k"), 0.0);
    glUniform1i(glGetUniformLocation(copyIrradianceProg, "deltaESampler"), deltaEUnit);
    drawQuad();


#ifdef BUILD_INSCATTER
    // copies deltaS into inscatter texture S (line 5 in algorithm 4.1)
    glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, inscatterTexture, 0);
    glViewport(0, 0, RES_MU_S * RES_NU, RES_MU);
    glUseProgram(copyInscatter1Prog);
    glUniform1i(glGetUniformLocation(copyInscatter1Prog, "deltaSRSampler"), deltaSRUnit);
    glUniform1i(glGetUniformLocation(copyInscatter1Prog, "deltaSMSampler"), deltaSMUnit);
    for (int layer = 0; layer < RES_R; ++layer) {
        setLayer(copyInscatter1Prog, layer);
        drawQuad();
    }

	saveTextureToXYZ3D(GL_COLOR_ATTACHMENT0_EXT, inscatterTexture, RES_MU_S * RES_NU, RES_MU, RES_R, 4, "C:\\Temp\\pas\\pas", GL_RGBA, true);
	saveTextureToPPM(GL_COLOR_ATTACHMENT0_EXT, inscatterTexture, RES_MU_S * RES_NU, RES_MU, RES_R, 4, "C:\\Temp\\pas\\pas", GL_RGBA, true);
	
	displayTexture();
	saveTextureToXYZ((GLenum)GL_COLOR_ATTACHMENT0_EXT, 764, 764, 3, "c:\\temp\\pas\\earth", GL_RGB, true);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glFinish();
/*
    // loop for each scattering order (line 6 in algorithm 4.1)
    for (int order = 2; order <= 4; ++order) {

        // computes deltaJ (line 7 in algorithm 4.1)
        glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, deltaJTexture, 0);
        glViewport(0, 0, RES_MU_S * RES_NU, RES_MU);
        
		glUseProgram(jProg);
        
		glUniform1f(glGetUniformLocation(jProg, "first"), order == 2 ? 1.0 : 0.0);
        glUniform1i(glGetUniformLocation(jProg, "transmittanceSampler"), transmittanceUnit);
        glUniform1i(glGetUniformLocation(jProg, "deltaESampler"), deltaEUnit);
        glUniform1i(glGetUniformLocation(jProg, "deltaSRSampler"), deltaSRUnit);
        glUniform1i(glGetUniformLocation(jProg, "deltaSMSampler"), deltaSMUnit);
        
		for (int layer = 0; layer < RES_R; ++layer) {
            setLayer(jProg, layer);
            drawQuad();
        }
	    
        // computes deltaE (line 8 in algorithm 4.1)
        glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, deltaETexture, 0);
        glViewport(0, 0, SKY_W, SKY_H);
        glUseProgram(irradianceNProg);
        glUniform1f(glGetUniformLocation(irradianceNProg, "first"), order == 2 ? 1.0 : 0.0);
        glUniform1i(glGetUniformLocation(irradianceNProg, "transmittanceSampler"), transmittanceUnit);
        glUniform1i(glGetUniformLocation(irradianceNProg, "deltaSRSampler"), deltaSRUnit);
        glUniform1i(glGetUniformLocation(irradianceNProg, "deltaSMSampler"), deltaSMUnit);
        drawQuad();

        // computes deltaS (line 9 in algorithm 4.1)
        glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, deltaSRTexture, 0);
        glViewport(0, 0, RES_MU_S * RES_NU, RES_MU);
        glUseProgram(inscatterNProg);
        glUniform1f(glGetUniformLocation(inscatterNProg, "first"), order == 2 ? 1.0 : 0.0);
        glUniform1i(glGetUniformLocation(inscatterNProg, "transmittanceSampler"), transmittanceUnit);
        glUniform1i(glGetUniformLocation(inscatterNProg, "deltaJSampler"), deltaJUnit);
        for (int layer = 0; layer < RES_R; ++layer) {
            setLayer(inscatterNProg, layer);
            drawQuad();
        }

        glEnable(GL_BLEND);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

        // adds deltaE into irradiance texture E (line 10 in algorithm 4.1)
        glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, irradianceTexture, 0);
        glViewport(0, 0, SKY_W, SKY_H);
        glUseProgram(copyIrradianceProg);
        glUniform1f(glGetUniformLocation(copyIrradianceProg, "k"), 1.0);
        glUniform1i(glGetUniformLocation(copyIrradianceProg, "deltaESampler"), deltaEUnit);
        drawQuad();

        // adds deltaS into inscatter texture S (line 11 in algorithm 4.1)
        glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, inscatterTexture, 0);
        glViewport(0, 0, RES_MU_S * RES_NU, RES_MU);
        glUseProgram(copyInscatterNProg);
        glUniform1i(glGetUniformLocation(copyInscatterNProg, "deltaSSampler"), deltaSRUnit);
        for (int layer = 0; layer < RES_R; ++layer) {
            setLayer(copyInscatterNProg, layer);
            drawQuad();
        }
		
        glDisable(GL_BLEND);
		
    }
*/
#else //BUILD_INSCATTER
    LoadTexture3D();
#endif //BUILD_INSCATTER

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    glFinish();
    cout << "ready." << endl;
    glUseProgram(drawProg);
}

void recompute(bool bLoadInScatter = false)
{
    glDeleteTextures(1, &transmittanceTexture);
    glDeleteTextures(1, &irradianceTexture);
    glDeleteTextures(1, &inscatterTexture);
    glDeleteTextures(1, &deltaETexture);
    glDeleteTextures(1, &deltaSRTexture);
    glDeleteTextures(1, &deltaSMTexture);
    glDeleteTextures(1, &deltaJTexture);
	glDeleteTextures(1, &drawTexture);
    glDeleteProgram(transmittanceProg);
    glDeleteProgram(irradiance1Prog);
    glDeleteProgram(inscatter1Prog);
    glDeleteProgram(copyIrradianceProg);
    glDeleteProgram(copyInscatter1Prog);
    glDeleteProgram(jProg);
    glDeleteProgram(irradianceNProg);
    glDeleteProgram(inscatterNProg);
    glDeleteProgram(copyInscatterNProg);
    glDeleteProgram(drawTxtProg);
	glDeleteProgram(drawProg);
    glDeleteFramebuffersEXT(1, &fbo);
    precompute(bLoadInScatter);
}

// ----------------------------------------------------------------------------
// RENDERING
// ----------------------------------------------------------------------------

int width, height;
int oldx, oldy;
int move;

vec3f s(0.0, 0.0, -1.0); //sun position
//vec3f s(0.0, -1.0, 0.0);

double lon = 0.0;
double lat = 0.0;
double theta = 0.0;
double phi = 0.0;
double d = Rg;
vec3d position;
mat4d view;
mat4d view1;

double exposure = 0.4;

void updateView()
{
	double co = cos(lon);
	double so = sin(lon);
	double ca = cos(lat);
	double sa = sin(lat);
	vec3d po = vec3d(co*ca, so*ca, sa) * Rg;
	vec3d px = vec3d(-so, co, 0);
    vec3d py = vec3d(-co*sa, -so*sa, ca);
    vec3d pz = vec3d(co*ca, so*ca, sa);

    double ct = cos(theta);
    double st = sin(theta);
    double cp = cos(phi);
    double sp = sin(phi);
    vec3d cx = px * cp + py * sp;
    vec3d cy = -px * sp*ct + py * cp*ct + pz * st;
    vec3d cz = px * sp*st - py * cp*st + pz * ct;
    position = po + cz * d;

    if (position.length() < Rg + 0.01) {
    	position.normalize(Rg + 0.01);
    }

    view = mat4d(cx.x, cx.y, cx.z, 0,
            cy.x, cy.y, cy.z, 0,
            cz.x, cz.y, cz.z, 0,
            0, 0, 0, 1);
    view = view * mat4d::translate(-position);
}

void updateView1()
{

	double co = cos(lon);
	double so = sin(lon);
	double ca = cos(lat);
	double sa = sin(lat);
	vec3d po = vec3d(co*ca, so*ca, sa) * Rg;
	vec3d px = vec3d(-so, co, 0);
    vec3d py = vec3d(-co*sa, -so*sa, ca);
    vec3d pz = vec3d(co*ca, so*ca, sa);

    double ct = cos(theta);
    double st = sin(theta);
    double cp = cos(phi);
    double sp = sin(phi);
    vec3d cx = px * cp + py * sp;
    vec3d cy = -px * sp*ct + py * cp*ct + pz * st;
    vec3d cz = px * sp*st - py * cp*st + pz * ct;
    position = po + cz * d;

	//position = vec3d(0.5,-0.2,0.1);
	position = vec3d(1.0,0.1,-0.1);
	//position = vec3d(0.1,0.1,0.0);
    if (position.length() < Rg + 0.01) {
    	position.normalize(Rg + 0.01);
    }

	//very nice position for fisheye
	position = vec3d(0.0,200.0+0.01,-Rg+0.01);
	//position = vec3d(.0,-1000.0,Rg/2.0+0.01);
	//position = vec3d(0.0,200.0+0.01,-((Rt+Rg)/2.0));
/*
    if (position.length() < Rg + 0.01) {
    	position.normalize(Rg + 0.01);
    }
*/

    view1 = mat4d(1.0,0.0,0.0, 0,
            0.0,1.0,0.0, 0,
            0.0, 0.0, 1.0, 0,
            0, 0, 0, 1);
/*
	view1 = mat4d(cx.x, cx.y, cx.z, 0,
            cy.x, cy.y, cy.z, 0,
            cz.x, cz.y, cz.z, 0,
            0, 0, 0, 1);
*/
    view1 = view1 * mat4d::translate(-position);

}

void displayTexture()
{
	int lwidth = 764;
	int lheight = 764;
	glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, drawTexture, 0);
    glViewport(0, 0, 764, 764);
    glUseProgram(drawTxtProg);
    

    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float h = position.length() - Rg;
	//float vfov = 2.0 * atan(float(lheight) / float(lwidth) * tan(175.0 / 180 * M_PI / 2.0)) / M_PI * 180;
    //mat4f proj = mat4f::perspectiveProjection(vfov, float(lwidth) / float(lheight), 0.000001* h, 100000 * h);//0.1 * h, 1e5 * h);
	
	float vfov = 2.0 * atan(float(height) / float(width) * tan(80.0 / 180 * M_PI / 2.0)) / M_PI * 180;
    mat4f proj = mat4f::perspectiveProjection(vfov, float(width) / float(height), 0.1 * h, 1e5 * h);

    mat4f iproj = proj.inverse();
    mat4d iview = view1.inverse();
	//mat4d iview = view.inverse();
    vec3d c = iview * vec3d(0.0, 0.0, 0.0);

    mat4f iviewf = mat4f(iview[0][0], iview[0][1], iview[0][2], iview[0][3],
        iview[1][0], iview[1][1], iview[1][2], iview[1][3],
        iview[2][0], iview[2][1], iview[2][2], iview[2][3],
        iview[3][0], iview[3][1], iview[3][2], iview[3][3]);
	
	//cout << "c.x, c.y, c.z: " <<  c.x  << ", " <<  c.y << ", " << c.z << endl;
	//cout << "s.x, s.y, s.z: " <<  s.x  << ", " <<  s.y << ", " << s.z << endl;
	glUniform1i(glGetUniformLocation(drawTxtProg, "reflectanceSampler"), reflectanceUnit);
    glUniform1i(glGetUniformLocation(drawTxtProg, "transmittanceSampler"), transmittanceUnit);
    glUniform1i(glGetUniformLocation(drawTxtProg, "irradianceSampler"), irradianceUnit);
    glUniform1i(glGetUniformLocation(drawTxtProg, "inscatterSampler"), inscatterUnit);
    glUniform3f(glGetUniformLocation(drawTxtProg, "c"), c.x, c.y, c.z);
    glUniform3f(glGetUniformLocation(drawTxtProg, "s"), s.x, s.y, s.z);
    glUniformMatrix4fv(glGetUniformLocation(drawTxtProg, "projInverse"), 1, true, iproj.coefficients());
    glUniformMatrix4fv(glGetUniformLocation(drawTxtProg, "viewInverse"), 1, true, iviewf.coefficients());
    glUniform1f(glGetUniformLocation(drawTxtProg, "exposure"), exposure);
	glUniform1f(glGetUniformLocation(drawTxtProg, "gwidth"), (float)lwidth);
	glUniform1f(glGetUniformLocation(drawTxtProg, "gheight"), (float)lheight);
    drawQuad();
}


void redisplayFunc()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float h = position.length() - Rg;
	//float vfov = 2.0 * atan(float(height) / float(width) * tan(175.0 / 180 * M_PI / 2.0)) / M_PI * 180;
    //mat4f proj = mat4f::perspectiveProjection(vfov, float(width) / float(height), 0.000001* h, 100000 * h);//0.1 * h, 1e5 * h);
	
	float vfov = 2.0 * atan(float(height) / float(width) * tan(80.0 / 180 * M_PI / 2.0)) / M_PI * 180;
    mat4f proj = mat4f::perspectiveProjection(vfov, float(width) / float(height), 0.1 * h, 1e5 * h);
	//mat4f proj = mat4f::orthographicProjection(width, height, 0.0001 * h, 1e5 * h);

    mat4f iproj = proj.inverse();
#ifdef _FISHEYE_
    mat4d iview = view1.inverse();
#else
	mat4d iview = view.inverse();
#endif
	vec3d c = iview * vec3d(0.0, 0.0, 0.0);

    mat4f iviewf = mat4f(iview[0][0], iview[0][1], iview[0][2], iview[0][3],
        iview[1][0], iview[1][1], iview[1][2], iview[1][3],
        iview[2][0], iview[2][1], iview[2][2], iview[2][3],
        iview[3][0], iview[3][1], iview[3][2], iview[3][3]);
	
	//cout << "c.x, c.y, c.z: " <<  c.x  << ", " <<  c.y << ", " << c.z << endl;
	//cout << "s.x, s.y, s.z: " <<  s.x  << ", " <<  s.y << ", " << s.z << endl;

    glUniform3f(glGetUniformLocation(drawProg, "c"), c.x, c.y, c.z);
    glUniform3f(glGetUniformLocation(drawProg, "s"), s.x, s.y, s.z);
    glUniformMatrix4fv(glGetUniformLocation(drawProg, "projInverse"), 1, true, iproj.coefficients());
    glUniformMatrix4fv(glGetUniformLocation(drawProg, "viewInverse"), 1, true, iviewf.coefficients());
    glUniform1f(glGetUniformLocation(drawProg, "exposure"), exposure);
	glUniform1f(glGetUniformLocation(drawProg, "gwidth"), (float)width);
	glUniform1f(glGetUniformLocation(drawProg, "gheight"), (float)height);
    drawQuad();

    glutSwapBuffers();
}

// ----------------------------------------------------------------------------
// USER INTERFACE
// ----------------------------------------------------------------------------

void reshapeFunc(int x, int y)
{
    width = x;
    height = y;
    glViewport(0, 0, x, y);
    glutPostRedisplay();
}

void mouseClickFunc(int button, int state, int x, int y)
{
    oldx = x;
    oldy = y;
    int modifiers = glutGetModifiers();
    bool ctrl = (modifiers & GLUT_ACTIVE_CTRL) != 0;
    bool shift = (modifiers & GLUT_ACTIVE_SHIFT) != 0;
    if (ctrl) {
    	move = 0;
    } else if (shift) {
        move = 1;
    } else {
    	move = 2;
    }
}

void mouseMotionFunc(int x, int y)
{
    if (move == 0) {
    	phi += (oldx - x) / 500.0;
    	theta += (oldy - y) / 500.0;
        theta = max(0.0, min(M_PI, theta));
        updateView();
        oldx = x;
        oldy = y;
    } else if (move == 1) {
    	double factor = position.length() - Rg;
    	factor = factor / Rg;
    	lon += (oldx - x) / 400.0 * factor;
    	lat -= (oldy - y) / 400.0 * factor;
        lat = max(-M_PI / 2.0, min(M_PI / 2.0, lat));
        updateView();
        oldx = x;
        oldy = y;
    } else if (move == 2) {
    	float vangle = asin(s.z);
    	float hangle = atan2(s.y, s.x);
    	vangle += (oldy - y) / 180.0 * M_PI / 4;
    	hangle += (oldx - x) / 180.0 * M_PI / 4;
    	s.x = cos(vangle) * cos(hangle);
    	s.y = cos(vangle) * sin(hangle);
    	s.z = sin(vangle);
        oldx = x;
        oldy = y;
    }
}

void specialKeyFunc(int c, int x, int y)
{
    switch (c) {
    case GLUT_KEY_PAGE_UP:
    	d = d * 1.05;
        updateView();
        break;
    case GLUT_KEY_PAGE_DOWN:
    	d = d / 1.05;
        updateView();
        break;
    case GLUT_KEY_F5:
	    recompute();
        glViewport(0, 0, width, height);
	case GLUT_KEY_F6:
		recompute(true);
        glViewport(0, 0, width, height);
        break;
    }
}

void keyboardFunc(unsigned char c, int x, int y)
{
    if (c == 27) {
        ::exit(0);
    } else if (c == '+') {
		exposure *= 1.1;
	} else if (c == '-') {
		exposure /= 1.1;
	}
}

void idleFunc()
{
    glutPostRedisplay();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
#ifdef _FISHEYE_
	glutInitWindowSize(768, 768);
#else
    glutInitWindowSize(1024, 768);
#endif
    glutCreateWindow("Atmospheric Scattering experments");
    glutCreateMenu(NULL);
    glutDisplayFunc(redisplayFunc);
    glutReshapeFunc(reshapeFunc);
    glutMouseFunc(mouseClickFunc);
    glutMotionFunc(mouseMotionFunc);
    glutSpecialFunc(specialKeyFunc);
    glutKeyboardFunc(keyboardFunc);
    glutIdleFunc(idleFunc);
    //glewInit();

    loadData();
    updateView1();
	//updateView();
	precompute();
    updateView();
    glutMainLoop();
}

#elif defined(LATLON_PROJ) //_ATMOSPHERE

///////////////////////////////////////////////////////////////////////////////
// main.cpp
// ========
// testing vertex array (glDrawElements, glDrawArrays)
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2005-10-04
// UPDATED: 2012-07-11
///////////////////////////////////////////////////////////////////////////////

#ifdef __APPLE__
#include <GLUT/glut.h>
#else

//#include <GL/glew.h>
//#include <glee.h>
//#ifdef __gl_h_
//#undef __gl_h_
//#endif

#include "GLTools.h"
//#include <GL/glut.h>
//#include <GL/glext.h>
//#include "tiffio.h"
//#include <GL/glut.h>
#endif

#include <vector>
#include <string>
#include <fstream>
#include <cmath>

#include "vec3.h"
#include "mat4.h"
#include "Main.h"
#include "utils.h"


//#include "glext.h"
#include <iostream>
#include <sstream>
#include <iomanip>
using std::stringstream;
using std::cout;
using std::endl;
using std::ends;


// GLUT CALLBACK functions
void displayCB();
void reshapeCB(int w, int h);
void timerCB(int millisec);
void keyboardCB(unsigned char key, int x, int y);
void mouseCB(int button, int stat, int x, int y);
void mouseMotionCB(int x, int y);

void initGL();
int  initGLUT(int argc, char **argv);
bool initSharedMem();
void clearSharedMem();
void initLights();
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
void drawString(const char *str, int x, int y, float color[4], void *font);
void drawString3D(const char *str, float pos[3], float color[4], void *font);
void toOrtho();
void toPerspective();
void draw1();
void draw2();
void draw3();
void draw4();
void draw5();


// constants
const int   SCREEN_WIDTH    = 400;
const int   SCREEN_HEIGHT   = 300;
const float CAMERA_DISTANCE = 10.0f;
const int   TEXT_WIDTH      = 8;
const int   TEXT_HEIGHT     = 13;


// global variables
void *font = GLUT_BITMAP_8_BY_13;
int screenWidth;
int screenHeight;
bool mouseLeftDown;
bool mouseRightDown;
bool mouseMiddleDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance;
int drawMode;
int maxVertices;
int maxIndices;

//#ifdef _WIN32
// function pointer to OpenGL extensions
// glDrawRangeElements() defined in OpenGL v1.2 or greater
//PFNGLDRAWRANGEELEMENTSPROC pglDrawRangeElements = 0;
//#define glDrawRangeElements pglDrawRangeElements
//#endif



// cube ///////////////////////////////////////////////////////////////////////
//    v6----- v5
//   /|      /|
//  v1------v0|
//  | |     | |
//  | |v7---|-|v4
//  |/      |/
//  v2------v3

// vertex coords array for glDrawArrays() =====================================
// A cube has 6 sides and each side has 2 triangles, therefore, a cube consists
// of 36 vertices (6 sides * 2 tris * 3 vertices = 36 vertices). And, each
// vertex is 3 components (x,y,z) of floats, therefore, the size of vertex
// array is 108 floats (36 * 3 = 108).
GLfloat vertices1[] = { 1, 1, 1,  -1, 1, 1,  -1,-1, 1,      // v0-v1-v2 (front)
                       -1,-1, 1,   1,-1, 1,   1, 1, 1,      // v2-v3-v0

                        1, 1, 1,   1,-1, 1,   1,-1,-1,      // v0-v3-v4 (right)
                        1,-1,-1,   1, 1,-1,   1, 1, 1,      // v4-v5-v0

                        1, 1, 1,   1, 1,-1,  -1, 1,-1,      // v0-v5-v6 (top)
                       -1, 1,-1,  -1, 1, 1,   1, 1, 1,      // v6-v1-v0

                       -1, 1, 1,  -1, 1,-1,  -1,-1,-1,      // v1-v6-v7 (left)
                       -1,-1,-1,  -1,-1, 1,  -1, 1, 1,      // v7-v2-v1

                       -1,-1,-1,   1,-1,-1,   1,-1, 1,      // v7-v4-v3 (bottom)
                        1,-1, 1,  -1,-1, 1,  -1,-1,-1,      // v3-v2-v7

                        1,-1,-1,  -1,-1,-1,  -1, 1,-1,      // v4-v7-v6 (back)
                       -1, 1,-1,   1, 1,-1,   1,-1,-1 };    // v6-v5-v4

// normal array
GLfloat normals1[]  = { 0, 0, 1,   0, 0, 1,   0, 0, 1,      // v0-v1-v2 (front)
                        0, 0, 1,   0, 0, 1,   0, 0, 1,      // v2-v3-v0

                        1, 0, 0,   1, 0, 0,   1, 0, 0,      // v0-v3-v4 (right)
                        1, 0, 0,   1, 0, 0,   1, 0, 0,      // v4-v5-v0

                        0, 1, 0,   0, 1, 0,   0, 1, 0,      // v0-v5-v6 (top)
                        0, 1, 0,   0, 1, 0,   0, 1, 0,      // v6-v1-v0

                       -1, 0, 0,  -1, 0, 0,  -1, 0, 0,      // v1-v6-v7 (left)
                       -1, 0, 0,  -1, 0, 0,  -1, 0, 0,      // v7-v2-v1

                        0,-1, 0,   0,-1, 0,   0,-1, 0,      // v7-v4-v3 (bottom)
                        0,-1, 0,   0,-1, 0,   0,-1, 0,      // v3-v2-v7

                        0, 0,-1,   0, 0,-1,   0, 0,-1,      // v4-v7-v6 (back)
                        0, 0,-1,   0, 0,-1,   0, 0,-1 };    // v6-v5-v4

// color array
GLfloat colors1[]   = { 1, 1, 1,   1, 1, 0,   1, 0, 0,      // v0-v1-v2 (front)
                        1, 0, 0,   1, 0, 1,   1, 1, 1,      // v2-v3-v0

                        1, 1, 1,   1, 0, 1,   0, 0, 1,      // v0-v3-v4 (right)
                        0, 0, 1,   0, 1, 1,   1, 1, 1,      // v4-v5-v0

                        1, 1, 1,   0, 1, 1,   0, 1, 0,      // v0-v5-v6 (top)
                        0, 1, 0,   1, 1, 0,   1, 1, 1,      // v6-v1-v0

                        1, 1, 0,   0, 1, 0,   0, 0, 0,      // v1-v6-v7 (left)
                        0, 0, 0,   1, 0, 0,   1, 1, 0,      // v7-v2-v1

                        0, 0, 0,   0, 0, 1,   1, 0, 1,      // v7-v4-v3 (bottom)
                        1, 0, 1,   1, 0, 0,   0, 0, 0,      // v3-v2-v7

                        0, 0, 1,   0, 0, 0,   0, 1, 0,      // v4-v7-v6 (back)
                        0, 1, 0,   0, 1, 1,   0, 0, 1 };    // v6-v5-v4



// vertex array for glDrawElements() and glDrawRangeElement() =================
// Notice that the sizes of these arrays become samller than the arrays for
// glDrawArrays() because glDrawElements() uses an additional index array to
// choose designated vertices with the indices. The size of vertex array is now
// 24 instead of 36, but the index array size is 36, same as the number of
// vertices required to draw a cube.
GLfloat vertices2[] = { 10, 10, 0,  10, 100, 0,  100,100, 0,  45,300, 0,   // v0,v1,v2,v3 (front)
                        30, 90, 0,   150, 290, 0,   260,10, 0,   40, 2, 0,   // v0,v3,v4,v5 (right)
                        200, 54, 0,   1, 32, 0,  230, 32, 0,  278, 89, 0,   // v0,v5,v6,v1 (top)
                       13, 82, 0,  199, 154, 0,  23,123, 0,  167,300, 0,   // v1,v6,v7,v2 (left)
                       14,15, 0,   21,20, 0,   120,231, 0,  145,89, 0,   // v7,v4,v3,v2 (bottom)
                        111,1, 0,  220,21, 0,  134, 201, 0,   121, 112, 0 }; // v4,v7,v6,v5 (back)

// normal array
GLfloat normals2[]  = { 0, 0, 1,   0, 0, 1,   0, 0, 1,   0, 0, 1,   // v0,v1,v2,v3 (front)
                        1, 0, 0,   1, 0, 0,   1, 0, 0,   1, 0, 0,   // v0,v3,v4,v5 (right)
                        0, 1, 0,   0, 1, 0,   0, 1, 0,   0, 1, 0,   // v0,v5,v6,v1 (top)
                       -1, 0, 0,  -1, 0, 0,  -1, 0, 0,  -1, 0, 0,   // v1,v6,v7,v2 (left)
                        0,-1, 0,   0,-1, 0,   0,-1, 0,   0,-1, 0,   // v7,v4,v3,v2 (bottom)
                        0, 0,-1,   0, 0,-1,   0, 0,-1,   0, 0,-1 }; // v4,v7,v6,v5 (back)

// color array
GLfloat colors2[]   = { 1, 1, 1,   1, 1, 0,   1, 0, 0,   1, 0, 1,   // v0,v1,v2,v3 (front)
                        1, 1, 1,   1, 0, 1,   0, 0, 1,   0, 1, 1,   // v0,v3,v4,v5 (right)
                        1, 1, 1,   0, 1, 1,   0, 1, 0,   1, 1, 0,   // v0,v5,v6,v1 (top)
                        1, 1, 0,   0, 1, 0,   0, 0, 0,   1, 0, 0,   // v1,v6,v7,v2 (left)
                        0, 0, 0,   0, 0, 1,   1, 0, 1,   1, 0, 0,   // v7,v4,v3,v2 (bottom)
                        0, 0, 1,   0, 0, 0,   0, 1, 0,   0, 1, 1 }; // v4,v7,v6,v5 (back)

// index array of vertex array for glDrawElements() & glDrawRangeElement()
GLubyte indices[]  = { 0, 1, 2, 3, 0};
GLubyte indices_old[]  = { 0, 1, 2,   2, 3, 0,      // front
                       4, 5, 6,   6, 7, 4,      // right
                       8, 9,10,  10,11, 8,      // top
                      12,13,14,  14,15,12,      // left
                      16,17,18,  18,19,16,      // bottom
                      20,21,22,  22,23,20 };    // back



// interleaved vertex array for glDrawElements() & glDrawRangeElements() ======
// All vertex attributes (position, normal, color) are packed together as a
// struct or set, for example, ((V,N,C), (V,N,C), (V,N,C),...).
// It is called an array of struct, and provides better memory locality.
GLfloat vertices3[] = { 1, 1, 1,   0, 0, 1,   1, 1, 1,              // v0 (front)
                       -1, 1, 1,   0, 0, 1,   1, 1, 0,              // v1
                       -1,-1, 1,   0, 0, 1,   1, 0, 0,              // v2
                        1,-1, 1,   0, 0, 1,   1, 0, 1,              // v3

                        1, 1, 1,   1, 0, 0,   1, 1, 1,              // v0 (right)
                        1,-1, 1,   1, 0, 0,   1, 0, 1,              // v3
                        1,-1,-1,   1, 0, 0,   0, 0, 1,              // v4
                        1, 1,-1,   1, 0, 0,   0, 1, 1,              // v5

                        1, 1, 1,   0, 1, 0,   1, 1, 1,              // v0 (top)
                        1, 1,-1,   0, 1, 0,   0, 1, 1,              // v5
                       -1, 1,-1,   0, 1, 0,   0, 1, 0,              // v6
                       -1, 1, 1,   0, 1, 0,   1, 1, 0,              // v1

                       -1, 1, 1,  -1, 0, 0,   1, 1, 0,              // v1 (left)
                       -1, 1,-1,  -1, 0, 0,   0, 1, 0,              // v6
                       -1,-1,-1,  -1, 0, 0,   0, 0, 0,              // v7
                       -1,-1, 1,  -1, 0, 0,   1, 0, 0,              // v2

                       -1,-1,-1,   0,-1, 0,   0, 0, 0,              // v7 (bottom)
                        1,-1,-1,   0,-1, 0,   0, 0, 1,              // v4
                        1,-1, 1,   0,-1, 0,   1, 0, 1,              // v3
                       -1,-1, 1,   0,-1, 0,   1, 0, 0,              // v2

                        1,-1,-1,   0, 0,-1,   0, 0, 1,              // v4 (back)
                       -1,-1,-1,   0, 0,-1,   0, 0, 0,              // v7
                       -1, 1,-1,   0, 0,-1,   0, 1, 0,              // v6
                        1, 1,-1,   0, 0,-1,   0, 1, 1 };            // v5

///////////////////////////////////////////////////////////////////////////////
// Convert latitude and longitude into X,Y
//
///////////////////////////////////////////////////////////////////////////////
void convert(double lat, double lon, /*out*/ double& x, /*out*/ double& y, /*out*/ double& z)
{
/*
	double alt = 0.0; //current altitude
	double Re = 6378137;
	double Rp = 6356752.31424518;

	double latrad = lat/180.0*M_PI;
	double lonrad = lon/180.0*M_PI;

	double coslat = cos(latrad);
	double sinlat = sin(latrad);
	double coslon = cos(lonrad);
	double sinlon = sin(lonrad);

	double term1 = (Re*Re*coslat)/
	  sqrt(Re*Re*coslat*coslat + Rp*Rp*sinlat*sinlat);

	double term2 = alt*coslat + term1;

	x=coslon*term2;
	y=sinlon*term2;
	z = alt*sinlat + (Rp*Rp*sinlat)/ 
		sqrt(Re*Re*coslat*coslat + Rp*Rp*sinlat*sinlat);
*/
double radius_of_world = 6378137;
double longitude = lon;
double latitude = lat;
//x = radius_of_world * cos(longitude) * sin(90 - latitude);
//y = radius_of_world * sin(longitude) * sin(90 - latitude);
//z = radius_of_world * cos(90 - latitude);

//Applying trig identities and actionscript we get:

x = radius_of_world * cos(longitude) * cos(latitude);
y = radius_of_world * sin(longitude) * cos(latitude);
z = radius_of_world * sin(latitude);
cout << "lat,lon: " << lat <<", " << lon <<", x,y: " << x << ", " << y << endl;
//Now that you have a point (x, y, z) you can project this on to the xy plane. This is a little tricker of a concept to grasp so I'll just show the code instead:
//double focal_length = 100000000;
//x = x * focal_length / (focal_length + z);
//y = y * focal_length / (focal_length + z);

}

///////////////////////////////////////////////////////////////////////////////
// Convert from X or Y to screen coord
// 
///////////////////////////////////////////////////////////////////////////////
double convReal2Screen(double coord, double min, double max, double screen_dim )
{
	return (coord-min)/(max-min) * screen_dim;
}


///////////////////////////////////////////////////////////////////////////////
// Initialize a geo data set Just sample list of world cities
// 
///////////////////////////////////////////////////////////////////////////////
#define MAX_GEON	1000*3
GLfloat verticesGeo[MAX_GEON];

// normal array
GLfloat normalsGeo[MAX_GEON];

// color array
GLfloat colorsGeo[MAX_GEON];

// index array of vertex array for glDrawElements() & glDrawRangeElement()
GLubyte indicesGeo[MAX_GEON]; 
void initGeoData()
{
	int count = 0;
	double x,y,z;
	double lon, lat;
	
	double max_x,max_y;
	double min_x,min_y;
	double Re = 6378137;
	double Rp = 6356752.31424518;
	min_x = -Re;
	max_x = Re;
	min_y = -Re;
	max_y = Re;

	//Vladivostok, Russia	43.10	132.0
	lat = 43.10; lon = 132.0;
	convert(lat, lon, x, y, z);
	verticesGeo[count] = convReal2Screen(x, min_x, max_x, screenWidth );
	colorsGeo[count] = 1.0;
	verticesGeo[++count] = convReal2Screen(y, min_y, max_y, screenHeight );
	colorsGeo[count] = 0.0;
	verticesGeo[++count] = 0.0;
	colorsGeo[count] = 1.0;

	//Oslo, Norway	59.57	10.42	
	lat = 59.57; lon = 10.42;
	convert(lat, lon, x, y, z);
	verticesGeo[++count] = convReal2Screen(x, min_x, max_x, screenWidth );
	colorsGeo[count] = 0.0;
	verticesGeo[++count] = convReal2Screen(y, min_y, max_y, screenHeight );
	colorsGeo[count] = 1.0;
	verticesGeo[++count] = 0.0;
	colorsGeo[count] = 0.0;

	//Johannesburg, South Africa	-26.12	28.4
	lat = -26.12; lon = 28.4;
	convert(lat, lon, x, y, z);
	verticesGeo[++count] = convReal2Screen(x, min_x, max_x, screenWidth );
	colorsGeo[count] = 0.0;
	verticesGeo[++count] = convReal2Screen(y, min_y, max_y, screenHeight );
	colorsGeo[count] = 1.0;
	verticesGeo[++count] = 0.0;
	colorsGeo[count] = 0.0;

	//Mexico City, Mexico	19.26	-99.7
	lat = 19.26; lon = -99.7;
	convert(lat, lon, x, y, z);
	verticesGeo[++count] = convReal2Screen(x, min_x, max_x, screenWidth );
	colorsGeo[count] = 1.0;
	verticesGeo[++count] = convReal2Screen(y, min_y, max_y, screenHeight );
	colorsGeo[count] = 1.0;
	verticesGeo[++count] = 0.0;
	colorsGeo[count] = 1.0;

	//Moscow, Russia	55.45	37.36	8:00 p.m.
	lat = 55.45; lon = 37.36;
	convert(lat, lon, x, y, z);
	verticesGeo[++count] = convReal2Screen(x, min_x, max_x, screenWidth );
	colorsGeo[count] = 1.0;
	verticesGeo[++count] = convReal2Screen(y, min_y, max_y, screenHeight );
	colorsGeo[count] = 0.0;
	verticesGeo[++count] = 0.0;
	colorsGeo[count] = 1.0;

	//London, England	51.32	-0.5	5:00 p.m.
	lat = 51.32; lon = -0.5;
	convert(lat, lon, x, y, z);
	verticesGeo[++count] = convReal2Screen(x, min_x, max_x, screenWidth );
	colorsGeo[count] = 0.0;
	verticesGeo[++count] = convReal2Screen(y, min_y, max_y, screenHeight );
	colorsGeo[count] = 0.0;
	verticesGeo[++count] = 0.0;
	colorsGeo[count] = 1.0;

	//Paris, France	48.48	2.20	6:00 p.m.
	lat = 48.48; lon = 2.20;
	convert(lat, lon, x, y, z);
	verticesGeo[++count] = convReal2Screen(x, min_x, max_x, screenWidth );
	colorsGeo[count] = 1.0;
	verticesGeo[++count] = convReal2Screen(y, min_y, max_y, screenHeight );
	colorsGeo[count] = 0.0;
	verticesGeo[++count] = 0.0;
	colorsGeo[count] = 0.0;

	count=0;
	indicesGeo[count] = 5;
	indicesGeo[++count] = 6;
	indicesGeo[++count] = 3;
	indicesGeo[++count] = 2;
	indicesGeo[++count] = 0;
	//indicesGeo[++count] = 5;
	//indicesGeo[++count] = 6;
}

///////////////////////////////////////////////////////////////////////////////
// draw 1: immediate mode
// 78 calls = 36 glVertex*() calls + 36 glColor*() calls + 6 glNormal*() calls
///////////////////////////////////////////////////////////////////////////////
void draw0()
{
    glPushMatrix();
    glColor3f(0,1.0,0);
	glBegin(GL_LINES);
		glVertex2f(100,2);
		glVertex2f(5,20);
    glEnd();
    glPopMatrix();
}


void draw1()
{
    glPushMatrix();
    glTranslatef(-2, 2, 0); // move to upper left corner
    glBegin(GL_LINE_LOOP);
        // front faces
        glNormal3f(0,0,0);
        // face v0-v1-v2
        glColor3f(1,1,0);
        glVertex3f(1,1,0);
        glColor3f(1,1,0);
        glVertex3f(-1,1,0);
        glColor3f(1,0,0);
        glVertex3f(-1,-1,0);
        // face v2-v3-v0
        glColor3f(1,0,0);
        glVertex3f(-1,-1,0);
        glColor3f(1,0,0);
        glVertex3f(1,-1,0);
        glColor3f(1,1,0);
        glVertex3f(1,1,0);

        // right faces
        glNormal3f(1,0,0);
        // face v0-v3-v4
        glColor3f(1,1,0);
        glVertex3f(1,1,0);
        glColor3f(1,0,0);
        glVertex3f(1,-1,0);
        glColor3f(0,0,0);
        glVertex3f(1,-1,0);
        // face v4-v5-v0
        glColor3f(0,0,0);
        glVertex3f(1,-1,0);
        glColor3f(0,1,0);
        glVertex3f(1,1,0);
        glColor3f(1,1,0);
        glVertex3f(1,1,0);

        // top faces
        glNormal3f(0,1,0);
        // face v0-v5-v6
        glColor3f(1,1,0);
        glVertex3f(1,1,0);
        glColor3f(0,1,0);
        glVertex3f(1,1,0);
        glColor3f(0,1,0);
        glVertex3f(-1,1,0);
        // face v6-v1-v0
        glColor3f(0,1,0);
        glVertex3f(-1,1,0);
        glColor3f(1,1,0);
        glVertex3f(-1,1,0);
        glColor3f(1,1,0);
        glVertex3f(1,1,0);
/*
        // left faces
        glNormal3f(-1,0,0);
        // face  v1-v6-v7
        glColor3f(1,1,0);
        glVertex3f(-1,1,0);
        glColor3f(0,1,0);
        glVertex3f(-1,1,0);
        glColor3f(0,0,0);
        glVertex3f(-1,-1,0);
        // face v7-v2-v1
        glColor3f(0,0,0);
        glVertex3f(-1,-1,-1);
        glColor3f(1,0,0);
        glVertex3f(-1,-1,1);
        glColor3f(1,1,0);
        glVertex3f(-1,1,1);

        // bottom faces
        glNormal3f(0,-1,0);
        // face v7-v4-v3
        glColor3f(0,0,0);
        glVertex3f(-1,-1,-1);
        glColor3f(0,0,1);
        glVertex3f(1,-1,-1);
        glColor3f(1,0,1);
        glVertex3f(1,-1,1);
        // face v3-v2-v7
        glColor3f(1,0,1);
        glVertex3f(1,-1,1);
        glColor3f(1,0,0);
        glVertex3f(-1,-1,1);
        glColor3f(0,0,0);
        glVertex3f(-1,-1,-1);

        // back faces
        glNormal3f(0,0,-1);
        // face v4-v7-v6
        glColor3f(0,0,1);
        glVertex3f(1,-1,-1);
        glColor3f(0,0,0);
        glVertex3f(-1,-1,-1);
        glColor3f(0,1,0);
        glVertex3f(-1,1,-1);
        // face v6-v5-v4
        glColor3f(0,1,0);
        glVertex3f(-1,1,-1);
        glColor3f(0,1,1);
        glVertex3f(1,1,-1);
        glColor3f(0,0,1);
        glVertex3f(1,-1,-1);
*/
    glEnd();

    glPopMatrix();
}

///////////////////////////////////////////////////////////////////////////////
// draw cube at upper-right corner with glDrawArrays
// A cube has only 8 vertices, but each vertex is shared for 3 different faces,
// which have different normals. Therefore, we need more than 8 vertex data to
// draw a cube. Since each face has 2 triangles, we need 6 vertices per face.
// (2 * 3 = 6) And, a cube has 6 faces, so, the total number of vertices for
// drawing a cube is 36 (= 6 faces * 6 vertices).
// Note that there are some duplicated vertex data for glDrawArray() because
// the vertex data in the vertex array must be sequentially placed in memory.
// For a cube, there are 24 unique vertex data and 12 redundant vertex data in
// the vertex array.
///////////////////////////////////////////////////////////////////////////////
void draw2()
{
    // enble and specify pointers to vertex arrays
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, 0, normals1);
    glColorPointer(3, GL_FLOAT, 0, colors1);
    glVertexPointer(3, GL_FLOAT, 0, vertices1);

    glPushMatrix();
    glTranslatef(2, 2, 0);                  // move to upper-right corner

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

///////////////////////////////////////////////////////////////////////////////
// draw cube at bottom-left corner with glDrawElements
// The main advantage of glDrawElements() over glDrawArray() is that
// glDrawElements() allows hopping around the vertex array with the associated
// index values.
// In a cube, the number of vertex data in the vertex array can be reduced to
// 24 vertices for glDrawElements().
// Note that you need an additional array (index array) to store how to traverse
// the vertext data. For a cube, we need 36 entries in the index array.
///////////////////////////////////////////////////////////////////////////////
void draw3()
{
    // enable and specify pointers to vertex arrays
    //glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    //glNormalPointer(GL_FLOAT, 0, normals2);
    glColorPointer(3, GL_FLOAT, 0, colorsGeo);
    glVertexPointer(3, GL_FLOAT, 0, verticesGeo);

    glPushMatrix();
    //glTranslatef(-2, -2, 0);                // move to bottom-left corner

    //glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, indices);
	//glDrawElements(GL_LINE_LOOP, 36, GL_UNSIGNED_BYTE, indices);
	glDrawElements(GL_LINE_LOOP, 5, GL_UNSIGNED_BYTE, indicesGeo);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    glDisableClientState(GL_COLOR_ARRAY);
    //glDisableClientState(GL_NORMAL_ARRAY);
}

///////////////////////////////////////////////////////////////////////////////
// draw cube at bottom-right corner with glDrawRangeElements()
// glDrawRangeElements() has two more parameters than glDrawElements() needs;
// start and end index. These values specifies a range of vertex data to be
// loaded into OpenGL. "start" param specifies where the range starts from, and
// "end" param specifies where the range ends. All the index values to be drawn
// must be between "start" and "end".
// Note that not all vertices in the range [start, end] will be referenced.
// But, if you specify a sparsely used range, it causes unnecessary process for
// many unused vertices in that range.
///////////////////////////////////////////////////////////////////////////////
void draw4()
{
    // enable and specify pointers to vertex arrays
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, 0, normals2);
    glColorPointer(3, GL_FLOAT, 0, colors2);
    glVertexPointer(3, GL_FLOAT, 0, vertices2);

    glPushMatrix();
    glTranslatef(2, -2, 0);                 // move to bottom-right

    // draw first half (18 elements){0,1,2, 2,3,0, 4,5,6, 6,7,4, 8,9,10, 10,11,8}
    // The minimum index value in this range is 0, and the maximum index is 11,
    // therefore, "start" param is 0 and "end" param is 11.
    // Then, OpenGL will prefetch only 12 vertex data from the array prior to
    // rendering. (half of total data)
    glDrawRangeElements(GL_TRIANGLES, 0, 11, 18, GL_UNSIGNED_BYTE, indices);

    // draw last half (18 elements) {12,13,14, 14,15,12, 16,17,18, 18,19,16, 20,21,22, 22,23,20}
    // The minimum index value in this range is 12, and the maximum index is 23,
    // therefore, "start" param is 12 and "end" param is 23.
    // Then, OpenGL will prefetch only 12 vertex data from the array prior to
    // rendering.
    // Note that the last param of glDrawRangeElements(). It is the pointer to
    // the location of the first index value to be drawn.
    glDrawRangeElements(GL_TRIANGLES, 12, 23, 18, GL_UNSIGNED_BYTE, indices+18);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

///////////////////////////////////////////////////////////////////////////////
// draw cube at bottom-left corner with glDrawElements and interleave array
// All the vertex data (position, normal, colour) can be placed together into a 
// single array, and be interleaved like (VNCVNC...). The interleave vertex data
// provides better memory locality.
// Since we are using a single interleaved vertex array to store vertex
// positions, normals and colours, we need to specify "stride" and "pointer"
// parameters properly for glVertexPointer, glNormalPointer and glColorPointer.
// Each vertex has 9 elements of floats (3 position + 3 normal + 3 color), so,
// the stride param should be 36 (= 9 * 4 bytes).
///////////////////////////////////////////////////////////////////////////////
void draw5()
{
    // enable and specify pointers to vertex arrays
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, 9 * sizeof(GLfloat), vertices3 + 3);
    glColorPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), vertices3 + 6);
    glVertexPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), vertices3);

    glPushMatrix();
    glTranslatef(-2, -2, 0);                // move to bottom-left

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, indices);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}






///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    // init global vars
    initSharedMem();

	initGeoData();
    // init GLUT and GL
    initGLUT(argc, argv);
    initGL();


    // check max of elements vertices and elements indices that your video card supports
    // Use these values to determine the range of glDrawRangeElements()
    // The constants are defined in glext.h
    glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &maxVertices);
    glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &maxIndices);

#ifdef _WIN32
    // get function pointer to glDrawRangeElements
    glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)wglGetProcAddress("glDrawRangeElements");
#endif
    
    // the last GLUT call (LOOP)
    // window will be shown and display callback is triggered by events
    // NOTE: this call never return main().
    glutMainLoop(); /* Start GLUT event-processing loop */

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// initialize GLUT for windowing
///////////////////////////////////////////////////////////////////////////////
int initGLUT(int argc, char **argv)
{
    // GLUT stuff for windowing
    // initialization openGL window.
    // it is called before any other GLUT routine
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);   // display mode

    glutInitWindowSize(screenWidth, screenHeight);  // window size

    glutInitWindowPosition(100, 100);               // window location

    // finally, create a window with openGL context
    // Window will not displayed until glutMainLoop() is called
    // it returns a unique ID
    int handle = glutCreateWindow(argv[0]);     // param is the title of window

    // register GLUT callback functions
    glutDisplayFunc(displayCB);
    glutTimerFunc(33, timerCB, 33);             // redraw only every given millisec
    glutReshapeFunc(reshapeCB);
    glutKeyboardFunc(keyboardCB);
    glutMouseFunc(mouseCB);
    glutMotionFunc(mouseMotionCB);

    return handle;
}



///////////////////////////////////////////////////////////////////////////////
// initialize OpenGL
// disable unused features
///////////////////////////////////////////////////////////////////////////////
void initGL()
{
    glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

    // enable /disable features
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    //glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    //glEnable(GL_TEXTURE_2D);
    //glEnable(GL_CULL_FACE);

     // track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glClearColor(0, 0, 0, 0);                   // background color
    glClearStencil(0);                          // clear stencil buffer
    glClearDepth(1.0f);                         // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

    initLights();
}



///////////////////////////////////////////////////////////////////////////////
// write 2d text using GLUT
// The projection matrix must be set to orthogonal before call this function.
///////////////////////////////////////////////////////////////////////////////
void drawString(const char *str, int x, int y, float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
    glDisable(GL_TEXTURE_2D);

    glColor4fv(color);          // set text color
    glRasterPos2i(x, y);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}



///////////////////////////////////////////////////////////////////////////////
// draw a string in 3D space
///////////////////////////////////////////////////////////////////////////////
void drawString3D(const char *str, float pos[3], float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
    glDisable(GL_TEXTURE_2D);

    glColor4fv(color);          // set text color
    glRasterPos3fv(pos);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}



///////////////////////////////////////////////////////////////////////////////
// initialize global variables
///////////////////////////////////////////////////////////////////////////////
bool initSharedMem()
{
    screenWidth = SCREEN_WIDTH;
    screenHeight = SCREEN_HEIGHT;

    mouseLeftDown = mouseRightDown = mouseMiddleDown = false;
    mouseX = mouseY = 0;

    cameraAngleX = cameraAngleY = 0.0f;
    cameraDistance = CAMERA_DISTANCE;

    drawMode = 0; // 0:fill, 1: wireframe, 2:points
    maxVertices = maxIndices = 0;

    return true;
}



///////////////////////////////////////////////////////////////////////////////
// clean up global vars
///////////////////////////////////////////////////////////////////////////////
void clearSharedMem()
{
}



///////////////////////////////////////////////////////////////////////////////
// initialize lights
///////////////////////////////////////////////////////////////////////////////
void initLights()
{
    // set up light colors (ambient, diffuse, specular)
    GLfloat lightKa[] = {.2f, .2f, .2f, 1.0f};  // ambient light
    GLfloat lightKd[] = {.7f, .7f, .7f, 1.0f};  // diffuse light
    GLfloat lightKs[] = {1, 1, 1, 1};           // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    // position the light
    float lightPos[4] = {0, 0, 20, 1}; // positional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                        // MUST enable each light source after configuration
}



///////////////////////////////////////////////////////////////////////////////
// set camera position and lookat direction
///////////////////////////////////////////////////////////////////////////////
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(posX, posY, posZ, targetX, targetY, targetZ, 0, 1, 0); // eye(x,y,z), focal(x,y,z), up(x,y,z)
}



///////////////////////////////////////////////////////////////////////////////
// display info messages
///////////////////////////////////////////////////////////////////////////////
void showInfo()
{
    // backup current model-view matrix
    glPushMatrix();                     // save current modelview matrix
    glLoadIdentity();                   // reset modelview matrix

    // set to 2D orthogonal projection
    glMatrixMode(GL_PROJECTION);        // switch to projection matrix
    glPushMatrix();                     // save current projection matrix
    glLoadIdentity();                   // reset projection matrix
    gluOrtho2D(0, screenWidth, 0, screenHeight); // set to orthogonal projection

    float color[4] = {1, 1, 1, 1};

    stringstream ss;
    ss << std::fixed << std::setprecision(3);

    ss << "Max Elements Vertices: " << maxVertices << ends;
    drawString(ss.str().c_str(), 1, screenHeight-TEXT_HEIGHT, color, font);
    ss.str("");

    ss << "Max Elements Indices: " << maxIndices << ends;
    drawString(ss.str().c_str(), 1, screenHeight-(2*TEXT_HEIGHT), color, font);
    ss.str("");

    // unset floating format
    ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);

    // restore projection matrix
    glPopMatrix();                   // restore to previous projection matrix

    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW);      // switch to modelview matrix
    glPopMatrix();                   // restore to previous modelview matrix
}



///////////////////////////////////////////////////////////////////////////////
// set projection matrix as orthogonal
///////////////////////////////////////////////////////////////////////////////
void toOrtho()
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    // set orthographic viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glOrtho(0, screenWidth, 0, screenHeight, -1, 1);
	gluOrtho2D(0, screenWidth, 0, screenHeight);

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



///////////////////////////////////////////////////////////////////////////////
// set the projection matrix as perspective
///////////////////////////////////////////////////////////////////////////////
void toPerspective()
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)(screenWidth)/screenHeight, 1.0f, 1000.0f); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



void to2Dprojection(int width, int height)
{
	//int vPort[4];
    
	//glGetIntegerv(GL_VIEWPORT, vPort);
	glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	    
	//glOrtho(0, vPort[2], 0, vPort[3], -1, 1);
	glOrtho(0, width, 0, height, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}





//=============================================================================
// CALLBACKS
//=============================================================================

void displayCB()
{
    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // save the initial ModelView matrix before modifying ModelView matrix
    glPushMatrix();

    // tramsform camera
    //glTranslatef(0, 0, -cameraDistance);
    //glRotatef(cameraAngleX, 1, 0, 0);   // pitch
    //glRotatef(cameraAngleY, 0, 1, 0);   // heading

	//draw0();
    //draw1();        // with immediate mode, glBegin()-glEnd() block
    //draw2();        // with glDrawArrays()
    draw3();        // with glDrawElements()
    //draw5();        // with glDrawElements() with interleave vertex array
    //draw4();        // with glDrawRangeElements()


    // print 2D text
    float pos[4] = {-4.0f,3.5f,0,1};
    float color[4] = {1,1,1,1};
    //drawString3D("Immediate", pos, color, font);
    pos[0] = 0.5f;
    drawString3D("glDrawArrays()", pos, color, font);
    pos[0] = -5.0f; pos[1] = -4.0f;
    //drawString3D("glDrawElements()", pos, color, font);
    pos[0] = 0.5f;
    //drawString3D("glDrawRangeElements()", pos, color, font);

    showInfo();     // print max range of glDrawRangeElements

    glPopMatrix();

    glutSwapBuffers();
}


void reshapeCB(int w, int h)
{
    screenWidth = w;
    screenHeight = h;
    //toPerspective();
	to2Dprojection(w,h);
}


void timerCB(int millisec)
{
    glutTimerFunc(millisec, timerCB, millisec);
    glutPostRedisplay();
}


void keyboardCB(unsigned char key, int x, int y)
{
    switch(key)
    {
    case 27: // ESCAPE
        clearSharedMem();
        exit(0);
        break;

    case 'd': // switch rendering modes (fill -> wire -> point)
    case 'D':
        drawMode = ++drawMode % 3;
        if(drawMode == 0)        // fill mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }
        else if(drawMode == 1)  // wireframe mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        else                    // point mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        break;

    default:
        ;
    }
}


void mouseCB(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

    if(button == GLUT_LEFT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if(state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if(button == GLUT_RIGHT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if(state == GLUT_UP)
            mouseRightDown = false;
    }

    else if(button == GLUT_MIDDLE_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseMiddleDown = true;
        }
        else if(state == GLUT_UP)
            mouseMiddleDown = false;
    }
}


void mouseMotionCB(int x, int y)
{
    if(mouseLeftDown)
    {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if(mouseRightDown)
    {
        cameraDistance -= (y - mouseY) * 0.2f;
        mouseY = y;
    }
}

#elif defined(MERCATOR_TEST)
//
//  Vector.hpp
//
//  This class is intended to provide a bare minimum for handling vectors
//  in order to avoid cluttering up the example with the implementation
//  of a robust Vector class.
//
#include "GLTools.h"


class Vector {
protected:
   double v[2];
public:
   // constructor for use with initial values
   Vector ( double a = 0.0, double b = 0.0 )
      { v[0] = a; v[1] = b; }
   // copy operator
   Vector &operator = ( Vector &source )
      {
         v[0] = source[0]; v[1] = source[1];
         return *this;
      }
   // subscript operator (abstract)
   double &operator [] ( int index )
      {
         return (index >= 0 && index <2) ? v[index] : v[0];
      }
};

//
//  The following class enforces a convention for using a Vector as
//  a spherical coordinate.
//
typedef double Degrees;
enum SphereIndex { LON = 0, LAT };
class Spherical : public Vector {
public:
   // subscript operator
   Degrees &operator [] ( SphereIndex index )
      {
          return v[index];
      }
};

//
//  The following class enforces a convention for using a Vector as
//  a planar coordinate.
//
typedef double Inches;
enum PlaneIndex { X=0, Y };
class Planar : public Vector {
public:
   // subscript operator
   Inches &operator [] ( PlaneIndex index )
      {
          return v[index];
      }
};

//
//  Projection Mathematics
//
//      Author - David T. Lowerre   ;{
//              8/27/93
//

//#include "vector.hpp"

enum { CLARKE = 0, WGS72, WGS84 };            // Geodetic Models

//
//  Projection Class
//
//   The projection class is ABSTRACT which means that you cannot
//   declare an object of this class. You can only derive a new
//   class from this one. To make the new class useable, you must
//   override the virtual methods LatLontoXY() and XYtoLatLon().
//
class Projection {
protected:
   int Model;
   double e, e2;          // eccentricity and eccentricity squared
   Planar xyResult;
   Spherical llResult;

public:

   Projection( int model );

   virtual Planar &XY( Spherical &ll ) = 0;
   virtual Spherical &LatLon( Planar &xy ) = 0;
};

//
//  This class converts from latitude and longitude to X and Y
//  coordinates using Mercator's Cylindrical projection. The system
//  must be set up by specifying the borders of the area in latitude
//  and longitude and the scale at the true scale latitude
//
//
//                                max(lat, lon)
//        -----------------------------
//        |                           | 
//        |                           | 
//        |                           | 
//        |...........................|  true scale latitude
//        |                           |
//        |                           |
//        |                           |
//        |                           |
//        -----------------------------
//    min(lat,lon)
//
//  Now, given a point p(lat, lon), we can find the corresponding X and Y
//  coordinates.
//
class Mercator : public Projection
{
   Spherical MinLL, MaxLL;
   double Scale;
   double Fmin;

   double F( double lat );
   void NewScale( double lat, double scale );

public:

   // constructors
   Mercator( int model,               // geodetic model
           Spherical &min,          // minimum lat and lon
           Spherical &max,          // maximum lat and lon
           double scale );          // scale at true scale latitude
   Mercator( int model,               // geodetic model
           Spherical &origin,       // center/origin lat and lon
           double scale );          // scale at origin

   // initialization methods
   void SetBounds( Spherical &min, Spherical &max, double scale );
   void SetOrigin( Spherical &origin, double scale );

   // conversion methods
   Planar &XY( Spherical &ll );
   Spherical &LatLon( Planar &xy );
};

//
//  Projection Mathematics
//
//      Author - David T. Lowerre     ;{
//              8/27/93
//
//
//  Projection Mathematics were derived from the formulas and descripions
//  in
//      American Practical Navigator, Bowditch
//      Conformal Projections in Geodesy and Cartography, Thomas
//              Map Projections-a Working Manual, Snyder
//
//#include "maproj.hpp"
#include <math.h>
#ifndef M_PI
#define M_PI 3.141592657
#define M_PI_2 M_PI*M_PI
#endif
/* conversion from degrees to radians */
const double toDeg = 180.0 / M_PI;
#define RAD(x) ((x)/toDeg)
/* conversion from radians to degrees */
#define DEG(x) ((x)*toDeg)

// Geodetic Models
//  These models differ primarily by their measurement of the polar
//  and equatorial radius of the earth
//
static struct {
   double P, Q;   // Polar and Equatorial Radii
} Radius[] =
{
   3432.275, 3443.950,    // Clarke 1866
   3432.370, 3443.917,    //WGS-72
   3432.371, 3443.918,    //WGS-84
};

Projection::Projection( int model )
{
   Model = model;
   double p = Radius[Model].P;
   double q = Radius[Model].Q;

   // for most projection mathematics, we need the 'eccentricity' and
   // the eccentricity squared. So we compute them once when the
   // projection is constructed.
   e2 = 1 - (p * p) / (q * q); // eccentricity squared
   e = sqrt( e2 );                         // eccentricity of spheroid
}

//  These functions convert from latitude and longitude to X and Y
//  coordinates using Mercator's Cylindrical projection. The system
//  must be set up by specifying the borders of the area in latitude
//  and longitude and the scale at the true scale latitude
//
//
//                                   max(lat, lon)
//         -------------------------------
//         |                             |
//         |                             |
//         |                             |
//         |.............................|  true scale latitude
//         |                             |
//         |                             |
//         |                             |
//         |                             | 
//         -------------------------------
//    min(lat,lon)
//
//  Now, given a point p(lat, lon), we can find the corresponding X and Y
//  coordinates.
//

Mercator::Mercator( int model, Spherical &min, Spherical &max,
                 double scale ) :
   Projection( model )
{
   SetBounds( min, max, scale );
}

Mercator::Mercator( int model, Spherical &origin, double scale ) :
   Projection( model )
{
   SetOrigin( origin, scale );
}

void Mercator::NewScale( double lat, double scale )
{
   // compute scale factor
   double sinLat = sin(RAD(lat));

   double Kpt = sqrt( 1.0 -
      e2 * sinLat * sinLat) / cos(RAD(lat) );

   double So = scale * Kpt;

   // Conversion factor for radians to inches
   Scale =
      72913.2 *               // number of inches in a nautical mile
         Radius[Model].Q /               // equatorial radius
            So;
}

//
// SetBounds - Change the parameters for the projection
//
void Mercator::SetBounds( Spherical &min, Spherical &max, double scale )
{
   // save the extrema
   MinLL = min;
   MaxLL = max;

   // recalculate the scale
   NewScale( (MinLL[LAT] + MaxLL[LAT]) / 2, scale );

   Fmin = F(RAD(MinLL[LAT]));   // compute once for later use
}

//
// SetOrigin - Change the origin for the projection
//
void Mercator::SetOrigin( Spherical &origin, double scale )
{

   // save the origin
   MinLL = origin;
   MaxLL = origin;

   // calculate the scale
   NewScale( origin[LAT], scale );

   Fmin = F(RAD(MinLL[LAT]));   // compute once for later use
}

//
// F(lat) converts from latitude to an unscaled Y value.
//  This is used both in transforming
//  from lat/lon to X/Y and X/Y to lat/lon.
//  latitude must be supplied in RADIANS
//             --                                             --
//             |  1+sin( |lat| )  -- 1 - e sin ( |lat| ) -- e  |
//  Y = 0.5 ln |  --------------  |  -------------------  |    |
//             |  1-sin( |lat| )  -- 1 + e sin ( |lat| ) --    |
//             --                                             --
double Mercator::F( double lat )
{
   double SinLat = sin( fabs(lat) );

   double eSinLat = e * SinLat;

   double Ym = log( (1+SinLat)/(1-SinLat) *
                    pow((1-eSinLat)/(1+eSinLat), e) );
   Ym *= 0.5;

   // result must assume the sign of the input latitude
   if( (lat < 0 && Ym > 0) ||
      (lat > 0 && Ym < 0 ) )
      Ym = -Ym;

   return Ym;
}

Planar &Mercator::XY( Spherical &ll )
{
   // X is easy, its just the delta in longitude times the scale
   xyResult[X] = RAD(ll[LON] - MinLL[LON]);
   xyResult[X] *= Scale;

   // Y is harder
   xyResult[Y] = F(RAD(ll[LAT])) - Fmin;
   xyResult[Y] *= Scale;

   return xyResult;
}

Spherical &Mercator::LatLon( Planar &xy )
{
   // Once again, X is easy.  Compute the longitude delta
   llResult[LON] = DEG( xy[X] / Scale );   // un-scale the X value
   llResult[LON] += MinLL[LON];            // add minimum longitude

   // As you might suspect, Latitude is harder. We must use
   // successive approximations to approach a suitable answer
   double Ym = xy[Y] / Scale + Fmin;
   double t = exp( -Ym );

   // initial latitude approximation
   double lat[2];
   lat[1] = M_PI_2 - 2.0 * atan( t );

   do {
      // save the old latitude
      lat[0] = lat[1];

      // Use old latitude to compute a more accurate answer
      double eSinLat = e * sin( lat[0] );
      double f = pow((1-eSinLat)/(1+eSinLat), e/2 );
      lat[1] = M_PI_2 - 2.0 * atan( t * f );

     // continue until the difference is acceptably small
   } while( fabs(lat[1]-lat[0]) > 0.1e-5 );

   // convert from radians to degrees
   llResult[LAT] = DEG(lat[1]);

   return llResult;
}

//
//  Compile this code with MAIN = 1 to build the following test code.
//


#ifdef MAIN

#include <stdio.h>

//
// Get a spherical coordinate from the user
//
Degrees GetCoord( char *val )
{
   double deg, min;
   printf( "Enter %s in Degrees/Minutes: ", val );
   scanf( "%lf %lf", &deg, &min );
   if( deg < 0 )
      min = -min;
   return deg + min/60.0;
}

//
//  Get a planar coordinate from the user
//
Inches GetInches( char *val )
{
   double res;

   printf( "Enter %s in inches: ", val );
   scanf( "%lf", &res );

   return res;
}

void main()
{

   Spherical min, max, ll;
   Planar xy;

   double scale, dummy;

   printf( "\nMercator Transform\n\n" );

   printf( "Enter Map Scale 1:" );
   scanf( "%lf", &scale );

   min[LAT] = GetCoord( "True Scale Latitude" );

   min[LON] = GetCoord( "Minimum Longitude" );

   Mercator *Merc = new Mercator( 2, min, scale );

   int done = 0;
   while( !done )
   {
      flushall();

      printf( "\n Choose:\n"
             "  1. Convert LAT/LON to X/Y\n"
             "  2. Convert X/Y to LAT/LON\n"
             "  3. Exit\n"
             "> " );

      switch( getchar() )
      {
      case '1':
         // convert from Lat/Lon in degrees/minutes to X/Y in inches
         ll[LAT] = GetCoord( "Point Lat" );
         ll[LON] = GetCoord( "Point Lon" );

         xy = Merc->XY( ll );

         printf( "X = %f Y = %f ( in meters)\n\n",
            xy[X]*0.0254, xy[Y]*0.0254 );
         break;

      case '2':
         // convert from X/Y in inches to Lat/Lon in degrees/minutes
         xy[X] = GetInches( "Point X" );
         xy[Y] = GetInches( "Point Y" );

         ll = Merc->LatLon( xy );
         printf( "Lat = %d %5.2f  Lon = %d %5.2f\n\n",
            (int)ll[LAT], fabs(modf( ll[LAT], &dummy ) * 60.0),
            (int)ll[LON], fabs(modf( ll[LON], &dummy ) * 60.0) );
         break;

      case '3':
         done = 1;
         break;

      default:
         break;
      }
   }
}

#endif
// End of File
#endif //_ATMOSPHERE