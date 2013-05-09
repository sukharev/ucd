

#include "netcdf_read.h"
#include <time.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include <sys/types.h>
#include <sys/timeb.h>



// This is the name of the data file we will read.
#define FILE_NAME "atmos_daily.tile3.nc"

// We are reading 4D data, a NZFULL x NLAT x NLON grid, with #time
//  timesteps of data.
#define NDIMS  4
#define NLAT    2000
#define NLON    2000
#define NZFULL  26

// Names of things.
//#define TEMP_NAME "temp"
#define TEMP_NAME "vort"

// Handle errors by printing an error message and exiting with a
// non-zero status.
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); return 2;}

float FindMedian(float* data, int len, int pos);
float FindAvg(float* data, int len);
float FindVariance(float* data, int len);

NetCDF::NetCDF(char *filename)
{
	m_init = false;
	m_Nd_var_num = 0;
	strcpy(m_filename, filename);

	load_header();

	m_tacfnname = "";
	m_tacfn = NULL;
	srand((unsigned)time( NULL ));
}

NetCDF::~NetCDF()
{   

	for(int i=0; i < m_nvars; i++) {
		if (m_var_arr[i])
			delete m_var_arr[i];
	}

	if (m_init)
		// Close the file.
		nc_close(m_ncid);

}

char** NetCDF::get_varnames(int *num, int dim)
{
	char** names = NULL;
	*num = 0;
	int j = 0;
	if( m_init) {
		names = new char*[m_nvars];
		for(int i=0; i < m_nvars; i++) {
			if (m_var_arr[i]->rh_ndims == dim) {
				names[j] = new char[NC_MAX_NAME];
				strcpy(names[j], m_var_arr[i]->var_name);
				j++;
			}
		}
		*num = j;
	}
	return names;
}

// get variable names of specified dimensions (hardcoded 2D)
/*
int NetCDF::get_varindices(int dim)

{
int j = 0;
if( m_init) {
for(int i=0; i < m_nvars; i++) {
if (m_var_arr[i]->rh_ndims == dim)
j++;
}
}

return j;
}
*/

int NetCDF::load_header()
{
	int status, ngatts, unlimdimid;



	status = nc_open(m_filename, NC_NOWRITE, &m_ncid);
	if (status != NC_NOERR) ERR(status);

	//find out what is in it
	status = nc_inq(m_ncid, &m_ndims, &m_nvars, &ngatts, &unlimdimid);
	if (status != NC_NOERR) ERR(status);

	//char out_filename[MAX_PATH];
	char* var_name = new char[NC_MAX_NAME];


	for(int i=0; i < m_ndims; i++) {
		char* dim_name = new char[NC_MAX_NAME];
		size_t lengthp;
		// get dimension names, lengths
		status = nc_inq_dim(m_ncid, i, dim_name, &lengthp);
		if (status != NC_NOERR) ERR(status);
		m_dim_name_arr[i] = dim_name;
		m_dim_length_arr[i] = lengthp;
	}   

	//for(int i=0; i < m_ndims; i++) {
	//    cout << "names: " << m_dim_name_arr[i] << endl;
	//}

	for(int i=0; i < m_nvars; i++) {
		//nc_inq_var         
		int  rh_id;                        /* variable ID */
		m_var_arr[i] = new NetCFD_var();

		m_var_arr[i]->order = -1;
		m_var_arr[i]->axis = '\0';

		//get variable names, types, shapes
		status = nc_inq_var (m_ncid, i, 
			m_var_arr[i]->var_name, 
			&(m_var_arr[i]->rh_type), 
			&(m_var_arr[i]->rh_ndims), 
			m_var_arr[i]->rh_dimids,
			&(m_var_arr[i]->rh_natts));
		if (status != NC_NOERR) ERR(status);


		// TODO: this code needs to be places into a separate function
		// it can be used as a filter later on. 
		m_Nd_var[m_var_arr[i]->var_name] = i;
		m_Nd_var_num++;


		if ((status = nc_inq_varid(m_ncid, m_var_arr[i]->var_name, &(m_var_arr[i]->temp_varid))))
			ERR(status);	

		m_var_arr[i]->total_size = 1;
		m_var_arr[i]->time_dep = false;
		m_var_arr[i]->count = new size_t[m_var_arr[i]->rh_ndims];
		m_var_arr[i]->start = new size_t[m_var_arr[i]->rh_ndims];
		for (int j=0; j < m_var_arr[i]->rh_ndims; j++) {
			m_var_arr[i]->dim[j] = new NetCFD_dim;
			m_var_arr[i]->dim[j]->length = m_dim_length_arr[m_var_arr[i]->rh_dimids[j]];
			strcpy(m_var_arr[i]->dim[j]->dim_name, m_dim_name_arr[m_var_arr[i]->rh_dimids[j]]);
			if(strcmp(m_var_arr[i]->dim[j]->dim_name,"time")!=0)
				m_var_arr[i]->total_size = m_var_arr[i]->total_size *  m_var_arr[i]->dim[j]->length;
			else
				m_var_arr[i]->time_dep = true;
			m_var_arr[i]->dim[j]->query_start = 0;
			m_var_arr[i]->dim[j]->query_count = m_var_arr[i]->dim[j]->length;
		}
		if(m_var_arr[i]->total_size == 1)
			m_var_arr[i]->total_size = 0;
		//status = read_var(m_var_arr[i]);


		//process some attributes
		/* find out how much space is needed for attribute values */
		char* axis;
		size_t  vr_len;
		status = nc_inq_attlen (m_ncid, m_var_arr[i]->temp_varid, "cartesian_axis", &vr_len);
		if (status == NC_NOERR) {

			/* allocate required space before retrieving values */
			axis = new char[vr_len + 1];  /* + 1 for trailing null */

			/* get attribute values */
			status = nc_get_att_text(m_ncid, m_var_arr[i]->temp_varid, "cartesian_axis", axis);
			if (status != NC_NOERR) 
				ERR(status);
			axis[vr_len] = '\0';       /* null terminate */
			if (strcmp(axis, "X")==0){
				m_var_arr[i]->axis = 'X';
			}
			else if (strcmp(axis, "Y")==0){
				m_var_arr[i]->axis = 'Y';
			}
			else if (strcmp(axis, "Z")==0){
				m_var_arr[i]->axis = 'Z';
			}
			delete axis;
		}
	}


	m_init = true;

	//int temp = m_Nd_var["salt"];
	//int temp1 = m_Nd_var["xt_ocean"]; 

	return status;
}


int NetCDF::get_var_size(char* name)
{
	size_t current_size = 1;
	int i = m_Nd_var[name];
	for (int j=0; j < m_var_arr[i]->rh_ndims; j++) {
		//remember count[0] should be 1 ( for 1 time step)
		if (strcmp(m_dim_name_arr[m_var_arr[i]->rh_dimids[j]], "time")!=0)
			current_size = current_size * m_var_arr[i]->dim[j]->query_count;
	}
	return current_size;
}

NetCFD_var* NetCDF::get_varinfo(const char* name, Filter* fltr)
{
	int temp = m_Nd_var[name];
	return get_varinfo_int(temp, fltr);
}	


NetCFD_var* NetCDF::get_varinfo_int(int index, Filter* fltr)
{
	NetCFD_var* var = m_var_arr[index];
	if(fltr){
		if(strcmp(fltr->m_include_str, "\0") != 0)
			if(strcmp(fltr->m_include_str, var->var_name) != 0)
				return NULL;
		if(strcmp(fltr->m_exclude_str, "\0") != 0)
			if(strstr(fltr->m_exclude_str, var->var_name) == 0)
				return NULL;
		if(fltr->m_dim != var->rh_ndims && fltr->m_dim != ANY_DIM)
			return NULL;
	}
	return var;
}	

int NetCDF::get_num_timesteps(char* name)
{
	NetCFD_var *var =  m_var_arr[m_Nd_var[name]];
	int rh_ndims = var->rh_ndims;
	int* rh_dimids = var->rh_dimids;
	for (int j=0; j < rh_ndims; j++) {
		//remember count[0] should be 1 ( for 1 time step)
		if (strcmp(m_dim_name_arr[rh_dimids[j]], "time")==0){
			return m_dim_length_arr[rh_dimids[j]];
		}
	}
}

// caller needs to pass fully allocated buffer to hold the data

int NetCDF::get_vardata(char* index, 
						int time_in, 
						int time_out, 
						int start_var, 
						int fraction,
						/* out */char** data,
						bool bFullSize)

{

	return get_vardata_impl(get_varinfo(index), 
		time_in, 
		time_out, 
		start_var, 
		fraction, 
		data,
		bFullSize);

}

int NetCDF::get_vardata_int(int index, 
							int time_in, 
							int time_out, 
							int start_var, 
							int fraction,
							/* out */char** data,
							bool bFullSize)

{

	return get_vardata_impl(get_varinfo_int(index),
		time_in, 
		time_out, 
		start_var, 
		fraction,
		data,
		bFullSize);

}

bool NetCDF::resettachandle(const char* filename, const char* varname, int tdim)
{
	char infn[256];
	sprintf(infn, "%s_%s_tac_%d.dat", filename, varname, tdim);
	string newname = infn;
	if(newname != m_tacfnname){
		if(m_tacfn)
			fclose(m_tacfn);
		m_tacfn = NULL;
		m_tacfnname = newname;	
		if((m_tacfn = fopen(infn, "rb")) == NULL)
			return false;
	}
	return true;
}

int NetCDF::get_totalsize(const char* var_name, int* datasize)
{
	NetCFD_var* vinfo = get_varinfo(var_name);
	assert(vinfo);

	int t = 0, z =0, y = 0, x = 0;
	if (vinfo->rh_ndims == 4)
	{
		t = vinfo->dim[0]->length;
		z = vinfo->dim[1]->length;
		y = vinfo->dim[2]->length;
		x = vinfo->dim[3]->length;
	}
	datasize[0] = x;
	datasize[1] = y;
	datasize[2] = z;
	return x * y * z;
}

//*************************************************************************
//*************************************************************************
bool NetCDF::readtac(const char* filename, const char* varname, float* points, 
					 int start, int end, /* spatial indices */
					 int total_timesteps, /* total num of timesteps in TAC file */
					 int timesteps /* timesteps needed */)
{
	if(!points || (total_timesteps < timesteps))
		return false;

	//in the future extract total number of timesteps from the header file
	FILE* in;
/*
	char infn[256];
	sprintf(infn, "%s_%s_tac.dat", filename, varname);
	if((in = fopen(infn, "rb")) == NULL)
*/
	if(!m_tacfn)
		return false;
	
	float* tempptr = points;
	
	int result = 0; 
	int current = start;
	bool bRet = false;
	while(current <= end){
		result = fseek(m_tacfn, total_timesteps*current*sizeof(float), SEEK_SET);
		if(!result){
			fread(tempptr, sizeof(float), timesteps, m_tacfn);
			bRet = true;
		}
		else{
			bRet = false;
			break;
		}
		tempptr += timesteps;
		current++;
	}

	//fclose(in);
	return bRet;
}
/*
bool get_tac(char const fnhead[], int tdim, int index, float* & tac)
{
	bool bRet = false;
	if(!tac){
		printf("get_tac: output array is not allocated!\n");
		return false;
	}

	FILE* in;
	char infn[256];
	sprintf(infn, "%s_out.dat", fnhead);
	 
	if ((in = fopen(infn, "rb")) == NULL) {
		printf(" can't open file %s for reading!\n", infn); 
		return false;
	}
	int result = fseek( in, tdim*index*sizeof(float), SEEK_SET);
	if(!result){
		fread(tac, sizeof(float), tdim, in);
		bRet = true;
	}
	fclose(in);
	return bRet;
}
*/

//*************************************************************************
// savetimestep() saves contents of 1 timestep in a binary file
// in the same format as it was extracted from NetCDF file and
// stored in the same directory as the original NC file.
//*************************************************************************
bool NetCDF::savetimestep(const char* filename, const char* varname, int timestep, float* pdata, int sizeofvolume)
{
	if(!pdata || sizeofvolume <= 0)
		return false;
	FILE* out;
	char outfn[256];
	
	sprintf(outfn, "%s_%s_%04d.dat", filename, varname, timestep);
	if ((out = fopen(outfn, "wb")) == NULL) {
		printf(" can't open file %s for reading!\n", outfn); 
		return false;
	}
	fwrite(pdata, sizeof(float), sizeofvolume, out);
	fclose(out);
	return true;
}

//*************************************************************************
//*************************************************************************
bool NetCDF::checktacfile(const char* filename, const char* varname, int tdim)
{
	FILE* in;
	char infn[256];
	sprintf(infn, "%s_%s_tac_%d.dat", filename, varname, tdim);
	if((in = fopen(infn, "rb")) == NULL)
		return false;
	fclose(in);
	return true;
}

//*************************************************************************
// createtacfile() creates TAC file in the following binary format
// X(0,0,0)_t1 X(0,0,0)_t2 X(0,0,0)_t3 ...
// X(0,0,1)_t1 X(0,0,1)_t2 X(0,0,1)_t3 ...
// ...
// so that each TAC can be easily extracted in a block read.
// end >= start. All timesteps between and including start and 
// end will be written into TAC file.
//*************************************************************************
bool NetCDF::createtacfile(const char* filename, const char* varname, int start, int end, int sizeofvolume)
{
	FILE *out;
	char infn[256], outfn[256];
	int tdim = end - start + 1;
	float data;
	float *ptdata = &data;
	int result = 0;
	float *A = new float[tdim];

	FILE** in = new FILE*[tdim];
	for (int i=start; i<=end; i++){
		sprintf(infn, "%s_%s_%04d.dat", filename, varname, i);
		if ((in[i] = fopen(infn, "rb")) == NULL) {
			printf(" can't open file %s for reading!\n", infn); 
			return false;
		}
	}

	sprintf(outfn, "%s_%s_tac_%d.dat", filename, varname, tdim);
	out = fopen(outfn, "wb"); 
	for(int c=0; c<sizeofvolume; c++){
		for (int i=start; i<=end; i++){
			result = fseek( in[i], c*sizeof(float), SEEK_SET);
			if(!result){
				//printf("reading %s\r", infn);
				fread(ptdata, sizeof(float), 1, in[i]);
				A[i] = *ptdata;
			}
		}
		if(!result){
			fwrite( A, sizeof(float), tdim, out );
			if(c % 10000 == 0)
				printf("writing tac file: tac num = %d\n", c);
		}
		else
			printf("ERROR: create_temporal_file(): file access error!\n");
	}
	for (int i=start; i<=end; i++){
		fclose(in[i]);
	}

	// remove all temporary TAC files
	for (int i=start; i<=end; i++){
		sprintf(infn, "%s_%s_%04d.dat", filename, varname, i);
		if (remove(infn)!= NULL) {
			printf(" can't remove file %s!\n", infn); 
			//return false;
		}
	}

	delete [] in;
	delete [] A;
	fclose(out);
	return true;
}
// caller needs to pass fully allocated buffer to hold the data

int NetCDF::get_vardata_impl(NetCFD_var *var, 
							 int time_in, 
							 int count_in,
							 int start_var, 
							 int fraction,
							 /* out */char** data,
							 bool bFullSize)

{
	if (!var)
		return 0;
	char* var_name = var->var_name;
	nc_type rh_type = var->rh_type;
	int rh_ndims = var->rh_ndims;
	int* rh_dimids = var->rh_dimids;
	int temp_varid = var->temp_varid;
	size_t* start = var->start;
	size_t* count = var->count;
	int status;

	size_t total_size = 1;
	for (int j=0; j < rh_ndims; j++) {
		//remember count[0] should be 1 ( for 1 time step)
		/*
		if (strcmp(m_dim_name_arr[rh_dimids[j]], "time")==0) {
		start[j] = time_in;
		count[j] = count_in;
		}
		else {
		count[j] = m_dim_length_arr[rh_dimids[j]]/fraction;
		start[j] = start_var;
		}
		*/
		if(strcmp(var->dim[j]->dim_name,"time")==0){
			start[j] = time_in;
			count[j] = count_in;
		}
		else {
			if(!bFullSize){
				if(count[j] = var->dim[j]->query_count > fraction) 
					count[j] = var->dim[j]->query_count/fraction;
				else
					count[j] = var->dim[j]->query_count;
				start[j] = var->dim[j]->query_start;
			}
			else{
				count[j] = var->dim[j]->length;
				start[j] = 0;
			}
		}
		total_size = total_size * count[j];
	}

	if(rh_ndims == 0) total_size = 0;

	//cerr << "Get the variable : " << var_name << " size: " << total_size << " dimensions " << rh_ndims << endl;

	switch(rh_type){
		case NC_FLOAT:
			if ((status = nc_get_vara_float(m_ncid, temp_varid, start,
				count, reinterpret_cast<float *>(*data))))
				ERR(status);
			break;
		case NC_DOUBLE:
			if ((status = nc_get_vara_double(m_ncid, temp_varid, start,
				count, reinterpret_cast<double *>(*data))))
				ERR(status);
			break;
		case NC_INT:
			if ((status = nc_get_vara_int(m_ncid, temp_varid, start,
				count, reinterpret_cast<int *>(*data))))
				ERR(status);
			break;
		case NC_SHORT:
			if ((status = nc_get_vara_short(m_ncid, temp_varid, start,
				count, reinterpret_cast<short *>(*data))))
				ERR(status);
			break;
		case NC_CHAR:
		case NC_BYTE:
		default:
			break;
	}
	// end switch

	return status;

}

int NetCDF::read_var(NetCFD_var *var)
{	
	if (!var)
		return -1;
	char* var_name = var->var_name;
	nc_type rh_type = var->rh_type;
	int rh_ndims = var->rh_ndims;
	int* rh_dimids = var->rh_dimids;
	int temp_varid;
	size_t* start;
	size_t* count;
	int status;

	// Get the varids of the pressure and temperature netCDF
	// variables.
	//cout << "Get the variable id : " << var_name << endl;
	if ((status = nc_inq_varid(m_ncid, var_name, &temp_varid)))
		ERR(status);

	count = new size_t[rh_ndims];
	start = new size_t[rh_ndims];
	size_t total_size = 1;
	for (int j=0; j < rh_ndims; j++) {
		//remember count[0] should be 1 ( for 1 time step)
		if (strcmp(m_dim_name_arr[rh_dimids[j]], "time")==0)
			count[j] = 1;
		else
			count[j] = m_dim_length_arr[rh_dimids[j]];
		total_size = total_size * count[j];
		cout << "dim[" << j << "] = " << m_dim_length_arr[rh_dimids[j]] << endl;
		start[j] = 0;
	}

	if(rh_ndims == 0) total_size = 0;

	float*  temp_in_float;
	double* temp_in_double;
	int*    temp_in_int;
	short*  temp_in_short;
	cout << "Get the variable : " << var_name << " size: " << total_size << " dimensions " << rh_ndims << endl;
	// Save it to file
	cout << "Save the data." << endl;


	switch(rh_type){
		case NC_FLOAT:
			temp_in_float = new float[total_size];
			// Read and check one record at a time.
			if ((status = nc_get_vara_float(m_ncid, temp_varid, start,
				count, temp_in_float)))
				ERR(status);
			cout << "NC_FLOAT " << temp_in_float[0] << endl;
			//outf.write(reinterpret_cast<char *>(temp_in_float), sizeof(float) * total_size);             
			break;
		case NC_DOUBLE:
			temp_in_double = new double[total_size];
			// Read and check one record at a time.
			if ((status = nc_get_vara_double(m_ncid, temp_varid, start,
				count, temp_in_double)))
				ERR(status);
			cout << "NC_DOUBLE " << temp_in_double[0] << endl;
			//outf.write(reinterpret_cast<char *>(temp_in_double), sizeof(double) * total_size);
			break;
		case NC_INT:
			temp_in_int = new int[total_size];
			// Read and check one record at a time.
			if ((status = nc_get_vara_int(m_ncid, temp_varid, start,
				count, temp_in_int)))
				ERR(status);
			cout << "NC_INT " << temp_in_int[0] << endl;
			//outf.write(reinterpret_cast<char *>(temp_in_int), sizeof(int) * total_size);
			break;
		case NC_SHORT:
			temp_in_short = new short[total_size];
			// Read and check one record at a time.
			if ((status = nc_get_vara_short(m_ncid, temp_varid, start,
				count, temp_in_short)))
				ERR(status);
			cout << "NC_SHORT " << temp_in_short[0] << endl;
			//outf.write(reinterpret_cast<char *>(temp_in_short), sizeof(short) * total_size);
			break;
		case NC_CHAR:
		case NC_BYTE:
		default:
			break;
	}
	// end switch




	//outf.close();

	//deallocate memory
	delete count;
	delete start;
}


// TODO: right now min and max values are hardcoded.
// in the near future we need to provide a UI control
// for users to configure the min and max in the runtime
// and also be able to provide quantinization method preerence.
bool NetCDF::getMinMax(char* varname, float* min, float* max)
{
	bool bResult = false;
	if (strcmp(varname,"temp") == 0) {
		*min = 20.0f;
		*max = 30.0f;
		bResult = true;
	}
	else if (strcmp(varname,"salt") == 0) {
		*min = 30.0f;
		*max = 37.0f;
		bResult = true;
	}
	else if (strcmp(varname,"wt") == 0) {
		*min = -0.00000001f;
		*max = 0.00006;//0.000120973193; //0.0f;
		bResult = true;
	}
	return bResult;
}


/*
nc_open                  // open existing netCDF dataset
...
nc_inq                    // find out what is in it
...
nc_inq_dim            // get dimension names, lengths
...
nc_inq_var            // get variable names, types, shapes
...
nc_inq_attname      // get attribute names
...
nc_inq_att          // get attribute types and lengths
...
nc_get_att          // get attribute values
...
nc_get_var            // get values of variables
...
nc_close                  // close netCDF dataset
*/


int NetCDFUtils::GetCoordPosition(int item, int index)
{
	int result = 0;
	NetCFD_var* var_info = m_varlist[item];
	//result = index % m_size[item];


	switch(var_info->axis){
		case 'X':
			result = index % m_size[item];
			break;
		case 'Y':
			index = index / m_lenX;
			result = index % m_size[item];
			break;
		case 'Z':
			index = index / (m_lenX * m_lenY);
			result = index % m_size[item];
			break;

		default:
			result = index % m_size[item];
			break;
	}

	return result; 
}
/*
int NetCDFUtils::GetCoordPosition(int item, int index)
{
int result = 0;
NetCFD_var* var_info = m_varlist[item];
//result = index % m_size[item];

switch(var_info->axis){
case 'X':
result = index % m_size[item];
break;
case 'Y':
index = index / m_lenX;
result = index % m_size[item];
break;
case 'Z':
index = index / (m_lenX * m_lenY);
result = index % m_size[item];
break;

default:
result = index % m_size[item];
break;
}

return result; 
}
*/

float** NetCDFUtils::ReadVarData(int time_in, int count_in, bool bnew, bool bQuantization){
	int size = 0;
	if(m_num == 0)
		return NULL;
	if(m_pdata_arr){
		if(bnew)
			Clean();
		else
			return m_pdata_arr;
	}

	NetCFD_var* varX = m_netcdf->get_varinfo("xt_ocean");
	NetCFD_var* varY = m_netcdf->get_varinfo("yt_ocean");
	int new_lenX = varX->dim[0]->query_count - varX->dim[0]->query_start;
	int new_lenY = varY->dim[0]->query_count - varY->dim[0]->query_start;
	if(new_lenX >0)
		m_lenX = new_lenX;
	if(new_lenY >0)
		m_lenY = new_lenY;

	m_pdata_arr =  new float*[m_num];
	for(int i=0; i< m_num; i++)
		m_pdata_arr[i] = NULL;
	names = new char*[m_num];
	dim_num = new int[m_num];
	dim_min = new float[m_num];
	dim_max = new float[m_num];
	m_size = new int[m_num];
	for(int i=0; i < m_num; i++){
		//int len = get_vardata(i, time_in, count_in, data);
		NetCFD_var* var_info = m_varlist[i];
		names[i] = var_info->var_name;
		dim_num[i] = var_info->rh_ndims;
		// set the right size

		m_size[i] = m_netcdf->get_var_size(var_info->var_name); //var_info->total_size;
		/*
		for (int j = 0; j < var_info->rh_ndims; j++)
		m_size[i] = m_size[i];
		if(var_info->time_dep)
		m_size[i] = m_size[i];
		*/

		// read data
		float* data;		
		data = new float[m_size[i]*count_in];
		if(!data){
			printf("Memory allocation error!\n");
			return NULL;
		}

		int res = m_netcdf->get_vardata(var_info->var_name, time_in, count_in, 0, 1, reinterpret_cast<char **>(&data));
		if(res == 2)
		{
			delete data;
			return NULL;
		}
		// needs to be cached

		//vector<float> v;
		//for(int k=0; k< m_size[i];k++){
		//	v.push_back(data[k]);
		//}
		//std::sort(v.begin(), v.end());
		//for(int k=m_size[i]-1; k>=0 ;k--){
		//	data[k] = v[k];
		//}

		if(m_netcdf->getMinMax(var_info->var_name, &dim_min[i], &dim_max[i])){
			for(int j=0; j<m_size[i]; j++){
				if (data[j] < dim_min[i])
					data[j] = dim_min[i];
				if (data[j] > dim_max[i])
					data[j] = dim_max[i];
			}
		}
		else{
			dim_min[i] = data[0];
			dim_max[i] = data[0];
			for(int j=0; j<m_size[i]; j++){
				if (data[j] < dim_min[i])
					dim_min[i] = data[j];
				if (data[j] > dim_max[i])
					dim_max[i] = data[j];
			}
			//bQuantization = false;
		}
		//#ifdef QUANTIZATION

		if(bQuantization){
			for (int j = 0; j < m_size[i]; j++) {
				float value;
				//if(dim_max[i] >= 0 &&  dim_min < 0)
				//	value = -(dim_max[i] - dim_min[i]) + (data[j] - dim_min[i]) / (dim_max[i] - dim_min[i]);
				//else
				value = (data[j] - dim_min[i]) / (dim_max[i] - dim_min[i]);
				data[j] = value;
			}
		}
		//dim_min[i] = 0.0;
		//dim_max[i] = 1.0;
		//#endif                
		if(var_info->rh_type == NC_FLOAT && var_info->rh_ndims == 4){

			int sizet = var_info->dim[0]->length;
			int sizez = var_info->dim[1]->length;
			int sizey = var_info->dim[2]->length;
			int sizex = var_info->dim[3]->length;


			// reverse the z direction  

			for (int z = 0; z < sizez / 2; z++){
				for (int y = 0; y < sizey; y++){ 
					for (int x = 0; x < sizex; x++) {
						int org_id = x + y * sizex + z * sizex * sizey;
						int new_id = x + y * sizex + (sizez - 1 - z) * sizex * sizey;
						float temp = data[new_id];
						data[new_id] = data[org_id];
						data[org_id] = temp;
						//if(data[org_id] < 0.0 || data[new_id] < 0.0)
						//	printf("NetCDFUtils::ReadVarData success - %f, %f\n",data[org_id], data[new_id] );
						//data[org_id] = temp;
						//if (	(x <= (m_stopIndX *m_vSize[0] /100) && x >= (m_startIndX *m_vSize[0] /100)) && 
						//	(y <= (m_stopIndY *m_vSize[1] /100) && y >= (m_startIndY*m_vSize[1] /100)))
						//m_pIndVolume[new_id] = m_pVolume[new_id];
						//else 
						//	m_pIndVolume[new_id] = 0; //30
					}
				}
			}


		}


		//#endif


		m_pdata_arr[i] = data;		

	}


	return m_pdata_arr;
}	
/*
void NetCDFUtils::SampleData(float* data, float* sample_data, int size, int fraction, char* varname, bool bDim)
{
if(!data || !sample_data) 
return;

int j = 0;
int newsize = size/fraction;
if(!bDim){
float newmin = 0.0;
float newmax = 0.0;
//float newmax = 0.0;
if (strcmp(varname,"temp") == 0) {
newmin = 20.0f;
newmax = 30.0f;            
}
else if (strcmp(varname,"salt") == 0) {
newmin = 30.0f;
newmax = 37.0f;
}
else if (strcmp(varname,"wt") == 0) {
newmin = -0.00000001f;
newmax = 0.0f;
}



for(int i=0; i<size && j<newsize; i += fraction){
if (data[i] < newmin)
sample_data[j] = newmin;
else
sample_data[j] = data[i]; //(data[i] - newmin) / (newmax - newmin);
j++;
}
}
else{
for(int i=0; i<size && j<newsize; i += fraction){
sample_data[j] = data[i];
j++;
}
}
//sample_data[0] = data[0];
//sample_data[size/fraction] = data[size-1];
}


float NetCDFUtils::UpdateSampleData(float* sample_data, int size, char* varname, float min, float max)
{
float newmin = 0.0;
float newmax = 0.0;
if(!sample_data) 
return min;

//float newmax = 0.0;
if (strcmp(varname,"temp") == 0) {
newmin = 20.0f;
newmax = 30.0f;            
}
else if (strcmp(varname,"salt") == 0) {
newmin = 30.0f;
newmax = 37.0f;
}
else if (strcmp(varname,"wt") == 0) {
newmin = -10.0f;
newmax = 0.0f;
}


//#if defined(CLIMATE) 
//                
//                   if (m_pVolume[i] < -100)
//                        m_pVolume[i] = 0;
//                    else
//                        m_pVolume[i] = (value < 0) ? 0.1 : value;
//#else

for(int i=0; i<size; i++){
if (sample_data[i] < newmin)
sample_data[i] = 0;
else
sample_data[i] = (data[i] - newmin) / (max - newmin);
}
//sample_data[0] = data[0];
//sample_data[size/fraction] = data[size-1];

return newmin;
}

*/
// new_start in %
// new_count in %
bool NetCDFUtils::ChangeDataSize(int new_start, int new_count, char axis, int* out_count)
{
	bool bResult = true;
	if (axis == '\0')
		return false;

	for(int i=0; i < m_num; i++){
		NetCFD_var* var_info = m_varlist[i];
		for (int j=0; j < var_info->rh_ndims; j++) {
			NetCFD_var* temp_info = m_netcdf->get_varinfo(var_info->dim[j]->dim_name);
			if(temp_info && (temp_info->axis == axis)){
				if(new_start != -1){
					int val_new_start = new_start * var_info->dim[j]->length / 100;
					var_info->dim[j]->query_start = val_new_start;
					if((val_new_start + var_info->dim[j]->query_count) > var_info->dim[j]->length){
						int diff = 	(val_new_start + var_info->dim[j]->query_count) - var_info->dim[j]->length;
						var_info->dim[j]->query_count -= diff;
						bResult = false;
						if(out_count)
							*out_count = diff;
					}
				}

				if(new_count != -1){
					int val_new_count = new_count * var_info->dim[j]->length / 100; 
					var_info->dim[j]->query_count = val_new_count;
					if((val_new_count + var_info->dim[j]->query_start)> var_info->dim[j]->length){
						int diff = 	(val_new_count + var_info->dim[j]->query_start) - var_info->dim[j]->length;
						var_info->dim[j]->query_count -= diff;
						bResult = false;
						if(out_count)
							*out_count = diff;
					}
				}
			}

		}
	}
	return bResult;
}


void NetCDFUtils::Cluster_volumeTACs(	int x_start, int x_end, 
									 int y_start, int y_end, 
									 int z_start, int z_end,
									 char* varname_in, 
									 int time_in, int time_count, 
									 int xblocksize, int yblocksize, int zblocksize)
{
}


void NetCDFUtils::Save_volumeTACs(	int x_start, int x_end, 
								  int y_start, int y_end, 
								  int z_start, int z_end,
								  char* varname_in, 
								  int time_in, int time_count, 
								  int xblocksize, int yblocksize, int zblocksize,
								  char* fname){
  float** dataset;
  int len = xblocksize*yblocksize*zblocksize;
  char filename[1024];

  if(!fname)
	  sprintf(filename, "/tmp/data.bin", time_in, time_count,
	  x_start, x_end,
	  y_start, y_end,
	  z_start, z_end,
	  varname_in);
  else
	  strcpy(filename, fname);
  ofstream outf(filename, ios::binary);
  assert(outf);	
  if(!outf)
	  return;
  //FILE *out = fopen("c:\\jeffs\\write_debug.txt", "w");


  outf.write(reinterpret_cast<char *>(&x_start), sizeof(int));
  outf.write(reinterpret_cast<char *>(&x_end), sizeof(int));	
  outf.write(reinterpret_cast<char *>(&y_start), sizeof(int));
  outf.write(reinterpret_cast<char *>(&y_end), sizeof(int));	
  outf.write(reinterpret_cast<char *>(&z_start), sizeof(int));	
  outf.write(reinterpret_cast<char *>(&z_end), sizeof(int));	
  outf.write(reinterpret_cast<char *>(&time_in), sizeof(int));
  outf.write(reinterpret_cast<char *>(&time_count), sizeof(int));
  outf.write(reinterpret_cast<char *>(&xblocksize), sizeof(int));	
  outf.write(reinterpret_cast<char *>(&yblocksize), sizeof(int));	
  outf.write(reinterpret_cast<char *>(&zblocksize), sizeof(int));	

  int counter_out=0;
  float min = 100.0, max = -100.0;
  float* dataPts = new float[len];
  NetCFD_var* var_info = m_netcdf->get_varinfo(varname_in);
  int sizet, sizex, sizey, sizez;
  if (var_info->rh_ndims == 4)
  {
		//sizet = var_info->dim[0]->length;
		sizez = var_info->dim[1]->length;
		sizey = var_info->dim[2]->length;
		sizex = var_info->dim[3]->length;
  }
  while(time_in < time_count){
	  float* data = new float[sizez*sizey*sizex];
	  m_netcdf->get_vardata(const_cast<char *>(varname_in), 
							time_in, 1, 0, 1, 
							reinterpret_cast<char **>(&data), true);
	  

#if defined(CLIMATE)            
	  // reverse the z direction  
	  for (int z = 0; z < sizez / 2; z++){
		  for (int y = 0; y < sizey; y++) { 
			  for (int x = 0; x < sizex; x++) {
				int org_id = x + y * sizex + z * sizex * sizey;
				int new_id = x + y * sizex + (sizez - 1 - z) * sizex * sizey;
				float temp = data[new_id];
				data[new_id] = data[org_id];            
				data[org_id] = temp;
			  }
		  }
	  }
#endif            
	  if(m_netcdf->getMinMax(var_info->var_name, &min, &max)){
		  for(int j=0; j<sizez*sizey*sizex; j++){
			  if (data[j] < min)
				  data[j] = min;
			  if (data[j] > max)		  	
				  data[j] = max;
		  }
	  }	
	  else{	
		  min = data[0];	
		  max = data[0];	
		  for(int j=0; j<sizez*sizey*sizex; j++){	
			  if (data[j] < min)
				  min = data[j];
			  if (data[j] > max)
				  max = data[j];
			}
	  }

	  for(int x1=x_start; x1 <= x_end; x1+=xblocksize){
		  for(int y1=y_start; y1 <= y_end; y1+=yblocksize){
			  for(int z1=z_start; z1 <= z_end; z1+=zblocksize){
				  int count=0;
				  for(int x = x1; x< (x1+xblocksize) && x<=x_end ; x++){
					  for(int y = y1; y<(y1+yblocksize)&& y<=y_end ; y++){
						  for(int z = z1; z<(z1+zblocksize)&& z<=z_end ; z++){
							  int id = x + y * sizex + z * sizex * sizey;
							  /*
							  if(data[id] < -100)
								  dataPts[count] = 0.0;
							  else
								  dataPts[count] = data[id];
							  */
							  
							  //if(data[id] > -100){
								  dataPts[count] = data[id];
								  count++;
							  //}
							  
#ifdef DEBUG
							  //printf("dataPts[%d] %f\n",count, dataPts[count]);
#endif
							  //count++;
						  }
					  }
				  }
				  //printf("%d %d %d %d %d\n", count, x1, y1, z1, len);
				  //float med = FindMedian(dataPts, len, len/2);
				  float med = FindAvg(dataPts, len); 
				  //float med = FindVariance(dataPts, count);

				  if(max < med)
					  max = med;
				  if(min > med)
					  min = med;
				  //float var = FindVariance(dataPts, len);
				  //float avg = FindAvg(dataPts, len);

				  outf.write(reinterpret_cast<char *>(&med), sizeof(float));
				  //fprintf(out, "%f\n", med);
				  //outf.write(reinterpret_cast<char *>(&med), sizeof(float));
				  counter_out++;

			  }
		  }
	  }
	  delete [] data;
			 //i = m_num; // or break;
		  //}
	  

	  //fprintf(out, "time step %d\n", time_in);
	  time_in++;
  }

  outf.write(reinterpret_cast<char *>(&min), sizeof(float));
  outf.write(reinterpret_cast<char *>(&max), sizeof(float));
  //printf("min = %f, max = %f\n", min, max);

  delete [] dataPts;
  //fprintf(out, "total %d\n",counter_out);
  //fclose(out);
  outf.close();

}



void NetCDFUtils::Init() 
{
	int num = m_netcdf->get_num_vars();
	m_num = 0; 

	// get all variables that satisfy conditions in m_fltr
	m_varlist = new NetCFD_var*[num];
	for(int i=0; i < num; i++){
		if(m_varlist[m_num] = m_netcdf->get_varinfo_int(i,m_fltr)){
			//m_map[m_num] = i;
			m_mapvarname[m_varlist[m_num]->var_name] = m_num;
			m_num++;
		}
		else{
			m_varlist[m_num] = NULL;
		}
	}

	// get all varaibles that describe dimesions of previous vars.
	num = m_num;
	int j = 0;
	for(int i=0;i < num; i++){
		j = 0;
		while(	(j < m_varlist[i]->rh_ndims) && 
			(m_varlist[i]->rh_ndims > 1)) {
				char* dimname = m_varlist[i]->dim[j]->dim_name;
				if(	strcmp(dimname,"time")!=0 &&
					/*strcmp(dimname,"zt_ocean")!=0 &&*/
					m_mapvarname.find(dimname) == m_mapvarname.end()){

						m_varlist[m_num] = m_netcdf->get_varinfo(dimname);
						if(m_varlist[m_num]){
							m_varlist[m_num]->order = m_num - num;
							m_mapvarname[m_varlist[m_num]->var_name] = m_num;
							m_num++;
						}
				}
				j++;
		}
	}

	NetCFD_var* varX = m_netcdf->get_varinfo("xt_ocean");
	NetCFD_var* varY = m_netcdf->get_varinfo("yt_ocean");
	int new_lenX = varX->dim[0]->query_count - varX->dim[0]->query_start;
	int new_lenY = varY->dim[0]->query_count - varY->dim[0]->query_start;
	if(new_lenX >0)
		m_lenX = new_lenX;
	if(new_lenY >0)
		m_lenY = new_lenY;
}


float FindAvg(float* data, int len)
{
	float total = 0;
	for(int i=0; i < len; i++){
		total += data[i];
	}
	return (float)(total/len);
}

int compare( const void *arg1, const void *arg2 )
{
	/* Compare all of both strings: */
	float diff = (*((float*)arg1) - *((float*)arg2));
	if (diff = 0.0) return 0;
	else if (diff > 0.0) return 1;
	else return -1;
}


float FindMedian(float* data, int len, int pos)
{
	/* Sort remaining args using Quicksort algorithm: */
	qsort( (void *)data, (size_t)len, sizeof( float ), compare );
	return data[pos]; 

}

float FindVariance(float* data, int len)
{
	if(len == 0)
		return 0;
	float temp;
	float sum = 0;
	float avg = FindAvg(data, len);

	for(int j = 0; j < len; j++){
		temp = (data[j] - avg) * (data[j] - avg);
		sum = sum + temp;
	}
	return sqrt(sum/len);
}



#ifdef PEARSONCPU
void NetCDFUtils::get_pearson_correlation(char const fnhead[], 							 
										  char const fnheadref[],
										  bool const cross,
										  char const outfnhead[], 
										  int const start, int const end,
										  bool const diag,
										  int const sidx, int const eidx,
										  int const sidy, int const eidy,
										  int const sidz, int const eidz, 
										  int &outid, FILE *debug)
{
	int xyz;

	if (sidx!=eidx && sidy==eidy && sidz==eidz) xyz = 0; // change in x
	if (sidx==eidx && sidy!=eidy && sidz==eidz) xyz = 1; // change in y
	if (sidx==eidx && sidy==eidy && sidz!=eidz) xyz = 2; // change in z

	int i, l, lmin, lmax, offset = 0, tdim = end - start + 1;
	float *ptdata;
	float *data = new float[xyzdim*tdim];
	float *dataref;
	char infn[256], outfn[256];
	FILE *in, *out;

	// read data
	ptdata = data;
	for (i=start; i<=end; i++)
	{
		sprintf(infn, "%s_%04d.dat", fnhead, i);
		if ((in = fopen(infn, "rb")) == NULL) {
			printf(" can't open file %s for reading!\n", infn); 
			exit(1);
		}
		printf("reading %s\r", infn);
		fread(ptdata, sizeof(float), xyzdim, in);
		ptdata += xyzdim;
		fclose(in);
	}
	printf("\n");

	// read reference data

	if (cross) 
	{
		dataref = new float[xyzdim*tdim];

		ptdata = dataref;
		for (i=start; i<=end; i++)
		{
			sprintf(infn, "%s_%04d.dat", fnheadref, i);
			if ((in = fopen(infn, "rb")) == NULL) {
				printf(" can't open file %s for reading!\n", infn); 
				exit(1);
			}
			printf("reading %s\r", infn);
			fread(ptdata, sizeof(float), xyzdim, in);
			ptdata += xyzdim;
			fclose(in);
		}
		printf("\n");
	}

	float min = 100.0, max = -100.0;

	float *correl = new float[xyzdim];
	float *A = new float[tdim];
	float *B = new float[tdim];

	if (xyz==0) {
		lmin = 0; lmax = (sidx > eidx) ? sidx-eidx : eidx-sidx;
	}
	if (xyz==1) {
		lmin = 0; lmax = (sidy > eidy) ? sidy-eidy : eidy-sidy;
	}
	if (xyz==2) {
		lmin = 0; lmax = (sidz > eidz) ? sidz-eidz : eidz-sidz;
	}

	for (l=lmin; l<=lmax; l++)
	{
		// get A
		int pt;
		if (xyz==0) {
			if (sidx > eidx)
			{
				pt = sidz * xydim + sidy * xdim + (sidx - l);
				fprintf(debug, "%d %d %d\n", sidx - l, sidy, sidz);
			}
			else 
			{
				pt = sidz * xydim + sidy * xdim + (sidx + l);
				fprintf(debug, "%d %d %d\n", sidx + l, sidy, sidz);
			}
		}
		if (xyz==1) {
			if (sidy > eidy)
			{
				pt = sidz * xydim + (sidy - l) * xdim + sidx;
				fprintf(debug, "%d %d %d\n", sidx, sidy - l, sidz);
			}
			else 
			{
				pt = sidz * xydim + (sidy + l) * xdim + sidx;
				fprintf(debug, "%d %d %d\n", sidx, sidy + l, sidz);
			}
		}
		if (xyz==2) {
			if (sidz > eidz)
			{
				pt = (sidz - l) * xydim + sidy * xdim + sidx;
				fprintf(debug, "%d %d %d\n", sidx, sidy, sidz - l);
			}
			else 
			{
				pt = (sidz + l) * xydim + sidy * xdim + sidx;
				fprintf(debug, "%d %d %d\n", sidx, sidy, sidz + l);
			}
		}


		if (cross)
		{
			if (!diag)
				for (i=0; i<tdim; i++)
				{
					A[i] = dataref[pt]; // get from reference data
					pt += xyzdim;
				}
		}
		else
		{
			for (i=0; i<tdim; i++)
			{
				A[i] = data[pt]; // get from original data
				pt += xyzdim;
			}
		}

		int count;
		/*
		// loop through every B
		for (count=0; count<xyzdim; count++)
		{
		pt = count;
		for (i=0; i<tdim; i++)
		{
		B[i] = data[pt];
		pt += xyzdim;
		}
		correl[count] = calculate_pearson_correltion(A, B, tdim);
		if (correl[count]<min) min = correl[count];
		if (correl[count]>max) max = correl[count];
		}
		*/

		///*
		int index;
		float sum_sq_x, sum_sq_y, sum_coproduct, sweep, iflt, delta_x, delta_y, mean_x, mean_y;
		float pop_sd_x, pop_sd_y, cov_x_y;

		// loop through every B
		for (count=0; count<xyzdim; count++)
		{
			sum_sq_x = 0.0;
			sum_sq_y = 0.0;
			sum_coproduct = 0.0;
			index = count;
			if (cross && diag) A[0] = dataref[index];
			B[0] = data[index];
			index += xyzdim;
			mean_x = A[0];
			mean_y = B[0];
			iflt = 2.0;

			for (i=1; i<tdim; i++)
			{
				sweep = (iflt - 1.0) / iflt;
				if (cross && diag) A[i] = dataref[index];
				B[i] = data[index];  
				delta_x = A[i] - mean_x;
				delta_y = B[i] - mean_y;
				sum_sq_x += delta_x * delta_x * sweep;
				sum_sq_y += delta_y * delta_y * sweep;
				sum_coproduct += delta_x * delta_y * sweep;
				mean_x += delta_x / iflt;
				mean_y += delta_y / iflt;
				iflt += 1.0;
				index += xyzdim;
			}

			pop_sd_x = sqrt(sum_sq_x / tdim);
			pop_sd_y = sqrt(sum_sq_y / tdim);
			cov_x_y = sum_coproduct / tdim;
			if (pop_sd_x * pop_sd_y == 0.0) correl[count] = dummy;
			else correl[count] = cov_x_y / (pop_sd_x * pop_sd_y);
			if (correl[count]<min) min = correl[count];
			if (correl[count]>max) max = correl[count];
		}
		//*/

		///*
		printf("min = %f, max= %f\n", min, max);

		sprintf(outfn, "%s-%04d.dat", outfnhead, outid);
		if ((out = fopen(outfn, "wb")) == NULL) {
			printf(" can't open file %s for writing!\n", outfn); 
			exit(1);
		}
		printf("output %s...\r", outfn);
		//for (i=0; i<xyzdim; i++) fprintf(out, "%f\n", correl[i]);
		fwrite(correl, sizeof(float), xyzdim, out);
		fclose(out);
		outid++;
		//*/
	}
	printf("\n");

	delete []data;
	if (cross) delete []dataref;
	delete []correl;
	delete []A;
	delete []B;
}
#endif //PEARSONCPU