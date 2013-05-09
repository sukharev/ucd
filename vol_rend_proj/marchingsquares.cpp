#include "marchingsquares.h"
#include "GL/glew.h"
#include "GL/gl.h"
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>
#include <math.h>

using namespace std;

#define SLICE_X_AXIS 10000
#define SLICE_Y_AXIS 10001
#define SLICE_Z_AXIS 10002
#define GRID_SAMPLE     1024
#define MAX_NUM_LINES	3
using namespace std;


MarchingSquares::MarchingSquares(float contour_value, float** grid, int num_saved, int timestamp) 
: m_contour(contour_value), m_pGrid(grid), m_num_saved(num_saved), m_timestamp(timestamp)
{
	//m_contour_list.push_back(m_contour);
	m_bupdate_all = false;
}

MarchingSquares::~MarchingSquares()
{
}

void MarchingSquares::update(float contour_value, int num_saved, int timestamp)
{
	m_contour = contour_value;
	if(num_saved == 0 && m_num_saved > 0){
		m_contour_list.clear();
		m_num_saved = num_saved;
		m_timestamp = timestamp; 
 		return;
	}
	if(m_num_saved != num_saved)
		m_contour_list.push_back(m_contour);
	m_num_saved = num_saved;
	if(m_timestamp != timestamp)
		m_bupdate_all = true;
	m_timestamp = timestamp; 
}


//
// TODO: add 1D interpolation later; for now going to use mid line intersections
// assumption that the display screen will be 0 to 1.5 (from -0.75 to 0.75) 
//

void MarchingSquares::getCaseItrpl(float contour, int choice, float cellstartx, float cellstarty, 
				  float nextx, float nexty, DataRectangle* dr,
				  /* out */ Line* lines, /* out */int* num_lines)

{
	
	float alfav0v1;
	float alfav1v2;
	float alfav2v3;
	float alfav0v3;

	float v1, v2;

	switch(choice){
		case 0: //0x00000000
			break;
		
		
		case 11://0x00001011:
			alfav1v2 = (dr->datav1 - contour)/(dr->datav1 - dr->datav2);
			alfav2v3 = (dr->datav3 - contour)/(dr->datav3 - dr->datav2);
			if(alfav1v2 <0 || alfav2v3 < 0){
				cout << "getCase() problem\n";
			}
			*num_lines = 1;
			
			lines[0].x0 = nextx;
			lines[0].y0 = (nexty*(alfav1v2) + cellstarty*(1-alfav1v2));
 			lines[0].x1 = (nextx*(alfav2v3) + cellstartx*(1-alfav2v3));
			lines[0].y1 = nexty;

			v1 = (dr->datav1*(1-alfav1v2) + dr->datav2*(alfav1v2));
 			v2 = (dr->datav3*(1-alfav2v3) + dr->datav2*(alfav2v3));

		
			/*
			lines[1].bExtra = true;
			lines[1].x0 = nextx;
			lines[1].y0 = (nexty + cellstarty)/2;
			lines[1].x1 = (nextx + cellstartx)/2;
			lines[1].y1 = nexty;
			*/
			break;
		case 4: //0x00000100:
			alfav1v2 = (dr->datav2 - contour)/(dr->datav2 - dr->datav1);
			alfav2v3 = (dr->datav2 - contour)/(dr->datav2 - dr->datav3);
			if(alfav1v2 <0 || alfav2v3 < 0){
				cout << "getCase() problem\n";
			}
			*num_lines = 1;
			
			lines[0].x0 = nextx;
			lines[0].y0 = (nexty*(1-alfav1v2) + cellstarty*alfav1v2);
 			lines[0].x1 = (nextx*(1-alfav2v3) + cellstartx*alfav2v3);
			lines[0].y1 = nexty;
		
		/*
			v1 = (dr->datav2*alfav1v2 + dr->datav1*(1-alfav1v2));
			v2 = (dr->datav2*alfav2v3 + dr->datav3*(1-alfav2v3));
			assert(fabs(v1-v2) < 1e-3);
		*/	                                            
			/*
			lines[1].bExtra = true;
			lines[1].x0 = nextx;
			lines[1].y0 = (nexty + cellstarty)/2;
			lines[1].x1 = (nextx + cellstartx)/2;
			lines[1].y1 = nexty;
			*/
			break;
		case 5: //0x00000101: // ambiguous case 
			alfav1v2 = (dr->datav2 - contour)/(dr->datav2 - dr->datav1);
			alfav2v3 = (dr->datav2 - contour)/(dr->datav2 - dr->datav3);
			if(alfav1v2 <0 || alfav2v3 < 0){
				cout << "getCase() problem\n";
			}
			*num_lines = 2;
			
			lines[0].x0 = nextx;
			lines[0].y0 = (nexty*(1-alfav1v2) + cellstarty*alfav1v2);
 			lines[0].x1 = (nextx*(1-alfav2v3) + cellstartx*alfav2v3);
			lines[0].y1 = nexty;
			

			alfav0v1 = (dr->datav0 - contour)/(dr->datav0 - dr->datav1);
			alfav0v3 = (dr->datav0 - contour)/(dr->datav0 - dr->datav3);
			if(alfav0v1 < 0 || alfav0v3 < 0){
				cout << "getCase() problem\n";
			}
			*num_lines = 1;
			
			lines[1].x0 = cellstartx;
			lines[1].y0 = (nexty*alfav0v3 + cellstarty*(1-alfav0v3));
			lines[1].x1 = (nextx*alfav0v1 + cellstartx*(1-alfav0v1));
			lines[1].y1 = cellstarty;
	
			break;
		case 6: //0x00000110:
		case 9: //0x00001001:
			alfav0v1 = (dr->datav1 - contour)/(dr->datav1 - dr->datav0);
			alfav2v3 = (dr->datav2 - contour)/(dr->datav2 - dr->datav3);
			if(alfav0v1 <0 || alfav2v3 < 0){
				cout << "getCase() problem\n";
			}
			*num_lines = 1;
			
			lines[0].x0 = (nextx*(1-alfav0v1) + cellstartx*(alfav0v1));
			lines[0].y0 = cellstarty;
			lines[0].x1 = (nextx*(1-alfav2v3) + cellstartx*(alfav2v3));
			lines[0].y1 = nexty;

			/*
			lines[1].bExtra = true;
			lines[1].x0 = (nextx + cellstartx)/2;
			lines[1].y0 = cellstarty;
			lines[1].x1 = (nextx + cellstartx)/2;
			lines[1].y1 = nexty;
			*/
			break;
		case 7: //0x00000111:
		case 8: //0x00001000:
			alfav0v3 = (dr->datav0 - contour)/(dr->datav0 - dr->datav3);
			alfav2v3 = (dr->datav2 - contour)/(dr->datav2 - dr->datav3);
			if(alfav0v3 <0 || alfav2v3 < 0){
				cout << "getCase() problem\n";
			}
			
			*num_lines = 1;
			lines[0].x0 = cellstartx;
			lines[0].y0 = (nexty*alfav0v3 + cellstarty*(1-alfav0v3));
			lines[0].x1 = (nextx*(1-alfav2v3) + cellstartx*alfav2v3);
			lines[0].y1 = nexty;
			break;
		case 10: //0x00001010: // ambiguous case 
			*num_lines = 2;
			alfav0v3 = (dr->datav0 - contour)/(dr->datav0 - dr->datav3);
			alfav2v3 = (dr->datav2 - contour)/(dr->datav2 - dr->datav3);
			if(alfav0v3 <0 || alfav2v3 < 0){
				cout << "getCase() problem\n";
			}
			lines[0].x0 = cellstartx;
			lines[0].y0 = (nexty*alfav0v3 + cellstarty*(1-alfav0v3));
			lines[0].x1 = (nextx*(1-alfav2v3) + cellstartx*alfav2v3);
			lines[0].y1 = nexty;
			
			alfav0v1 = (dr->datav1 - contour)/(dr->datav1 - dr->datav0);
			alfav1v2 = (dr->datav1 - contour)/(dr->datav1 - dr->datav2);
			if(alfav0v1 <0 || alfav1v2 < 0){
				cout << "getCase() problem\n";
			}
			
			lines[1].x0 = (nextx*(1-alfav0v1) + cellstartx*alfav0v1);
			lines[1].y0 = cellstarty;
			lines[1].x1 = nextx;
			lines[1].y1 = (nexty*alfav1v2 + cellstarty*(1-alfav1v2));
			
			break;

		case 3: //0x00000011:
		case 12: //0x00001100:
			alfav0v3 = (dr->datav0 - contour)/(dr->datav0 - dr->datav3);
			alfav1v2 = (dr->datav1 - contour)/(dr->datav1 - dr->datav2);
			if(alfav0v3 <0 || alfav1v2 < 0){
				cout << "getCase() problem\n";
			}
			*num_lines = 1;
			lines[0].x0 = cellstartx;
			lines[0].y0 = (nexty*alfav0v3 + cellstarty * (1-alfav0v3));
			lines[0].x1 = nextx;
			lines[0].y1 = (nexty*alfav1v2 + cellstarty * (1-alfav1v2));
			break;
		case 2: //0x00000010:
		case 13: //0x00001101:
			alfav0v1 = (dr->datav1 - contour)/(dr->datav1 - dr->datav0);
			alfav1v2 = (dr->datav1 - contour)/(dr->datav1 - dr->datav2);
			if(alfav0v1 <0 || alfav1v2 < 0){
				cout << "getCase() problem\n";
			}
			*num_lines = 1;
			lines[0].x0 = (nextx*(1-alfav0v1) + cellstartx*alfav0v1);
			lines[0].y0 = cellstarty;
			lines[0].x1 = nextx;
			lines[0].y1 = (nexty*alfav1v2 + cellstarty*(1-alfav1v2));
			break;
		case 1: //0x00000001:
		case 14: //0x00001110:
			alfav0v1 = (dr->datav0 - contour)/(dr->datav0 - dr->datav1);
			alfav0v3 = (dr->datav0 - contour)/(dr->datav0 - dr->datav3);
			if(alfav0v1 < 0 || alfav0v3 < 0){
				cout << "getCase() problem\n";
			}
			*num_lines = 2;
			lines[0].x0 = cellstartx;
			lines[0].y0 = (nexty*alfav0v3 + cellstarty*(1-alfav0v3));
			lines[0].x1 = (nextx*alfav0v1 + cellstartx*(1-alfav0v1));
			lines[0].y1 = cellstarty;
			/*
			lines[1].bExtra = true;
			lines[1].x0 = cellstartx;
			lines[1].y0 = (nexty + cellstarty)/2;
			lines[1].x1 = (nextx + cellstartx)/2;
			lines[1].y1 = cellstarty;
			*/
			break;
		case 15: //0x00001111:
			break;
		default:
			break;
	}
}

/*
void MarchingSquares::getCaseGrid(int choice, float cellstartx, float cellstarty, 
				  float nextx, float nexty, DataRectangle* dr,
				   Line* lines, int* num_lines)
{
	
	//float alfav0v1;
	//float alfav1v2;
	//float alfav2v3;
	//float alfav0v3;
	
	switch(choice){
		case 0: //0x00000000
			break;
		
		case 4: //0x00000100:
		case 11://0x00001011:
			*num_lines = 1;
			lines[0].x0 = nextx;
			lines[0].y0 = (nexty + cellstarty)/2;
			lines[0].x1 = (nextx + cellstartx)/2;
			lines[0].y1 = nexty;
			break;
		case 5: //0x00000101: // ambiguous case 
			*num_lines = 2;
			lines[0].x0 = nextx;
			lines[0].y0 = (nexty + cellstarty)/2;
			lines[0].x1 = (nextx + cellstartx)/2;
			lines[0].y1 = nexty;

			lines[1].x0 = cellstartx;
			lines[1].y0 = (nexty + cellstarty)/2;
			lines[1].x1 = (nextx + cellstartx)/2;
			lines[1].y1 = cellstarty;
			
			break;
		case 6: //0x00000110:
		case 9: //0x00001001:
			*num_lines = 1;
			lines[0].x0 = (nextx + cellstartx)/2;
			lines[0].y0 = cellstarty;
			lines[0].x1 = (nextx + cellstartx)/2;
			lines[0].y1 = nexty;
			break;
		case 7: //0x00000111:
		case 8: //0x00001000:
			*num_lines = 1;
			lines[0].x0 = cellstartx;
			lines[0].y0 = (nexty + cellstarty)/2;
			lines[0].x1 = (nextx + cellstartx)/2;
			lines[0].y1 = nexty;
			break;
		case 10: //0x00001010: // ambiguous case 
			*num_lines = 2;
			lines[0].x0 = cellstartx;
			lines[0].y0 = (nexty + cellstarty)/2;
			lines[0].x1 = (nextx + cellstartx)/2;
			lines[0].y1 = nexty;

			lines[1].x0 = (nextx + cellstartx)/2;
			lines[1].y0 = cellstarty;
			lines[1].x1 = nextx;
			lines[1].y1 = (nexty + cellstarty)/2;
			break;

		case 3: //0x00000011:
		case 12: //0x00001100:
			*num_lines = 1;
			lines[0].x0 = cellstartx;
			lines[0].y0 = (nexty + cellstarty)/2;
			lines[0].x1 = nextx;
			lines[0].y1 = (nexty + cellstarty)/2;
			break;
		case 2: //0x00000010:
		case 13: //0x00001101:
			*num_lines = 1;
			lines[0].x0 = (nextx + cellstartx)/2;
			lines[0].y0 = cellstarty;
			lines[0].x1 = nextx;
			lines[0].y1 = (nexty + cellstarty)/2;
			break;
		case 1: //0x00000001:
		case 14: //0x00001110:
			*num_lines = 1;
			lines[0].x0 = cellstartx;
			lines[0].y0 = (nexty + cellstarty)/2;
			lines[0].x1 = (nextx + cellstartx)/2;
			lines[0].y1 = cellstarty;
			break;
		case 15: //0x00001111:
			break;
		default:
			break;
	}
}
*/

int MarchingSquares::get_vertex_info(float value, float contour)
{
	if(value >= contour)
		return 0x00000001;
	else
		return 0x00000000;
}



void MarchingSquares::DisplayLine(Line* lines, int num_lines)
{
/*
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();	
	glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();	
	glLoadIdentity();
*/	
	
	    	
	//glScalef(2, 2, 1);
    	//glTranslatef(-0.5, -0.5, 0);
//glTranslatef(0, 0.045, 0);    	
    	      	

	//display
	
	for (int k=0; k<num_lines;k++){
		if(lines[k].bExtra)
			glColor3f(1.0f,1.0f,1.0f);
		else
			glColor4f(0.0f,0.0f, 0.0f,0.3);
		glEnable(GL_BLEND);                // Enable Blending
        	// Set The Blend Mode       
        	glBlendFunc(GL_SRC_ALPHA ,GL_ONE_MINUS_SRC_ALPHA);
        	glLineWidth(3.0);            // Set The Line Width
        	
		glBegin(GL_LINES);
			glVertex2f(lines[k].x0*2.0-1.0, lines[k].y0*2.0-1.0-0.001);
			glVertex2f(lines[k].x1*2.0-1.0, lines[k].y1*2.0-1.0-0.001);
			
			//glVertex2f(lines[k].x0*1.5 - 0.75, lines[k].y0*1.5 - 0.75);
			//glVertex2f(lines[k].x1*1.5 - 0.75, lines[k].y1*1.5 - 0.75);
			//glVertex2f(lines[k].x0*2.0 - 1.0, lines[k].y0*2.03-0.94);
			//glVertex2f(lines[k].x1*2.0 - 1.0, lines[k].y1*2.03-0.94);
			
			//glVertex2f(lines[k].x0*1.5, lines[k].y0*1.5);
			//glVertex2f(lines[k].x1*1.5, lines[k].y1*1.5);
		glEnd();
		glDisable(GL_BLEND);

	}
	
	
	for (int k=0; k<num_lines;k++){
		if(lines[k].bExtra)
			glColor3f(0.0f,0.0f,0.0f);
		else
			glColor3f(1.0f,1.0f,1.0f);
		glEnable(GL_BLEND);                // Enable Blending
        	// Set The Blend Mode       
        	glBlendFunc(GL_SRC_ALPHA ,GL_ONE_MINUS_SRC_ALPHA);
        	glLineWidth(2.0);            // Set The Line Width
        	
		glBegin(GL_LINES);
			glVertex2f(lines[k].x0*2.0-1.0, lines[k].y0*2.0-1.0);
			glVertex2f(lines[k].x1*2.0-1.0, lines[k].y1*2.0-1.0);
			
			//glVertex2f(lines[k].x0*1.5 - 0.75, lines[k].y0*1.5 - 0.75);
			//glVertex2f(lines[k].x1*1.5 - 0.75, lines[k].y1*1.5 - 0.75);
			//glVertex2f(lines[k].x0*2.0 - 1.0, lines[k].y0*2.03-0.94);
			//glVertex2f(lines[k].x1*2.0 - 1.0, lines[k].y1*2.03-0.94);
			
			//glVertex2f(lines[k].x0*1.5, lines[k].y0*1.5);
			//glVertex2f(lines[k].x1*1.5, lines[k].y1*1.5);
		glEnd();
		glDisable(GL_BLEND);
	}
	
/*	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();	
		
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();	
*/
}

/*
//        v011______v111
//       / |       /|
//   v001 _____ v101|
//	| v010____|_v100
//	| /	  |/
//	v000_____v100
//
//
float tri_intrp(float* data, float** coord,
            int newx_ind, int newy_ind, int newz_ind,
            int sizex, int sizey, int sizez, int newsize)
{
    float V[2][2][2];
    int xi, yi, zi;
    xi = yi = zi = 0;
   
    int startx = (sizex-1) * newx_ind / (newsize-1);
    int starty = (sizey-1) * newy_ind / (newsize-1);
    int startz = (sizez-1) * newz_ind / (newsize-1);

    float s_x = (float)((sizex-1) * newx_ind) / (float)(newsize-1) - startx;
    float s_y = (float)((sizey-1) * newy_ind) / (float)(newsize-1) - starty;
    float s_z = (float)((sizez-1) * newz_ind) / (float)(newsize-1) - startz;

    for(int z=startz; z <= (startz+1); z++){
        yi = 0;
        for(int y=starty; y <= (starty+1); y++){
            xi = 0;
            for(int x=startx; x <= (startx+1); x++){
               
                if(x == sizex || y == sizey || z == sizez)
                    V[xi][yi][zi] = 0;
                else{
                    int v_id = x + y * sizex + z * sizex * sizey;
                    V[xi][yi][zi] = data[v_id];
                }
                //for this test only
                //V[xi][yi][zi] = data[x][y][z];
                xi++;
            }
            yi++;
        }
        zi++;
    }
       
    float* cx = coord[0];
    float* cy = coord[1];
    float* cz = coord[2];
    float diffx, diffy, diffz;
    if(startx+1 != sizex)
        diffx = cx[startx+1] - cx[startx];
    else diffx = 1;
   
    if(starty+1 != sizey)
        diffy = cy[starty+1] - cy[starty];
    else diffy = 1;
   
    if(startz+1 != sizez)
        diffz = cz[startz+1] - cz[startz];
    else diffz = 1;

    float value =    V[0][0][0] * (diffx - s_x) *(diffy - s_y) *(diffz - s_z) +
    V[1][0][0] * s_x *(diffy - s_y) *(diffz - s_z) +
    V[0][1][0] * (diffx - s_x) *s_y *(diffz - s_z) +
    V[0][0][1] * (diffx - s_x) *(diffy - s_y) *s_z +
    V[1][0][1] * s_x *(diffy - s_y) *s_z +
    V[0][1][1] * (diffx - s_x) *s_y *s_z +
    V[1][1][0] * s_x *s_y *(diffz - s_z) +
    V[1][1][1] * s_x *s_y *s_z;
   
    return value;
}
*/

/*
//
//	v3_____v2
//	|	|
//	|	|
//	v0_____v1
//
//
void MarchingSquares::compute(int sizex, int sizey, int sizez, float* data)
{
	char filename[1024];
    	char str[20];
	float contour = m_contour;

        //glPushMatrix();
        //glLineWidth(1.0);
        //glColor3f(1.0f,1.0f,1.0f);
	
	// going along the Z axis 
	list<float>::iterator v = m_contour_list.begin();
	int size = m_contour_list.size();
	int count = 0;
	do{

	{
	if(m_bupdate_all)
		contour = *v;
	else
		count = m_num_saved;
	sprintf(filename, "contour%d_%dzaxis.cfg", m_timestamp, count);
	ofstream outf;
	outf.open(filename,ios::binary);
    	assert(outf);
	//outf.write(reinterpret_cast<char *>(SLICE_Z_AXIS), sizeof(int));
	for (int z = 0; z < sizez; z++){
		sprintf(str, "Z_%d", z);
		outf.write(reinterpret_cast<char *>(str), sizeof(char)*20);
		for (int y = 0; y < sizey-1; y++) {
			for (int x = 0; x < sizex-1; x++) {
				int v0_id = x + y * sizex + z * sizex * sizey;
				int v1_id = x+1 + y * sizex + z * sizex * sizey;
				int v2_id = x+1 + (y+1) * sizex + z * sizex * sizey;
				int v3_id = x + (y+1) * sizex + z * sizex * sizey;
				
				int v0 = get_vertex_info(data[v0_id], contour);
				v0 = v0 << 0;
				int v1 = get_vertex_info(data[v1_id], contour);
				v1 = v1 << 1;
				int v2 = get_vertex_info(data[v2_id], contour);
				v2 = v2 << 2;
				int v3 = get_vertex_info(data[v3_id], contour);
				v3 = v3 << 3;
				int res = v0 | v1 | v2 | v3;
				
				Line lines[MAX_NUM_LINES];
				int num_lines = 0;
				DataRectangle dr(data[v0_id], data[v1_id], data[v2_id], data[v3_id]);
				
				
				float newx = m_pGrid[0][x];
				float newy = m_pGrid[1][y];
				float nextx = m_pGrid[0][x+1];
				float nexty = m_pGrid[1][y+1];

				getCaseGrid(res,newx,newy, nextx, nexty,  &dr, lines, &num_lines);
				outf.write(reinterpret_cast<char *>(&z), sizeof(int));
				outf.write(reinterpret_cast<char *>(&y), sizeof(int));
				outf.write(reinterpret_cast<char *>(&x), sizeof(int));
				
				outf.write(reinterpret_cast<char *>(&num_lines), sizeof(int));
				
				for(int n=0; n<num_lines; n++){
					outf.write(reinterpret_cast<char *>(&(lines[n].x0)), sizeof(float));
					outf.write(reinterpret_cast<char *>(&(lines[n].y0)), sizeof(float));
					outf.write(reinterpret_cast<char *>(&(lines[n].x1)), sizeof(float));
					outf.write(reinterpret_cast<char *>(&(lines[n].y1)), sizeof(float));
					outf.write(reinterpret_cast<char *>(&(lines[n].bExtra)), sizeof(int));
				}
				
				//DisplayLine(lines, num_lines);
			}
		}
	}
	outf.close();
	}

	{
	sprintf(filename, "contour%d_%dyaxis.cfg", m_timestamp, count);
	ofstream outf(filename);
    	assert(outf);
	// going along the Y axis 
	//outf.write(reinterpret_cast<char *>(SLICE_Y_AXIS), sizeof(int));
	for (int y = 0; y < sizey; y++){
		sprintf(str, "Y_%d", sizey - y);
		int tempy = y;
		outf.write(reinterpret_cast<char *>(str), sizeof(char)*20);
		for (int z = 0; z < sizez-1; z++)
			for (int x = 0; x < sizex-1; x++) {
				int v0_id = x + tempy * sizex + z * sizex * sizey;
				int v1_id = x+1 + tempy * sizex + z * sizex * sizey;
				int v2_id = x+1 + tempy * sizex + (z+1) * sizex * sizey;
				int v3_id = x + tempy * sizex + (z+1) * sizex * sizey;
				
				int v0 = get_vertex_info(data[v0_id], contour);
				v0 = v0 << 0;
				int v1 = get_vertex_info(data[v1_id], contour);
				v1 = v1 << 1;
				int v2 = get_vertex_info(data[v2_id], contour);
				v2 = v2 << 2;
				int v3 = get_vertex_info(data[v3_id], contour);
				v3 = v3 << 3;
				int res = v0 | v1 | v2 | v3;
				
				Line lines[MAX_NUM_LINES];
				int num_lines = 0;
				DataRectangle dr(data[v0_id], data[v1_id], data[v2_id], data[v3_id]);

				float newx = m_pGrid[0][x];
				float newz = m_pGrid[2][z];
				float nextx = m_pGrid[0][x+1];
				float nextz = m_pGrid[2][z+1];
					
				getCaseGrid(res,newx,newz, nextx, nextz,  &dr, lines, &num_lines);
				//getCase(res,x,z, sizex, sizez,  &dr, lines, &num_lines);
				outf.write(reinterpret_cast<char *>(&z), sizeof(int));
				outf.write(reinterpret_cast<char *>(&y), sizeof(int));
				outf.write(reinterpret_cast<char *>(&x), sizeof(int));
				outf.write(reinterpret_cast<char *>(&num_lines), sizeof(int));
				for(int n=0; n<num_lines; n++){
					outf.write(reinterpret_cast<char *>(&(lines[n].x0)), sizeof(float));
					outf.write(reinterpret_cast<char *>(&(lines[n].y0)), sizeof(float));
					outf.write(reinterpret_cast<char *>(&(lines[n].x1)), sizeof(float));
					outf.write(reinterpret_cast<char *>(&(lines[n].y1)), sizeof(float));
					outf.write(reinterpret_cast<char *>(&(lines[n].bExtra)), sizeof(int));
				}
				//DisplayLine(lines, num_lines);
			}
	}
	outf.close();
	}

	{

	sprintf(filename, "contour%d_%dxaxis.cfg", m_timestamp, count);
	ofstream outf(filename);
    	assert(outf);
	// going along the X axis 
	//outf.write(reinterpret_cast<char *>(SLICE_X_AXIS), sizeof(int));
	for (int x = 0; x < sizex; x++){
		sprintf(str, "X_%d", x);
		outf.write(reinterpret_cast<char *>(str), sizeof(char)*20);
		for (int z = 0; z < sizez-1; z++)
			for (int y = 0; y < sizey-1; y++){ 
				
				int v0_id = x + y * sizex + z * sizex * sizey;
				int v1_id = x + (y+1) * sizex + z * sizex * sizey;
				int v2_id = x + (y+1) * sizex + (z+1) * sizex * sizey;
				int v3_id = x + y * sizex + (z+1) * sizex * sizey;

				int v0 = get_vertex_info(data[v0_id], contour);
				v0 = v0 << 0;
				int v1 = get_vertex_info(data[v1_id], contour);
				v1 = v1 << 1;
				int v2 = get_vertex_info(data[v2_id], contour);
				v2 = v2 << 2;
				int v3 = get_vertex_info(data[v3_id], contour);
				v3 = v3 << 3;
				int res = v0 | v1 | v2 | v3;
				
				Line lines[MAX_NUM_LINES];
				int num_lines = 0;
				DataRectangle dr(data[v0_id], data[v1_id], data[v2_id], data[v3_id]);


				float newy = m_pGrid[1][y];
				float newz = m_pGrid[2][z];
				float nexty = m_pGrid[1][y+1];
				float nextz = m_pGrid[2][z+1];

				getCaseItrpl(res,newy,newz, nexty, nextz,  &dr, lines, &num_lines);
				//getCaseGrid(res,newy,newz, nexty, nextz,  &dr, lines, &num_lines);
				outf.write(reinterpret_cast<char *>(&z), sizeof(int));
				outf.write(reinterpret_cast<char *>(&y), sizeof(int));
				outf.write(reinterpret_cast<char *>(&x), sizeof(int));
				outf.write(reinterpret_cast<char *>(&num_lines), sizeof(int));
				for(int n=0; n<num_lines; n++){
					outf.write(reinterpret_cast<char *>(&(lines[n].x0)), sizeof(float));
					outf.write(reinterpret_cast<char *>(&(lines[n].y0)), sizeof(float));
					outf.write(reinterpret_cast<char *>(&(lines[n].x1)), sizeof(float));
					outf.write(reinterpret_cast<char *>(&(lines[n].y1)), sizeof(float));
					outf.write(reinterpret_cast<char *>(&(lines[n].bExtra)), sizeof(int));
				}
				//DisplayLine(lines, num_lines);
			}
	}
	outf.close();
	}
	++v;
	++count;
	}while(v != m_contour_list.end() && m_bupdate_all);
	//glPopMatrix();
	m_bupdate_all= false;
}
*/
/*
void MarchingSquares::load_file(int axis, int sliceid, int sizex, int sizey, int sizez) 
{
	char filename[1024];
    	int temp;
	char str[100];
	
	glPushMatrix();
        glLineWidth(3.0);
        glColor3f(1.0f,1.0f,1.0f);
	//glColor3f(0.0f,0.0f,0.0f);
    	if (axis == SLICE_Z_AXIS){
		for(int num=0; num<=m_num_saved; num++){
			int sizearr[3] = {sizex, sizey, sizez};
			sprintf(filename, "contour%d_%dzaxis.cfg", m_timestamp, num);
			ifstream inf;
			inf.open(filename, ios::in | ios::binary);
			inf.clear();
			inf.seekg(0,ios::beg);
    
    			if (!inf)
        			return;
			// going along the Z axis 
			sprintf(str, "Z_%d", sliceid); 
			//istream_iterator<char> iter(inf);
			
			bool bFound = false;
			for (int z = 0; z < sizez && !bFound; z++) {
				
				char str_out[20]="\0";
				int len = strlen(str);
				inf.read(str_out, sizeof(char)*20);
				if(strcmp(str_out, str)==0){
					bFound = true;
					cout << "found " << str_out << endl;
				}
				for (int y = 0; y < sizey-1; y++) {
				for (int x = 0; x < sizex-1; x++) {
					int xf = 0; 
					int yf = 0;
					int zf = 0;
					Line lines[MAX_NUM_LINES];
					int num_lines = 0;
					char temp[20] = "\0";
					inf.read(reinterpret_cast<char *>(&zf), sizeof(int));
					inf.read(reinterpret_cast<char *>(&yf), sizeof(int));
					inf.read(reinterpret_cast<char *>(&xf), sizeof(int));
					
					inf.read(reinterpret_cast<char *>(&num_lines), sizeof(int));
	
					
					for(int n=0; n<num_lines; n++){
						inf.read(reinterpret_cast<char *>(&(lines[n].x0)), sizeof(float));
						inf.read(reinterpret_cast<char *>(&(lines[n].y0)), sizeof(float));
						inf.read(reinterpret_cast<char *>(&(lines[n].x1)), sizeof(float));
						inf.read(reinterpret_cast<char *>(&(lines[n].y1)), sizeof(float));
						inf.read(reinterpret_cast<char *>(&(lines[n].bExtra)), sizeof(int));
					}
					if(bFound && num_lines > 0)
						DisplayLine(lines, num_lines);
				}
				}
			}
			inf.close();
		}
	}
	else if (axis == SLICE_Y_AXIS){
		for(int num=0; num<=m_num_saved; num++){
			int sizearr[3] = {sizex, sizey, sizez};
			sprintf(filename, "contour%d_%dyaxis.cfg", m_timestamp, num);
			ifstream inf(filename);
    			char str_out[100]="\0";

    			if (!inf)
        			return;
			if(sliceid > sizey /2 )
				sliceid = sizey - sliceid -2;
			else if(sliceid < sizey /2)
				sliceid = sizey - sliceid + 2; 
			sprintf(str, "Y_%d", sliceid); 
			
			// going along the Y axis 
			bool bFound = false;
			for (int y = 0; y < sizey && !bFound; y++) {
				
				char str_out[20]="\0";
				int len = strlen(str);
				inf.read(str_out, sizeof(char)*20);
				if(strcmp(str_out, str)==0){
					bFound = true;
					cout << "found " << str_out << endl;
				}
				for (int z = 0; z < sizez-1; z++){
					for (int x = 0; x < sizex-1; x++) {
						int xf = 0; 
						int yf = 0;
						int zf = 0;
						Line lines[MAX_NUM_LINES];
						int num_lines = 0;
						inf.read(reinterpret_cast<char *>(&zf), sizeof(int));
						inf.read(reinterpret_cast<char *>(&yf), sizeof(int));
						inf.read(reinterpret_cast<char *>(&xf), sizeof(int));
						inf.read(reinterpret_cast<char *>(&num_lines), sizeof(int));
						for(int n=0; n<num_lines; n++){
							inf.read(reinterpret_cast<char *>(&(lines[n].x0)), sizeof(float));
							inf.read(reinterpret_cast<char *>(&(lines[n].y0)), sizeof(float));
							inf.read(reinterpret_cast<char *>(&(lines[n].x1)), sizeof(float));
							inf.read(reinterpret_cast<char *>(&(lines[n].y1)), sizeof(float));
							inf.read(reinterpret_cast<char *>(&(lines[n].bExtra)), sizeof(int));
						}
						if(bFound && num_lines > 0)
							DisplayLine(lines, num_lines);
					}
				}
			}
			inf.close();
		}
	}
	else if (axis == SLICE_X_AXIS){
		for(int num=0; num<=m_num_saved; num++){
			int sizearr[3] = {sizex, sizey, sizez};
			sprintf(filename, "contour%d_%dxaxis.cfg", m_timestamp, num);
			ifstream inf(filename);
			char str_out[100]="\0";
			
			if (!inf)
        			return;
			sprintf(str, "X_%d", sliceid); 
			
			// going along the X axis 
			bool bFound = false;
			for (int x = 0; x < sizex && !bFound; x++) {
				
				char str_out[20]="\0";
				int len = strlen(str);
				inf.read(str_out, sizeof(char)*20);
				if(strcmp(str_out, str)==0){
					bFound = true;
					cout << "found " << str_out << endl;
				}
				for (int z = 0; z < sizez-1; z++){
					for (int y = 0; y < sizey-1; y++){ 
						int xf = 0; 
						int yf = 0;
						int zf = 0;
						Line lines[MAX_NUM_LINES];
						int num_lines = 0;
						inf.read(reinterpret_cast<char *>(&zf), sizeof(int));
						inf.read(reinterpret_cast<char *>(&yf), sizeof(int));
						inf.read(reinterpret_cast<char *>(&xf), sizeof(int));
						inf.read(reinterpret_cast<char *>(&num_lines), sizeof(int));
						for(int n=0; n<num_lines; n++){
							inf.read(reinterpret_cast<char *>(&(lines[n].x0)), sizeof(float));
							inf.read(reinterpret_cast<char *>(&(lines[n].y0)), sizeof(float));
							inf.read(reinterpret_cast<char *>(&(lines[n].x1)), sizeof(float));
							inf.read(reinterpret_cast<char *>(&(lines[n].y1)), sizeof(float));
							inf.read(reinterpret_cast<char *>(&(lines[n].bExtra)), sizeof(int));
						}
						if(bFound && num_lines > 0)
							DisplayLine(lines, num_lines);
					}
				}
			}
			inf.close();
		}
	}
	else{
	}
	glPopMatrix();
}
*/
float MarchingSquares::linear_intrp1D(	float val1, float val2, float alfa)
{
	return (val1 * alfa + val2 * (1 - alfa));
}

/*
void MarchingSquares::compute_for_slice(int sizex, int sizey, int sizez, float* data, int nSliceAxis, float slicepos)
{
	char filename[1024];
    	char str[20];
	float contour = m_contour;
	int count = 0;
	int sliceid;
	int v0_id[2], v1_id[2], v2_id[2], v3_id[2];
	float val0, val1, val2, val3; 

	glPushMatrix();
        glLineWidth(3.0);
        glColor3f(1.0f,1.0f,1.0f);

	if(nSliceAxis == SLICE_Z_AXIS){
		float real_sliceid = (float)((slicepos + 1) * sizez) / 2.0;
		sliceid = floor(real_sliceid);
		int z = sliceid;
		for (int y = 0; y < sizey-1; y++) {
			for (int x = 0; x < sizex-1; x++) {
				int v0_id = x + y * sizex + z * sizex * sizey;
				int v1_id = x+1 + y * sizex + z * sizex * sizey;
				int v2_id = x+1 + (y+1) * sizex + z * sizex * sizey;
				int v3_id = x + (y+1) * sizex + z * sizex * sizey;
				
				int v0 = get_vertex_info(data[v0_id], contour);
				v0 = v0 << 0;
				int v1 = get_vertex_info(data[v1_id], contour);
				v1 = v1 << 1;
				int v2 = get_vertex_info(data[v2_id], contour);
				v2 = v2 << 2;
				int v3 = get_vertex_info(data[v3_id], contour);
				v3 = v3 << 3;
				int res = v0 | v1 | v2 | v3;
				
				Line lines[MAX_NUM_LINES];
				int num_lines = 0;
				DataRectangle dr(data[v0_id], data[v1_id], data[v2_id], data[v3_id]);
				
				
				float newx = m_pGrid[0][x];
				float newy = m_pGrid[1][y];
				float nextx = m_pGrid[0][x+1];
				float nexty = m_pGrid[1][y+1];

				getCaseItrpl(res,newx,newy, nextx, nexty,  &dr, lines, &num_lines);	
				if(num_lines > 0)
					DisplayLine(lines, num_lines);		
			}
		}
	}
	else if(nSliceAxis == SLICE_Y_AXIS){
		float real_sliceid = (float)((slicepos + 1) * sizey) / 2.0;
		sliceid = floor(real_sliceid);

		if(sliceid > sizey /2 )
			sliceid = sizey - sliceid -2;
		else if(sliceid < sizey /2)
			sliceid = sizey - sliceid + 2; 
		
		int y = sliceid;
		int tempy = y;
		for (int z = 0; z < sizez-1; z++)
			for (int x = 0; x < sizex-1; x++) {
				int v0_id = x + tempy * sizex + z * sizex * sizey;
				int v1_id = x+1 + tempy * sizex + z * sizex * sizey;
				int v2_id = x+1 + tempy * sizex + (z+1) * sizex * sizey;
				int v3_id = x + tempy * sizex + (z+1) * sizex * sizey;
				
				int v0 = get_vertex_info(data[v0_id], contour);
				v0 = v0 << 0;
				int v1 = get_vertex_info(data[v1_id], contour);
				v1 = v1 << 1;
				int v2 = get_vertex_info(data[v2_id], contour);
				v2 = v2 << 2;
				int v3 = get_vertex_info(data[v3_id], contour);
				v3 = v3 << 3;
				int res = v0 | v1 | v2 | v3;
				
				Line lines[MAX_NUM_LINES];
				int num_lines = 0;
				DataRectangle dr(data[v0_id], data[v1_id], data[v2_id], data[v3_id]);

				
				float newx = m_pGrid[0][x];
				float newz = m_pGrid[2][z];
				float nextx = m_pGrid[0][x+1];
				float nextz = m_pGrid[2][z+1];
					
				getCaseItrpl(res,newx,newz, nextx, nextz,  &dr, lines, &num_lines);
				
				if(num_lines > 0)
					DisplayLine(lines, num_lines);
			}
	}
	else if(nSliceAxis == SLICE_X_AXIS)
	{
		float real_sliceid = (float)((slicepos + 1) * sizex) / 2.0;
		sliceid = floor(real_sliceid);
		//sprintf(filename, "contour%d_%dxaxis_slice%f.cfg", m_timestamp, count, slicepos);
		//ofstream outf(filename);
		//assert(outf);
	
	
		// going along the X axis 
		//outf.write(reinterpret_cast<char *>(SLICE_X_AXIS), sizeof(int));
		
		int x;
		if(slicepos > 0)
			x = sliceid - 1;
		else
			x = sliceid - 1;
			
		x = floor((slicepos + 1.0) * 0.5 * (sizex - 1));
		
		for (int z = 0; z < sizez-1; z++){
			for (int y = 0; y < sizey-1; y++){ 
				
				//if(sliceid == real_sliceid){
				v0_id[0] = x + y * sizex + z * sizex * sizey;
				v1_id[0] = x + (y+1) * sizex + z * sizex * sizey;
				v2_id[0] = x + (y+1) * sizex + (z+1) * sizex * sizey;
				v3_id[0] = x + y * sizex + (z+1) * sizex * sizey;
				
				val0 = data[v0_id[0]];
				val1 = data[v1_id[0]];
				val2 = data[v2_id[0]];
				val3 = data[v3_id[0]];
				
				int v0 = get_vertex_info(val0, contour);
				v0 = v0 << 0;
				int v1 = get_vertex_info(val1, contour);
				v1 = v1 << 1;
				int v2 = get_vertex_info(val2, contour);
				v2 = v2 << 2;
				int v3 = get_vertex_info(val3, contour);
				v3 = v3 << 3;
				int res = v0 | v1 | v2 | v3;
				
				Line lines[MAX_NUM_LINES];
				int num_lines = 0;
				DataRectangle dr(val0, val1, val2, val3);
	
	
				float newy = m_pGrid[1][y];
				float newz = m_pGrid[2][z] ;
				float nexty = m_pGrid[1][y+1];
				float nextz = m_pGrid[2][z+1] ;
	
				getCaseItrpl(res,newy,newz, nexty, nextz,  &dr, lines, &num_lines);
				//getCaseGrid(res,newy,newz, nexty, nextz,  &dr, lines, &num_lines);
				
				if(num_lines > 0)
					DisplayLine(lines, num_lines);
			}
		}
		//outf.close();
	}
	glPopMatrix();
}
*/

void MarchingSquares::compute_for_texture(float* buffer, int height, int width)
{
	float val0, val1, val2, val3; 
	float contour = m_contour;
	int count = 0;



        /*
        glColor3f(1.0, 0.0, 0.0);
	float x0,y0,x1,y1;
	glBegin(GL_LINES);
		x0 = 1.0;
		y0 = 0.0;
		glVertex2f(x0, y0);
		x1 = 1.0;
		y1 = 0.75;
		glVertex2f(x1, y1);	
	glEnd();
	*/

        list<float>::iterator v = m_contour_list.begin();
	int size = m_contour_list.size();
	float arr_contours = 0.5;
	do{
		if(count > 0){
			contour = *v;
			++v;
		}
		++count;
		
	glPushMatrix();
    	//glLoadIdentity();    
    	//glTranslatef(-0.75,-0.75,0);
    
    	glColor3f(1.0f,1.0f,1.0f);
    	
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);    // Use The Good Calculations
        glEnable(GL_LINE_SMOOTH);            // Enable Anti-Aliasing
        glLineWidth(3.0);
        
	for (int y = 0; y < height-1; y++){
		for (int x = 0; x < width-1; x++){ 
			val0 = buffer[x + y*width];
			val1 = buffer[(x+1) + y*width];
			val2 = buffer[(x+1) + (y+1)*width];
			val3 = buffer[x + (y+1)*width];
			
			int v0 = get_vertex_info(val0, contour);
			v0 = v0 << 0;
			int v1 = get_vertex_info(val1, contour);
			v1 = v1 << 1;
			int v2 = get_vertex_info(val2, contour);
			v2 = v2 << 2;
			int v3 = get_vertex_info(val3, contour);
			v3 = v3 << 3;
			int res = v0 | v1 | v2 | v3;
			
			Line lines[MAX_NUM_LINES];
			int num_lines = 0;
			DataRectangle dr(val0, val1, val2, val3);


			float newx = (float)x/(float)(width-1);
			float newy = (float)y/(float)(height-1);
			float nextx = (float)(x+1)/(float)(width-1);
			float nexty = (float)(y+1)/(float)(height-1);

			getCaseItrpl(contour, res,newx,newy, nextx, nexty,  &dr, lines, &num_lines);
			if(num_lines > 0)
				DisplayLine(lines, num_lines);
		}
	}
	glDisable(GL_LINE_SMOOTH);
	glPopMatrix();
	
	}while(v != m_contour_list.end());
		
	
	/*
	glScalef(2, 2, 1);
    	glTranslatef(-0.5, -0.5, 0);
    	
    	glColor3f(1,0,0);
    	glBegin(GL_QUADS);
    		glVertex2f(0.9, 0.9);
		glVertex2f(1, 0.9);
		glVertex2f(1, 1);
		glVertex2f(0.9, 1);
    	glEnd();
    	
    	glBegin(GL_QUADS);
    		glVertex2f(0.8, 0.8);
		glVertex2f(0.9, 0.8);
		glVertex2f(0.9, 0.9);
		glVertex2f(0.8, 0.9);
    	glEnd();
    	*/
    	
//glTranslatef(0, 0.045, 0);    	
	
	
}


/*
void MarchingSquares::load_for_slice(int axis, float slicepos, int sizex, int sizey, int sizez) 
{
	char filename[1024];
    	int temp;
	char str[100];
	int sliceid;
	
	glPushMatrix();
        glLineWidth(3.0);
        glColor3f(1.0f,1.0f,1.0f);
	if (axis == SLICE_X_AXIS){
		//for(int num=0; num<=m_num_saved; num++){
		int num = 0;
		int sizearr[3] = {sizex, sizey, sizez};
		float real_sliceid = (float)((slicepos + 1) * sizex) / 2.0;
		sliceid = floor(real_sliceid);
		sprintf(filename, "contour%d_%dxaxis_slice%f.cfg", m_timestamp, num, slicepos);

		ifstream inf(filename);
		char str_out[100]="\0";
		
		if (!inf)
			return;
		int x = sliceid;
		
		
		for (int z = 0; z < sizez-1; z++){
			for (int y = 0; y < sizey-1; y++){ 
				int xf = 0; 
				int yf = 0;
				int zf = 0;
				Line lines[MAX_NUM_LINES];
				int num_lines = 0;
				inf.read(reinterpret_cast<char *>(&zf), sizeof(int));
				inf.read(reinterpret_cast<char *>(&yf), sizeof(int));
				inf.read(reinterpret_cast<char *>(&xf), sizeof(int));
				inf.read(reinterpret_cast<char *>(&num_lines), sizeof(int));
				for(int n=0; n<num_lines; n++){
					inf.read(reinterpret_cast<char *>(&(lines[n].x0)), sizeof(float));
					inf.read(reinterpret_cast<char *>(&(lines[n].y0)), sizeof(float));
					inf.read(reinterpret_cast<char *>(&(lines[n].x1)), sizeof(float));
					inf.read(reinterpret_cast<char *>(&(lines[n].y1)), sizeof(float));
					inf.read(reinterpret_cast<char *>(&(lines[n].bExtra)), sizeof(int));
				}
				if(num_lines > 0)
					DisplayLine(lines, num_lines);
			}
		}
		inf.close();
		//}
	}
	else{}
	glPopMatrix();
}
*/

//        v011______v111
//       / |       /|
//   v001 _____ v101|
//	| v010____|_v110
//	| /	  |/
//	v000_____v100
//
//
/*
float MarchingSquares::tri_intrp(float* data, float** coord,
            int newx_ind, int newy_ind, int newz_ind,
            int sizex, int sizey, int sizez, int newsizex, int newsizey, int newsizez)
{
    float V[2][2][2];
    int xi, yi, zi;
    xi = yi = zi = 0;
   
    int startx = (sizex-1) * newx_ind / (newsizex-1);
    int starty = (sizey-1) * newy_ind / (newsizey-1);
    int startz = (sizez-1) * newz_ind / (newsizez-1);

    float s_x = (float)((sizex-1) * newx_ind) / (float)(newsizex-1) - startx;
    float s_y = (float)((sizey-1) * newy_ind) / (float)(newsizey-1) - starty;
    float s_z = (float)((sizez-1) * newz_ind) / (float)(newsizez-1) - startz;

    for(int z=startz; z <= (startz+1); z++){
        yi = 0;
        for(int y=starty; y <= (starty+1); y++){
            xi = 0;
            for(int x=startx; x <= (startx+1); x++){
               
                if(x == sizex || y == sizey || z == sizez)
                    V[xi][yi][zi] = 0;
                else{
                    int v_id = x + y * sizex + z * sizex * sizey;
                    V[xi][yi][zi] = data[v_id];
                }
                //for this test only
                //V[xi][yi][zi] = data[x][y][z];
                xi++;
            }
            yi++;
        }
        zi++;
    }
       
    float* cx = coord[0];
    float* cy = coord[1];
    float* cz = coord[2];
    float diffx, diffy, diffz;
    if(startx+1 != sizex)
        diffx = cx[startx+1] - cx[startx];
    else diffx = 1;
   
    if(starty+1 != sizey)
        diffy = cy[starty+1] - cy[starty];
    else diffy = 1;
   
    if(startz+1 != sizez)
        diffz = cz[startz+1] - cz[startz];
    else diffz = 1;

    float value =    V[0][0][0] * (diffx - s_x) *(diffy - s_y) *(diffz - s_z) +
    V[1][0][0] * s_x *(diffy - s_y) *(diffz - s_z) +
    V[0][1][0] * (diffx - s_x) *s_y *(diffz - s_z) +
    V[0][0][1] * (diffx - s_x) *(diffy - s_y) *s_z +
    V[1][0][1] * s_x *(diffy - s_y) *s_z +
    V[0][1][1] * (diffx - s_x) *s_y *s_z +
    V[1][1][0] * s_x *s_y *(diffz - s_z) +
    V[1][1][1] * s_x *s_y *s_z;
   
    return value;
}
*/
