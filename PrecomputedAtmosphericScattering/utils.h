#ifndef _UTILS_H
#define _UTILS_H
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void opengl_version(){
	const char* vendor = (const char*)glGetString( GL_VENDOR );
	const char* renderer = (const char*)glGetString( GL_RENDERER );
	const char* version = (const char*)glGetString( GL_VERSION );
	const char* extensions = (const char*)glGetString( GL_EXTENSIONS );
	char fn[MAX_PATH];
	sprintf(fn,"c:\\Temp\\pas\\opengl_info.txt");
	FILE *fp = fopen(fn, "w");
	fprintf(fp,"%s, %s, %s, %s\n",vendor, renderer, version, extensions);
	fclose(fp);
}

typedef struct {
     unsigned char red,green,blue;
} PPMPixel;

typedef struct {
     int x, y;
     PPMPixel *data;
} PPMImage;

#define CREATOR "RPFELGUEIRAS"
#define RGB_COMPONENT_COLOR 255

static PPMImage *readPPM(const char *filename)
{
     char buff[16];
     PPMImage *img;
     FILE *fp;
     int c, rgb_comp_color;
     //open PPM file for reading
     fp = fopen(filename, "rb");
     if (!fp) {
          fprintf(stderr, "Unable to open file '%s'\n", filename);
          exit(1);
     }

     //read image format
     if (!fgets(buff, sizeof(buff), fp)) {
          perror(filename);
          exit(1);
     }

    //check the image format
    if (buff[0] != 'P' || buff[1] != '6') {
         fprintf(stderr, "Invalid image format (must be 'P6')\n");
         exit(1);
    }

    //alloc memory form image
    img = (PPMImage *)malloc(sizeof(PPMImage));
    if (!img) {
         fprintf(stderr, "Unable to allocate memory\n");
         exit(1);
    }

    //check for comments
    c = getc(fp);
    while (c == '#') {
    while (getc(fp) != '\n') ;
         c = getc(fp);
    }

    ungetc(c, fp);
    //read image size information
    if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {
         fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
         exit(1);
    }

    //read rgb component
    if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
         fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
         exit(1);
    }

    //check rgb component depth
    if (rgb_comp_color!= RGB_COMPONENT_COLOR) {
         fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
         exit(1);
    }

    while (fgetc(fp) != '\n') ;
    //memory allocation for pixel data
    img->data = (PPMPixel*)malloc(img->x * img->y * sizeof(PPMPixel));

    if (!img) {
         fprintf(stderr, "Unable to allocate memory\n");
         exit(1);
    }

    //read pixel data from file
    if (fread(img->data, 3 * img->x, img->y, fp) != img->y) {
         fprintf(stderr, "Error loading image '%s'\n", filename);
         exit(1);
    }

    fclose(fp);
    return img;
}

int saveTextureToPPM(GLenum id, GLuint texture, 
						int texX, int texY, int texZ, 
						int sizeDim, 
						char* filename, 
						GLenum format, 
						bool bDebug = false)
{
  //int dim = 3; //texDim
  float* pTexBuffer = new float[texX * texY * texZ * sizeDim];
  //glReadBuffer(id);
  glBindTexture(GL_TEXTURE_3D, texture);
  glGetTexImage(GL_TEXTURE_3D, 0, format, GL_FLOAT, pTexBuffer);
  
  int i, j;
  char fn[MAX_PATH];
  char fnt[MAX_PATH];
  for(int d = 0; d < texZ; d++){
	sprintf(fn,"%s_%d.ppm",filename,d);
	sprintf(fnt,"%s_%d.txt",filename,d);
	FILE *fp = fopen(fn, "wb");
	FILE *fpt = fopen(fnt, "w");
	(void) fprintf(fp, "P6\n%d %d\n255\n", texX, texY);
  
	for(int i=0; i<texX; i++){
		for(int j=0; j<texY; j++){
			static unsigned char color[3];
		    color[0] = pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim]*(float)255;  // red
		    color[1] = pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim+1]*(float)255;  // green
		    color[2] = pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim+2]*(float)255;  // blue 
		    (void) fwrite(color, 1, 3, fp);
#ifdef _STORAGE_TEST_
			fprintf(fpt,"[%d][%d] = %f, %f, %f\n",i,j,
				acos(pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim])*180.0/M_PI,
				acos(pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim+1])*180.0/M_PI,
				acos(pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim+2])*180.0/M_PI);
#else
			fprintf(fpt,"[%d][%d] = %f, %f, %f\n",i,j,
				pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim],
				pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim+1],
				pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim+2]);
#endif
		}
	}
	fclose(fpt);
	fclose(fp);
  }
  return EXIT_SUCCESS;
}

//assumptions: texture id is 2D texture
void saveTextureToXYZ(GLenum id, int texX, int texY, 
					int sizeDim, 
					char* filename, 
					GLenum format, 
					bool bDebug = false)
{
	//int dim = 3; //texDim
	float* pTexBuffer = new float[texX * texY * sizeDim];
	glReadBuffer(id);
	glReadPixels(0, 0, texX, texY, format, GL_FLOAT, pTexBuffer);
	//glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, 0, 0, RES_MU_S * RES_NU, RES_MU, 0 );
	//glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pTexBuffer ); 
	char fn[MAX_PATH];
	sprintf(fn,"%s.ppm",filename);
	FILE *fp = fopen(fn, "wb"); 
	if(!fp){
		delete [] pTexBuffer;
		printf("\nERROR: cannot open file %s\n",filename);
		return;
	}
	(void) fprintf(fp, "P6\n%d %d\n255\n", texX, texY);
	for(int i=0; i<texX; i++){
		for(int j=0; j<texY; j++){
			static unsigned char color[3];
		    color[0] = pTexBuffer[i*texY*sizeDim + j*sizeDim]*(float)256;  // red
		    color[1] = pTexBuffer[i*texY*sizeDim + j*sizeDim+1]*(float)256;;  // green
		    color[2] = pTexBuffer[i*texY*sizeDim + j*sizeDim+2]*(float)256;;  // blue 
		    (void) fwrite(color, 1, 3, fp);
		}
	}
	fclose(fp);
}

//
void saveTextureToXYZ3D(GLenum id, GLuint texture, 
						int texX, int texY, int texZ, 
						int sizeDim, 
						char* filename, 
						GLenum format, 
						bool bDebug = false)
{
	//int dim = 3; //texDim
	float* pTexBuffer = new float[texX * texY * texZ * sizeDim];
	//glReadBuffer(id);
	glBindTexture(GL_TEXTURE_3D, texture);
	//glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, pTexBuffer);
	glGetTexImage(GL_TEXTURE_3D, 0, format, GL_FLOAT, pTexBuffer);

	char fn[MAX_PATH];
	sprintf(fn,"%s.xyz",filename);
	FILE* fp = fopen(fn,"w");
	if(!fp){
		printf("\nERROR: cannot open file %s\n",fn);
		return;
	}
	for(int d = 0; d < texZ; d++){
		for(int i=0; i<texX; i++){
			for(int j=0; j<texY; j++){
				for(int k=0; k<sizeDim; k++){
					if(k < sizeDim -1)
						fprintf(fp,"%f ",pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim+k]);
					else
						fprintf(fp,"%f",pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim+k]);
				}
				fprintf(fp,"\n");
			}
		}
		fprintf(fp,"____d=%d________________\n",d);
	}
	fclose(fp);

	if(bDebug){
		for(int d = 0; d < texZ; d++){
			float max_x = -100.0;
			float max_y = -100.0;
			float max_z = -100.0;
			float max_w = -100.0;
			//assert(sizeDim == sizeDim);
			//sizeDim = 3;
			unsigned char *a_f =  (unsigned char*)new unsigned char[sizeDim*texY*texX];
			for(int i = 0; i < texX; i++){
				for(int j = 0; j < texY; j++){
					if(max_x < pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim]) max_x = pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim];
					if(max_y < pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim + 1]) max_y = pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim + 1];
					if(sizeDim >= 3)
						if(max_z < pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim + 2]) max_z = pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim + 2];
					//if(sizeDim == 4)
					//	if(max_z < pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim + 3]) max_z = pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim + 3];
				}
			}

			for(int i = 0; i < texX; i++){
				for(int j = 0; j < texY; j++){
					a_f[i*texY*sizeDim + j*sizeDim] = (unsigned char)((255)*(pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim]/max_x));
					a_f[i*texY*sizeDim + j*sizeDim + 1] = (unsigned char)((255)*(pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim + 1]/max_y));
					if(sizeDim >= 3)
						a_f[i*texY*sizeDim + j*sizeDim + 2] = (unsigned char)((255)*(pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim + 2]/max_z));
					if(sizeDim == 4)
						a_f[i*texY*sizeDim + j*sizeDim + 3] = (unsigned char)((255)*(pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim + 3]/max_z));
				}
			}
			char debugfn[MAX_PATH];
			sprintf(debugfn,"%s_%d.ppm",filename,d);
			//cutSavePPMub( debugfn, a_f, texX, texY);
			delete [] a_f;
		}
	}
	delete [] pTexBuffer;
}


#endif //_UTILS_H