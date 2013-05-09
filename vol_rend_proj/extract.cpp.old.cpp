#include <stdio.h>
#include <string.h>
#include <netcdf.h>
#include <fstream>
#include <iostream>
#include <assert.h>

using namespace std;
   
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
   
/*
main()
{
    int ncid, temp_varid;
   
// The start and count arrays will tell the netCDF library where to
//    read our data. 
    size_t start[NDIMS], count[NDIMS];
   
// Program variables to hold the data we will read. We will only
//    need enough space to hold one timestep of data; one record.
    float *temp_in = new float[NZFULL* NLAT * NLON];
   
    // Error handling.
    int retval;
   
    // Open the file.
    cout << "Open " << FILE_NAME << endl;
    if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
        ERR(retval);
   
    // Get the varids of the pressure and temperature netCDF
    // variables. 
    cout << "Get the variable id : " << TEMP_NAME << endl;
    if ((retval = nc_inq_varid(ncid, TEMP_NAME, &temp_varid)))
        ERR(retval);
   
  // Read the data. Since we know the contents of the file we know
  // that the data arrays in this program are the correct size to
  // hold one timestep.
    count[0] = 1;
    count[1] = NZFULL;
    count[2] = NLAT;
    count[3] = NLON;
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    start[3] = 0;
   
    cout << "Get the variable : " << TEMP_NAME << endl;
    // Read and check one record at a time.
    if ((retval = nc_get_vara_float(ncid, temp_varid, start,
        count, temp_in)))
        ERR(retval);
 
    // Save it to file
    cout << "Save the data." << endl;
    ofstream outf("vorticity.dat");
    assert(outf);
    outf.write(reinterpret_cast<char *>(temp_in), sizeof(float) * NZFULL * NLAT * NLON);
    outf.close();
   
    // Close the file.
    if ((retval = nc_close(ncid)))
        ERR(retval);
   
    printf("*** SUCCESS reading!\n");
    return 0;
}

*/


#define FUNC(a,b,c) a ## b(c)
		

void test_1(int p)
{
	cout << "test_1() " << p << endl;
}

void test_2(int p)
{
	cout << "test_2() " << p << endl;
}

int main()
{
	int status, ncid, ndims, nvars, ngatts, unlimdimid;

	 
     
	 status = nc_open("C:\\uc_davis\\research\\src\\hongfeng\\netcdf\\sresa1b_ncar_ccsm3_0_run1_200001.nc", NC_NOWRITE, &ncid);
     if (status != NC_NOERR) ERR(status);
        
     //find out what is in it 
     status = nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid);
     if (status != NC_NOERR) ERR(status);
     
	 char* var_name = new char[NC_MAX_NAME];
	 char** dim_name_arr = new char*[NC_MAX_VAR_DIMS];
	 size_t* dim_length_arr = new size_t[NC_MAX_VAR_DIMS];

     for(int i=0; i < ndims; i++) {
		char* dim_name = new char[NC_MAX_NAME];
		 size_t lengthp;
     	// get dimension names, lengths
		status = nc_inq_dim(ncid, i, dim_name, &lengthp);
		if (status != NC_NOERR) ERR(status);
		dim_name_arr[i] = dim_name;
		dim_length_arr[i] = lengthp;
     }       

	 //for(int i=0; i < ndims; i++) {
	 //	 cout << "names: " << dim_name_arr[i] << endl;
	 //}
     
     for(int i=0; i < nvars; i++) {
     	//nc_inq_var             
		
		int  rh_id;                        /* variable ID */
		nc_type rh_type;                   /* variable type */
		int rh_ndims;                      /* number of dims */
		int rh_dimids[NC_MAX_VAR_DIMS];    /* dimension ids */
		int rh_natts;                       /* number of attributes */
        int temp_varid;
		size_t* start; 
		size_t*	count;

		//get variable names, types, shapes
		status = nc_inq_var (ncid, i, var_name, &rh_type, &rh_ndims, rh_dimids,
                          &rh_natts);
		if (status != NC_NOERR) ERR(status);

		// Get the varids of the pressure and temperature netCDF
		// variables. 
		//cout << "Get the variable id : " << var_name << endl;
		if ((status = nc_inq_varid(ncid, var_name, &temp_varid)))
			ERR(status);

		count = new size_t[rh_ndims];
		start = new size_t[rh_ndims];
		size_t total_size = 1;
		for (int j=0; j < rh_ndims; j++) {
			count[j] = dim_length_arr[rh_dimids[j]];
			total_size = total_size * count[j];
			start[j] = 0;
		}

		if(rh_ndims == 0) total_size = 0;
		
		float*  temp_in_float;
		double* temp_in_double;
		int*    temp_in_int;
		short*  temp_in_short;
		cout << "Get the variable : " << var_name << " size: " << total_size << endl;
		switch(rh_type){
			case NC_FLOAT:
				temp_in_float = new float[total_size];
				// Read and check one record at a time.
				if ((status = nc_get_vara_float(ncid, temp_varid, start,
					count, temp_in_float)))
					ERR(status);
				cout << "NC_FLOAT " << temp_in_float[0] << endl;
				break;
			case NC_DOUBLE:
				temp_in_double = new double[total_size];
				// Read and check one record at a time.
				if ((status = nc_get_vara_double(ncid, temp_varid, start,
					count, temp_in_double)))
					ERR(status);
				cout << "NC_DOUBLE " << temp_in_double[0] << endl;
				break;
			case NC_INT:
				temp_in_int = new int[total_size];
				// Read and check one record at a time.
				if ((status = nc_get_vara_int(ncid, temp_varid, start,
					count, temp_in_int)))
					ERR(status);
				cout << "NC_INT " << temp_in_int[0] << endl;
				break;
			case NC_SHORT:
				temp_in_short = new short[total_size];
				// Read and check one record at a time.
				if ((status = nc_get_vara_short(ncid, temp_varid, start,
					count, temp_in_short)))
					ERR(status);
				cout << "NC_SHORT " << temp_in_short[0] << endl;
				break;
			case NC_CHAR:
			case NC_BYTE:
			default: 
				break;
		} 
		// end switch
	    //nc_inq_attname      /* get attribute names */
        //  ...
        //nc_inq_att          /* get attribute types and lengths */
        //  ...
        //nc_get_att          /* get attribute values */
        //  ...

		//deallocate memory
		delete count;
		delete start;
	 }
    //nc_get_var             /* get values of variables */
    //  ...
    // Close the file.
    if ((status = nc_close(ncid)))
        ERR(status);                  /* close netCDF dataset */
         
}

/*
nc_open                   // open existing netCDF dataset 
           ...
         nc_inq                    // find out what is in it 
              ...
            nc_inq_dim             // get dimension names, lengths 
              ...
            nc_inq_var             // get variable names, types, shapes 
                 ...
               nc_inq_attname      // get attribute names 
                 ...
               nc_inq_att          // get attribute types and lengths 
                 ...
               nc_get_att          // get attribute values 
                 ...
            nc_get_var             // get values of variables 
              ...
         nc_close                  // close netCDF dataset 
*/