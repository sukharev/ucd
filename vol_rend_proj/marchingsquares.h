#ifndef MARCHINGSQUARES_H
#define MARCHINGSQUARES_H

#include <vector>
#include <list>
using namespace std;

//
//	v3_____v2
//	|	|
//	|	|
//	v0_____v1
//
//
struct DataRectangle
{
	float datav0;
	float datav1;
	float datav2;
	float datav3;
	DataRectangle(float nv0, float nv1, float nv2, float nv3)
		 : datav0(nv0), datav1(nv1), datav2(nv2), datav3(nv3) {}
};


struct Line
{
	// start
	float x0;
	float y0;

	// finish
	float x1;
	float y1;

	// debugging flag
	bool bExtra;
	Line()
	{
		x0 = 0.0;
		x1 = 0.0;
		y0 = 0.0;
		y1 = 0.0;
		bExtra = false;
	}
};

class MarchingSquares
{
public:
	MarchingSquares(float contour_value, float** grid, int num_saved, int timestamp);
	~MarchingSquares();
public:
	void compute(int sizex, int sizey, int sizez, float* data);
	void update(float contour_value, int num_saved, int timestamp);
	void load_file(int axis, int sliceid, int sizex, int sizey, int sizez);

	void compute_for_slice(int sizex, int sizey, int sizez, float* data, int nSliceAxis, float slicepos);
	void compute_for_texture(float* buffer, int height, int width);
	//void load_for_slice(int axis, float slicepos, int sizex, int sizey, int sizez);
	float linear_intrp1D(	float val1, float val2, float alfa);
private:
	void getCaseItrpl(	float contour, int choice, float cellstartx, float cellstarty, 
				float nextx, float nexty, DataRectangle* dr,
				/* out */ Line* lines, /* out */int* num_lines);
	void getCaseGrid(	int choice, float cellstartx, float cellstarty, 
				float nextx, float nexty, DataRectangle* dr,
				/* out */ Line* lines, /* out */int* num_lines);
	int get_vertex_info(float value, float contour);

	void DisplayLine(Line* lines, int num_lines);
	float tri_intrp(float* data, float** coord,
            int newx_ind, int newy_ind, int newz_ind,
            int sizex, int sizey, int sizez, int newsize);


	void save_file();

	float* m_data;
	float m_contour;
	int m_axis;
	int m_timestamp;

	float** m_pGrid;
	int m_num_saved;
	bool m_bupdate_all;
	list<float> m_contour_list;
};

#endif //MARCHINGSQUARES_H