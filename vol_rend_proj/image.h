/***************************************************************************
                          image.h  -  description
                             -------------------
    begin                : Thu Jul 28 2005
    copyright            : (C) 2005 by Hongfeng Yu
    email                : hfyu@ucdavis.edu
 ***************************************************************************/
 
#ifndef CLASS_IMAGE
#define CLASS_IMAGE

#include <string>

using namespace std;

#define IMAGE_TGA	0x1000
#define IMAGE_JPEG	0x2000
#define IMAGE_TIFF	0x3000

struct A3D_TGA_HEADER{
	unsigned char nID;
	unsigned char nColorMapType;
	unsigned char nImageType;
	unsigned short nColorMapOrigin;
	unsigned char nColorMapLength;
	unsigned char nColorMapWidth;
	unsigned short nXOrigin;
	unsigned short nYOrigin;
	unsigned short nImageWidth;
	unsigned short nImageHeight;
	unsigned short nFlags;
};

class Image{
        long cols;
        long rows;
        unsigned char colornum;
        long size;
        unsigned char *image;
        unsigned char *image2;
public:
        bool readImage(char *name);
        bool writeImage(char *name);
        Image() { cols=rows=size=0; colornum=0; image=NULL; }
        Image(long c, long r, int cn=3);
        ~Image() { if(image) delete []image; }
        void setParameters(long c, long r, int cn);
        void tiff2Image(char *name);
        void image2Tiff(const char *filename);
        void image2Jpeg(const char *filename);
        void image2TGA(const char * filename);
        void readBuffer();
        void saveImage(string & S, int nMode);
        unsigned char * getBuffer(){return image2;}
};

#endif
