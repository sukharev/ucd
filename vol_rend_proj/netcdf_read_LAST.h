
#ifndef NETCDF_READ
#define NETCDF_READ

//#include <stdio.h>
#include <string>
#include <netcdf.h>
#include <fstream>
#include <iostream>
#include <assert.h>
#include <vector>
#include <algorithm>
#include <map>

//#ifndef MAX_PATH
#define MAX_PATH1    512
//#endif 
#define MAX_VARS	20
#define SEARCH_DIM 	4  // we are olny interested in data with SEARCH_DIM number of dimensions	

//#define MAX_VAR		5
//#define SAMPLE	20
using namespace std;

struct NetCFD_dim
{
	char dim_name[NC_MAX_NAME];
	int length;
	int query_start;
	int query_count;
};

struct NetCFD_var
{
	NetCFD_var(){}
	~NetCFD_var(){}

	char var_name[NC_MAX_NAME];
	nc_type rh_type;                  /* variable type */
	int rh_ndims;                      /* number of dims */
	int rh_dimids[NC_MAX_VAR_DIMS];    /* dimension ids */
	NetCFD_dim * dim[NC_MAX_VAR_DIMS]; 
	int rh_natts;                      /* number of attributes */
	int temp_varid; 
	size_t* start;
    size_t* count;
	int total_size;
	bool time_dep;

	//order: only used if var is a dim, default value is -1 for all other vars
	int order;
	//axis only used for 1-dim vardims, default '\0'
	char axis;
};

#define ANY_DIM		0
struct Filter
{
	Filter()
	{
		m_exclude_str[0] = '\0';
		m_include_str[0] = '\0';
		m_dim = ANY_DIM;
	}

	int m_dim;
	char m_exclude_str[MAX_PATH1];
	char m_include_str[MAX_PATH1];
};

class NetCDF
{
public:
	NetCDF(char* filename);
	~NetCDF();
	int load_header();
	int read_var(NetCFD_var *var);

	// returns number of variables and array of variable names
	char** get_varnames(/*out*/ int *num, int dim = SEARCH_DIM);
	int get_num_timesteps(char* name);
	NetCFD_var* get_varinfo(const char* name, Filter* fltr = NULL);
	NetCFD_var* get_varinfo_int(int index, Filter* fltr = NULL);
	
	// caller needs to pass fully allocated buffer to hold the data

	//wrapper on top next get_vardata_impl

	int get_vardata(char* index, 
						int time_in, 
						int time_out, 
						int start_var, 
						int fraction,
						/* out */char** data,
						bool bFullSize = false);
	int get_vardata_int(int index, 
						int time_in, 
						int time_out, 
						int start_var, 
						int fraction, 
						/* out */char** data,
						bool bFullSize = false);



	// caller needs to pass fully allocated buffer to hold the data

	int get_vardata_impl(NetCFD_var *var, 
		int time_in, 
		int time_out, 
		int start_var, 
		int fraction,
		/* out */char** data,
		bool bFullSize = false);

	int get_varindices();
	int get_var_size(int index);
	int get_var_size(char* name);

	bool getMinMax(char* varname, float* min, float* max);
	int get_num_vars() 
	{ 
		return m_nvars; 
	}
	// return the map from index of 
	//map<int,int> get_ind_map(int dim);

public:
	int m_ncid;
	int m_ndims;							//number of all dimesions
	int m_nvars;
	char* m_dim_name_arr[NC_MAX_VAR_DIMS];				//dimensions length
	size_t m_dim_length_arr[NC_MAX_VAR_DIMS];			//dimensions name
	char m_filename[MAX_PATH1];			//netCDF filename (ext .nc)
	NetCFD_var* m_var_arr[MAX_VARS];
	map<string, int> m_Nd_var;
	int m_Nd_var_num;
	bool m_init;
};


//TODO: pay attention to lifespan of this class
//pointer to instance of class NetCDF should NOT be deallocated while 
//the instance of this class is still around
//implement mechanism to enforce this or redesign the whole structure.
class NetCDFUtils
{
public:
	NetCDFUtils(NetCDF* netcdf, Filter* fltr = NULL) { 
		m_fltr = fltr;
		m_netcdf = netcdf; 
		m_pdata_arr = NULL;
		
		m_lenX = 1;
		m_lenY = 1;
		names = NULL;
		dim_min = NULL;
		dim_max = NULL;
		dim_num = NULL;
		Init();
	}

	~NetCDFUtils() {
		Clean();
		if(m_varlist)
			delete m_varlist;
	}
	
	void Clean(){
		
		if(m_pdata_arr){
			for(int i=0; i < m_num; i++){
				delete [] m_pdata_arr[i];
			}
			delete [] m_pdata_arr;
		}
		if(names)
			delete [] names;
		if(dim_min)
			delete [] dim_min;
		if(dim_max)
			delete [] dim_max;
		if(m_size)
			delete [] m_size;
		if(dim_num)
			delete [] dim_num;

	}

	void Init();
public:
	int GetCoordPosition(int item, int index);
	int get_num_vars() { return m_num;}

	float** ReadVarData(int time_in, int count_in, bool bnew, bool bQuantization=true);
	void SampleData(float* data, float* sample_data, int size, int fraction, char* varname, bool bDim);
	//float UpdateSampleData(float* sample_data, int size, char* varname, float min, float max);

	// new_start in %
	// new_count in %
	bool ChangeDataSize(int new_start, int new_count, char axis, int* out_count = NULL);
	int GetDimDataIndex(int index, NetCFD_var* var, char* dimname);
	
	//float** GetSubset(	int x_start, int y_start, int z_start,
	//		     	int x_end, int y_end, int z_end,
	//		     	int time_in, int time_count);
	
	//void SaveSubset(		int x_start, int x_end, 
	//						int y_start, int y_end, 
	//						int z_start, int z_end,
	//						int time_in, int time_count);
	void SaveSubset(int x_start, int x_end, int y_start,
			     int y_end, int z_start, int z_end,
				 char* varname_in, 
			     int time_in, int time_count);
	
	void Save_volumeTACs(	int x_start, int x_end, 
							int y_start, int y_end, 
							int z_start, int z_end,
							char* varname_in, 
							int time_in, int time_count,
							int xblocksize, int yblocksize, int zblocksize,
							char* filename = NULL);
	
	void Cluster_volumeTACs(int x_start, int x_end, 
							int y_start, int y_end, 
							int z_start, int z_end,
							char* varname_in, 
							int time_in, int time_count, 
							int xblocksize, int yblocksize, int zblocksize);
	
	void SavePearson(		char* varname1, char* varname2, 
							bool const cross,
							char const outfnhead[], 
							int time_in, int time_out, //int const start, int const end,
							bool const diag,
							int const sidx, int const eidx,
							int const sidy, int const eidy,
							int const sidz, int const eidz, 
							int &outid, FILE *debug
							);

	// careful with this function, always check that the buffer passed for argument dataout is allocated and is large enough
	void Get_subsetTACs(int x_start, int x_end, 
							int y_start, int y_end, 
							int z_start, int z_end,
							char* varname_in, 
							int time_in, int time_count, 
							int xblocksize, int yblocksize, int zblocksize,
							/* out */ float** dataout);

public:

	NetCDF* m_netcdf;
	float** m_pdata_arr;
	float* dim_min;
	float* dim_max;
	int* dim_num;
	char** names;
	int m_num;
	int* m_size;
	NetCFD_var** m_varlist;
	Filter* m_fltr;
	//map<int, int> m_map;
	map<string, int> m_mapvarname;

	int m_lenX;
	int m_lenY;
};

/*
...reading netcdf data arrays...

for (int z = 0; z < m_vSize[2] / 2; z++)
    for (int y = 0; y < m_vSize[1]; y++) 
        for (int x = 0; x < m_vSize[0]; x++) {
             int org_id = x + y * m_vSize[0] + z * m_vSize[0] * m_vSize[1];
		}
*/
#endif //NETCDF_READ
