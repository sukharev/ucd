/***************************************************************************
                          image.cpp  -  description
                             -------------------
    begin                : Thu Jul 28 2005
    copyright            : (C) 2005 by Hongfeng Yu
    email                : hfyu@ucdavis.edu
 ***************************************************************************/
#ifndef LINUX
#include <windows.h>
#endif

#include <stdlib.h>

#include <iostream>
#include <fstream>
#include "image.h"
// #include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <float.h>
//#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>


/*
extern "C"{
#include "jpeglib.h"
}
*/


using namespace std;

Image::Image(long c, long r, int cn)
{
        cols=c; rows=r; colornum=cn;
        size=cols*rows*colornum;
        image=NULL;
        image2=NULL;
}

void Image::setParameters(long c, long r, int cn)
{
        cols=c; rows=r; colornum=cn;
        size=cols*rows*colornum;
        if(image) delete []image;
        image=NULL;
        image2=NULL;
}

bool Image::readImage(char *name) {
        ifstream fp(name, ios::in | ios::binary);
        if(fp==NULL) {
                cerr<<"Cannot Open File "<<name<<endl;
                return false;
        }

        fp.read((char*)&cols, sizeof(long));
        fp.read((char*)&rows, sizeof(long));
        fp.read((char*)&colornum, sizeof(unsigned char));

        size=cols*rows*colornum;
        if(image) delete []image;
        image=new (unsigned char)(size);
        if(image==NULL) {
                cerr<<"Image Allocation Failed!"<<endl;
                exit(1);
        }
        fp.read((char*)image, sizeof(unsigned char)*size);
        fp.close();

        return true;
}
        
bool Image::writeImage(char *name)
{
        if(image==NULL) {
                cerr<<"The Image is Empty!"<<endl;
                return false;
        }

        fstream fp(name, ios::out|ios::binary);
		if(!fp) {// ==NULL) {
                cerr<<"Cannot Create File "<<name<<endl;
                return false;
        }

        fp.write((const char*)&cols, sizeof(long));
        fp.write((const char*)&rows, sizeof(long));
        fp.write((const char*)&colornum, sizeof(unsigned char));

        fp.write((const char*)image, sizeof(unsigned char)*size);
        fp.close();
        return true;
}

void Image::tiff2Image(char *name)
{
}

void Image::image2Tiff(const char *name)
{
        if(image==NULL) {
                cerr<<"The Image is Empty!"<<endl;
                return;
        }
        
        long strip;

        /* open file and check for errors */
        FILE *f = fopen (name, "wb");
        if (f == NULL) {
                fprintf (stderr, "ERROR: Fail to Create File %s", name);
                exit (-1);
        }

        /* write the tiff header */
        fputc('M',f); fputc('M',f);  /* byte order */
        fputc(0,f); fputc(42,f); /* "version" number */
        fputc(0,f); fputc(0,f); fputc(0,f); fputc(0x10,f); /* ifd offset */

        /* write the ifd */
        fseek(f,0x10,SEEK_SET); /* beginning of ifd */
        fputc(0,f); fputc(14,f); /* number of entries */

        fputc(0x00,f); fputc(0xfe,f); /* tag 254 - NEW SUBFILE TYPE */
        fputc(0x00,f); fputc(0x04,f); /* type LONG */
        fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
        fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); /* the usual */

        fputc(0x01,f); fputc(0x00,f); /* tag 256 - I_WIDTH */
        fputc(0x00,f); fputc(0x04,f); /* type LONG */
        fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
        fputc(0x00,f); fputc(0x00,f); fputc(cols / 256 % 256,f); fputc(cols % 256,f); /* the data */

        fputc(0x01,f); fputc(0x01,f); /* tag 257 - I_LENGTH */
        fputc(0x00,f); fputc(0x04,f); /* type LONG */
        fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
        fputc(0x00,f); fputc(0x00,f); fputc(rows / 256 % 256,f); fputc(rows % 256,f); /* the data */

        fputc(0x01,f); fputc(0x02,f); /* tag 258 - BITS_PER_SAMPLE */
        fputc(0x00,f); fputc(0x03,f); /* type SHORT */

        if (colornum == 1) { /* 8-bit grayscale */
                fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
                fputc(0x00,f); fputc(0x08,f); fputc(0x00,f); fputc(0x00,f); /* the data - one byte per pixel */
        } /* end if(colors == 1) */
        else if (colornum == 3) {  /* 24-bit color */
                fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x03,f); /* three of them */
                fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); fputc(0x20,f); /* location of the data */
        } /* end if(colors == 3) */

        fputc(0x01,f); fputc(0x03,f); /* tag 259 - COMPRESSION */
        fputc(0x00,f); fputc(0x03,f); /* type SHORT */
        fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
        fputc(0x00,f); fputc(0x01,f); fputc(0x00,f); fputc(0x00,f); /* no compression */

        fputc(0x01,f); fputc(0x06,f); /* tag 262 - PHOTOMETRIC INTERPRETATION */
        fputc(0x00,f); fputc(0x03,f); /* type SHORT */
        fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
        if (colornum == 1) {
                fputc(0x00,f); fputc(0x01,f); fputc(0x00,f); fputc(0x00,f); /* grayscale level; Black is zero */
        } /* end if(colors) */
        else if (colornum == 3) {
                fputc(0x00,f); fputc(0x02,f); fputc(0x00,f); fputc(0x00,f); /* 24-bit rgb color; Black is zero */
        } /* end if(colors) */

        fputc(0x01,f); fputc(0X11,f); /* tag 273 - STRIP OFFSETS */
        fputc(0x00,f); fputc(0x04,f); /* type LONG */
        if (colornum == 1) {
                fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
                fputc(0x00,f); fputc(0x00,f); fputc(0x03,f); fputc(0x00,f); /* beginning of first (and only) strip */
        } /* end if(colors) */
        else if (colornum == 3) {
                fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x03,f); /* 3 of them */
                fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); fputc(0x30,f); /* offset of first strip offset */
        } /* end if(colors) */
  
        fputc(0x01,f); fputc(0x15,f); /* tag 277 - SAMPLES PER PIXEL */
        fputc(0x00,f); fputc(0x03,f); /* type SHORT */
        fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
        if (colornum == 1) { /* 8-bit grayscale */
                fputc(0x00,f); fputc(0x01,f); fputc(0x00,f); fputc(0x00,f); /* one sample per pixel */
        } /* end if(colors) */
        else if (colornum == 3) { /* 24-bit rgb color */
                fputc(0x00,f); fputc(0x03,f); fputc(0x00,f); fputc(0x00,f); /* three samples per pixel */
        } /* end if(colors) */

        fputc(0x01,f); fputc(0x16,f); /* tag 278 - ROWS PER STRIP */
        fputc(0x00,f); fputc(0x04,f); /* type LONG */
        fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
        fputc(0x00,f); fputc(0x00,f); fputc(rows /256 % 256,f); fputc(rows % 256,f); /* the count */

        fputc(0x01,f); fputc(0x17,f); /* tag 279 - STRIP BYTE COUNTS */
        fputc(0x00,f); fputc(0x04,f); /* type LONG */

        if (colornum == 1) {
                fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
                fputc(size / 256 / 256 / 256 % 256, f);
                fputc(size / 256 / 256 % 256, f);
                fputc(size / 256 % 256,f);
                fputc(size % 256,f); /* the count */
        } /* end if(color) */
        else if (colornum == 3) {
                fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x03,f); /* three of them */
                fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); fputc(0x60,f); /* location of first item */
        } /* end if(color) */

        fputc(0x01,f); fputc(0x1A,f); /* tag 282 - X_RESOLUtiON */
        fputc(0x00,f); fputc(0x05,f); /* type RATIONAL */
        fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
        fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); fputc(0x00,f); /* location of the data */

        fputc(0x01,f); fputc(0x1B,f); /* tag 283 - Y_RESOLUTION */
        fputc(0x00,f); fputc(0x05,f); /* type RATIONAL */
        fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
        fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); fputc(0x10,f); /* location of the data */

        fputc(0x01,f); fputc(0x1C,f); /* tag 284 - PLANAR_CONFIGURATION */
        fputc(0x00,f); fputc(0x03,f); /* type SHORT */
        fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
        fputc(0x00,f); fputc(0x02,f); fputc(0x00,f); fputc(0x00,f); /* planar format */

        fputc(0x01,f); fputc(0x28,f); /* tag 296 - RESOLUTION_UNIT */
        fputc(0x00,f); fputc(0x03,f); /* type SHORT */
        fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
        fputc(0x00,f); fputc(0x02,f); fputc(0x00,f); fputc(0x00,f); /* default value - inches */

        fputc(0,f); fputc(0,f); fputc(0,f); fputc(0,f); /* there is no other ifd */

        fseek(f,0x100,SEEK_SET); /* X_RESOLUTION data */
        fputc(0,f); fputc(0,f); fputc(0,f); fputc(72,f); /* 72/inch is default */
        fputc(0,f); fputc(0,f); fputc(0,f); fputc(1,f);
        fseek(f,0x110,SEEK_SET); /* Y_RESOLUTION data */
        fputc(0,f); fputc(0,f); fputc(0,f); fputc(72,f); /* 72/inch is default */
        fputc(0,f); fputc(0,f); fputc(0,f); fputc(1,f);
        fseek(f,0x120,SEEK_SET); /* BITS_PER_SAMPLE data */
        fputc(0x00,f); fputc(0x08,f); /* one byte per color */
        fputc(0x00,f); fputc(0x08,f);
        fputc(0x00,f); fputc(0x08,f);

        fseek(f,0x130,SEEK_SET); /* STRIP_OFFSETS data */
        strip = 0x300; /* beginning of red strip */
        fputc(strip / 256 / 256 / 256 % 256, f);
        fputc(strip / 256 / 256 % 256, f);
        fputc(strip / 256 % 256,f);
        fputc(strip % 256,f);
        strip = size / 3 + 0x300; /* beginning of green strip */
        fputc(strip / 256 / 256 / 256 % 256, f);
        fputc(strip / 256 / 256 % 256, f);
        fputc(strip / 256 % 256,f);
        fputc(strip % 256,f);
        strip = 2 * size / 3 + 0x300; /* beginning of blue strip */
        fputc(strip / 256 / 256 / 256 % 256, f);
        fputc(strip / 256 / 256 % 256, f);
        fputc(strip / 256 % 256,f);
        fputc(strip % 256,f);
        fseek(f,0x160,SEEK_SET); /* STRIP_BYTE_COUNTS data */
        fputc(size / 3 / 256 / 256 / 256 % 256, f); /* red */
        fputc(size / 3 / 256 / 256 % 256, f);
        fputc(size / 3 / 256 % 256,f);
        fputc(size / 3 % 256,f);
        fputc(size / 3 / 256 / 256 / 256 % 256, f); /* green */
        fputc(size / 3 / 256 / 256 % 256, f);
        fputc(size / 3 / 256 % 256,f);
        fputc(size / 3 % 256,f);
        fputc(size / 3 / 256 / 256 / 256 % 256, f); /* blue */
        fputc(size / 3 / 256 / 256 % 256, f);
        fputc(size / 3 / 256 % 256,f);
        fputc(size / 3 % 256,f);

        fseek(f,0x300,SEEK_SET); /* where to begin writing data */
        fwrite(image,1,size,f); /* write the data */
        fclose(f);
}


void Image::image2Jpeg(const char *filename)
{
	/*
	int image_width = cols;
	int image_height= rows;
	int quality = 999;
	
	JSAMPLE * image_buffer = image2;

	if (image_buffer == NULL){
		cerr << "Image::image2Jpeg: out of memory" << endl;
		exit(1000);
	}

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	FILE * outfile;
	JSAMPROW row_pointer[1];
	int row_stride;

	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_compress(&cinfo);

	if ((outfile = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		exit(1000);
	}
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = image_width;
	cinfo.image_height = image_height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);

	jpeg_start_compress(&cinfo, TRUE);

	row_stride = image_width * 3;


	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	fclose(outfile);

	jpeg_destroy_compress(&cinfo);
	*/
}

void Image::image2TGA(const char * filename)
{
	int nX = cols;
	int nY = rows;
	
	A3D_TGA_HEADER tgaHeader;
	char r,g,b;
	int i,j;

	tgaHeader.nID = 0x0;
	tgaHeader.nColorMapType = 0x0;
	tgaHeader.nImageType = 0x2;
	tgaHeader.nColorMapOrigin = 0x0;
	tgaHeader.nColorMapLength = 0x0;
	tgaHeader.nColorMapWidth = 0x0;
	tgaHeader.nXOrigin = 0x0;
	tgaHeader.nYOrigin = 0x0;
	tgaHeader.nImageWidth = nX;
	tgaHeader.nImageHeight = nY;
	tgaHeader.nFlags = 0x18;

	ofstream outf(filename);
	if(!outf){
		cerr << "Error: can not open " << filename << endl;
		exit(10000);
	}

	outf.write(reinterpret_cast<const char *>(&tgaHeader), sizeof(A3D_TGA_HEADER));

	unsigned char *image_buffer = image2;

	for (i=nY-1;i>=0;i--)
	for (j=0;j<nX;j++)
	{
		r = image_buffer[(i * nX + j)*3];
		g = image_buffer[(i * nX + j)*3+1];
		b = image_buffer[(i * nX + j)*3+2];
		outf.write(reinterpret_cast<const char*>(&b), sizeof(char));
		outf.write(reinterpret_cast<const char*>(&g), sizeof(char));
		outf.write(reinterpret_cast<const char*>(&r), sizeof(char));
	}
	outf.close();
}

void Image::readBuffer()
{
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT,viewport);
        cols=viewport[2]; rows=viewport[3];
        colornum=3;
        size=cols*rows*colornum;
        long buffersize=cols*rows*4;
        if(image != NULL)         
            delete []image;
            
//         if(image2 != NULL) 
//             delete []image2;
        
        image=NULL;
        image2=NULL;
        
        unsigned char *buffer=new unsigned char[buffersize];
        unsigned char *tbf=new unsigned char[buffersize];
        image=new unsigned char [size];
        image2=new unsigned char[size];
        if(buffer==NULL || image==NULL || tbf==NULL) {
                cerr<<"Image Allocation Failed!"<<endl;
                exit(1);
        }
        glReadPixels(0, 0, viewport[2], viewport[3], GL_RGBA, GL_UNSIGNED_BYTE, tbf);

        long p=0;
        for(int j=rows-1; j>=0; j--) {
                for(int i=j*cols*4, k=0; k<cols; k++, i+=4) {
                        buffer[p++]=tbf[i];
                        buffer[p++]=tbf[i+1];
                        buffer[p++]=tbf[i+2];
                        buffer[p++]=tbf[i+3];
                }
        }
                 
        
        float bc, ba;
        for(int i=0, a=3, j=0; i<buffersize; i+=4, j++, a+=4) {
                bc=buffer[i];
                ba=buffer[a];
                image[j]=(unsigned char)(bc*(ba/255.0));
                //image[j] = (unsigned char)bc;
                
        }
        for(int i=1, a=3, j=rows*cols; i<buffersize; i+=4, j++, a+=4) {
                bc=buffer[i];
                ba=buffer[a];
                image[j]=(unsigned char)(bc*(ba/255.0));
                //image[j] = (unsigned char)bc;                
        }
        for(int i=2, a=3, j=rows*cols*2; i<buffersize; i+=4, j++, a+=4) {
                bc=buffer[i];
                ba=buffer[a];
                image[j]=(unsigned char)(bc*(ba/255.0));
                //image[j] = (unsigned char)bc;                
        }

        for(int i=0; i<rows*cols; i++)
        {
		image2[i*3] = buffer[i*4];
		image2[i*3+1] = buffer[i*4+1];
		image2[i*3+2] = buffer[i*4+2];
        }

        if(buffer) delete []buffer;
        if(tbf) delete []tbf;

        //glRasterPos3f(10, 10, -5);
        //glDrawPixels(viewport[2],  viewport[3], GL_RGB, GL_UNSIGNED_BYTE, image);      
}

void Image::saveImage(string & S, int nMode)
{
	switch(nMode)
	{
		case IMAGE_TGA:
			S += ".tga";
			return image2TGA(S.c_str());
			break;
		case IMAGE_JPEG:
			S += ".jpeg";
			return image2Jpeg(S.c_str());
			break;
		case IMAGE_TIFF:
			S += ".tiff";
			return image2Tiff(S.c_str());
			break;
		default:
			cerr << "Unknown image format: currently only support TGA and JPEG. " << endl;
			break;
	}
}

