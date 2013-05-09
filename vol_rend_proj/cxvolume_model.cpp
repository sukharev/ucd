
#include "GL/glew.h"
#include "cxvolume_model.h"
#include "cxparameters.h"

#include "shader.h"
#define CUDA_CORR	

// Required to include CUDA vector types
#include <vector_types.h>
#include "cutil_inline.h"
#include <cuda.h>
extern "C" void runTest1(int N, int M,float* h_A, float* h_B, float* h_C, int max_num_threads);

cxVolumeModel::cxVolumeModel(int volID){
	m_pVolume = NULL;
	m_pIndVolume = NULL;
	m_startIndX = 0;
	m_stopIndX = 99;
	m_startIndY = 0;
	m_stopIndY = 99;
	m_IndYsize = 100;
	m_IndXsize = 100;
	m_time_in = 0;
	m_time_out = 1;
	m_timelen = 0;
	m_nVolumeType = VOLUME_TYPE_FLOAT;
	m_nTexVol = 0;
	m_nTexVolPearson = 0;
#ifdef SHOW_CLUSTER
	m_nTexClus = 0;
#endif

#ifdef SV_CORRELATION
	m_nTexCorr = 0;
#endif

	m_nIndTexVol = 0;
	m_nTexBack = 0;
	m_nVolProgram = 0;
	m_fNormalSpacing = 1.0/64.0;    
	m_bReload = false;

	m_nTexTF = 0;
	for(int i=0; i<3; i++){
		m_pGrid[i] = NULL;
	}

	m_nVolumeID = volID;

	//InitVolume();

	m_pTime = NULL;

	m_netcdf = NULL;

	m_TAC_start = 0;
	m_TAC_finish = 1;

	m_GridMap[0] = NULL;
	m_GridMap[1] = NULL;
	m_GridMap[2] = NULL;

	m_bcorr = false;
	m_corrdata = NULL;
	m_corrmap = NULL;

	//Pearson Correlation
	m_bCorrelation = false; // this flag activates all Pearson correlation.

	// one variable
	m_bPearsonVariable = false;
	//m_nStartTimePearson = 0;
	//m_nEndTimePearson = 17;  
	m_nStartTimePearson = 280;
	//m_nEndTimePearson = 315;
	//m_nNumTimePearson = 12; //36;  // same as NetCDF count of timesteps
	m_nEndTimePearson = 351;
	m_total_timesteps = TOTAL_CORREL_CALC;
	m_nNumTimePearson = TS_USED_IN_CORREL_CALC;//32;
	//m_nStartTimePearson = 0;
	//m_nEndTimePearson = 35;
	//m_nNumTimePearson = 36;
	
	m_fMin1 = 0.0;
	m_fMin2 = 0.0;
	m_fMax1 = 0.0;
	m_fMax2 = 0.0;

	m_pVisStatus = new cxVisStatus;
	m_pVisStatus->m_nVolumeID = volID;

	m_vars[0] = "";
	m_vars[1] = "";
}

cxVolumeModel::~cxVolumeModel() {
	ClearVolume();
	if(m_netcdf)
		delete m_netcdf;
	if(m_ms){
		delete m_ms;
		m_ms = NULL;
	}
}

void cxVolumeModel::SetSize(cx3DVector size)
{ 
	m_vSize = size;
	m_fMaxSize = m_vSize[0];

	if ( m_fMaxSize < m_vSize[1] )
		m_fMaxSize = m_vSize[1];

	if ( m_fMaxSize < m_vSize[2] )
		m_fMaxSize = m_vSize[2];

}

void cxVolumeModel::SetTimeLen(int time)
{
	m_timelen = time;
}

void cxVolumeModel::SetSize(int x, int y, int z)
{
	SetSize(cx3DVector(x, y, z));
}


void cxVolumeModel::SetCenter(cx3DVector center)
{ 
	m_vCenter = center;
}

void cxVolumeModel::SetCenter(int x, int y, int z)
{
	m_vCenter = cx3DVector(x, y, z);
}


#ifdef OLD_CODE
void cxVolumeModel::Read()
{

	ClearVolume();
	ClearVolTex();

	ReadFile();

	CreateDataTex();

	ClearVolume();

	//ReadGrid("C:/jeffs/uc_davis/src/vol_rend/vol_rend_proj/volumerender/grid.dat");

#ifdef LINUX
	//ReadTime("D:/jeffs/src/vol_rend/vol_rend_proj/volumerender/time.dat");
#endif
}

#endif

void cxVolumeModel::Read(const char* listitem, 
		const char* listitem2, 
		bool bCorrelation)
{
	m_bCorrelation = bCorrelation;
	ClearVolume();
	ClearVolTex();

#ifdef POINT_CLOUD
	ReadFlexible(m_netcdf, m_time_in, m_nt_pointcloud, listitem);
	CreateDataTex();
	CreateIndexDataTex();
#else

	if(!bCorrelation){    
		ReadNetCDF(m_netcdf, listitem, m_time_in, m_time_out);
		CreateDataTex();
		CreateIndexDataTex();

	}else{
		strcpy(m_bCorrItem1,listitem);
		strcpy(m_bCorrItem2,listitem);
		// reading data to be used with (GPU) Pearson Correlation
		int num_vars = 1;
		m_bPearsonVariable = false;
		//string vars[2];
		m_vars[0] = listitem;
		if(strcmp(listitem, listitem2)!=0){
			num_vars = 2;
			m_bPearsonVariable = true;
			m_vars[1] = listitem2;
		}
		else{
			m_vars[1] = listitem;
		}
#ifdef CUDA_CORR

		bool bTACexists = false;
		int n = 0;
		while(n < num_vars){
			bTACexists = m_netcdf->checktacfile(m_netcdf->m_filename,m_vars[n].c_str(), m_total_timesteps);
			if(!bTACexists)
				break;
			n++;
		}
		if(!bTACexists){
			ReadNetCDF3(m_netcdf, m_time_in, m_total_timesteps /*m_nNumTimePearson*/, m_vars, num_vars, true);
		}
		else{
			int dim[3];
			ClearVolTex();
			m_nVolumeSize = m_netcdf->get_totalsize(m_vars[0].c_str(),dim);
			for(int i=0; i < 3; i++)
				m_vSize[i] = dim[i];
			//SetTimeLen(t);
			if (m_pVolume != NULL){
				delete [] (float*) m_pVolume;
				m_pVolume = NULL;
			}
			SetSize(m_vSize[0], m_vSize[1], m_vSize[3]);
			SetCenter( m_vSize[0]/2, m_vSize[1]/2, m_vSize[2]/2);
			NetCFD_var* vinfo = m_netcdf->get_varinfo(m_vars[0].c_str());
			ReadGrid(vinfo);
			// allocate memory for NetCDF data for all timesteps and and all selected variables
			m_pVolume = new float[m_nVolumeSize]; // should be 2
			assert(m_pVolume);
		}
		// call CUDA correlation calculations kernel using data saved in TAC file
		calculate_corr(m_netcdf->m_filename, m_vars, num_vars);

#else
		ReadNetCDF2(m_netcdf, m_time_in, m_nNumTimePearson/*m_time_out*/, vars, num_vars);
#endif
		CreateDataTexPearson();
	}
#endif

#ifdef SHOW_CLUSTER
	CreateClusterTex();
#endif

#ifdef SV_CORRELATION
	CreateCorrTex();
#endif
	//ClearVolume();
	//ReadGrid("c:/jeffs/uc_davis/src/vol_rend/vol_rend_proj/volumerender/grid.dat");
#ifdef LINUX
	//	ReadTime("D:/jeffs/src/vol_rend/vol_rend_proj/volumerender/time.dat");
#endif
}




//TODO: needs to be re-written
#ifdef PEARSONCPU
void cxVolumeModel::ReadDataPearson(float mpVolume[], char * filehead, int const mnVariable, unsigned int mnTexVolPearson,
		bool debug, char fn[], int start, int end)
{
	int ts; 
	//int volumesize = m_nVolumeSize / m_nNumTimePearson;
	char head_file[1024];
	char data_file[1024];
	char imp_file[1024];
	char temp[1024];
	char ts_string[256];
	float *ptVolume = NULL;
	ostringstream timestep;

	if (mnTexVolPearson != 0)  return;
	//if (mnTexVolPearson != 0)  
	//   glDeleteTextures(1, (const GLuint*)&mnTexVolPearson);

	//mpVolume = new float[m_nVolumeSize];
	//float *mpVolume = new float[m_nVolumeSize*m_nNumTimePearson];
	//if(mpVolume == NULL) {
	//    cerr << "Memory leak" << endl;
	//    return;
	//}
	ptVolume = mpVolume;

	for (ts=m_nStartTimePearson; ts<=m_nEndTimePearson; ts++)
	{
#ifdef HURRICANE
		timestep << setw(2) << setfill('0') << ts;
#else
		timestep << setw(4) << setfill('0') << ts;
#endif
		cout << "Load timestep " << ts << endl;

		sprintf(temp, "%s.dat", filehead);
		sprintf(ts_string, "%04d", ts);
		sprintf(data_file, temp, ts_string);
		printf("%s\n", data_file);

		ifstream datafile(data_file, ios::binary);

		if (datafile == NULL) {
			cerr << "Can not open " << data_file << endl;
			return;
		}    

		if (m_nVolumeType == VOLUME_TYPE_FLOAT) {
			printf("address = %d\n", ptVolume);
			datafile.read(reinterpret_cast<char *>(ptVolume), sizeof(float) * m_nVolumeSize);
			//datafile.read(reinterpret_cast<char *>(ptVolume), sizeof(float) * volumesize);
			//ptVolume += sizeof(float) * m_nVolumeSize;
			ptVolume += m_nVolumeSize;
			//ptVolume += volumesize;
		}
		datafile.close();
	}
	/*
	   FILE *out;
	   out = fopen("test_all.txt", "wb");
	   for (int i=0; i<m_nVolumeSize; i++)
	   fprintf(out, "%f\n", mpVolume[i]);
	   fclose(out);
	   */
	// normalize the data

	float min, max, dummy = -10000000000.0f;

#ifdef QUANTIZATION
	if (mnVariable != DATA_MIXFRAC) {

		if (mnVariable == DATA_HO2) {
			min = 0;
			//max = 0.000343621;
			max = 0.000316;
		}            
		else if (mnVariable == DATA_OH) {
			min = 0;
			max = 0.0207485;
		}            
		else if (mnVariable == DATA_CHI) {
			//min = 0;
			//max = 12.2035;
			min = -2.0;
			//max = 0.75;
			max = 0.5;
		}
		else if (mnVariable == DATA_TEMP) {
			//min = 20.0f;
			//max = 30.0f;
			min = 100.0f;
			max = -100.0f;
			//min = max = mpVolume[start];
			for (int i = start; i <= end; i++) {
				if (mpVolume[i] != dummy)
				{
					if (min > mpVolume[i])
						min = mpVolume[i];
					if (max < mpVolume[i])
						max = mpVolume[i];
				}
			}
			printf("min = %f, max = %f\n", min, max);
		}
		else if (mnVariable == DATA_SALT) {
			//min = 30.0f;
			//max = 37.0f;  
			min = 100.0f;
			max = -100.0f;
			//min = max = mpVolume[start];
			for (int i = start; i <= end; i++) {
				if (mpVolume[i] != dummy)
				{
					if (min > mpVolume[i])
						min = mpVolume[i];
					if (max < mpVolume[i])
						max = mpVolume[i];
				}
			}
			printf("min = %f, max = %f\n", min, max);
		}
		else if (mnVariable == DATA_AMP) {
			min = 0.0f;
			max = 1.0f;            
		}
		else if (mnVariable == DATA_VORTS) {
			min = 0.002292f;
			max = 12.446266f;
		}
		else if (mnVariable == DATA_QVAPOR) {
			min = 0.0f;
			max = 0.02368f;
		}
		else if (mnVariable == DATA_Y_OH) {
			min = 0.0f;
			max = 0.008756f;
		}
		else {

#ifdef SUPERNOVA
			min = 0.0f;
			max = 204.728 * 1.5;

			//#elif defined (CLIMATE)
			//								min = 20.0f; max = 30.0f; // climate temp
			//min = 30.0f; max = 37.0f; // climate salt

			//#elif defined (EARTHQUAKE)
			//								min = 0.0f;
			//max = 3.029119f; // earthquake amplitude
			//								max = 1.0f;
#elif defined (FIVEJETS)
			min = 237647.640625f;
			//max = 346325.312500f; // fivejets energy 
			max = 300000.0f; 
#else
			min = max = mpVolume[start];

			//for (int i = 1; i < m_nVolumeSize*m_nNumTimePearson; i++) {
			for (int i = start; i <= end; i++) {
				//for (int i = 1; i < m_nVolumeSize; i++) {
				if (min > mpVolume[i])
					min = mpVolume[i];
				if (max < mpVolume[i])
					max = mpVolume[i];
			}
#endif                
			cout << "min " << min << " max " << max << endl;                
		}

		m_fMin = min;
		m_fMax = max;    
		//for (int i = 0; i < m_nVolumeSize*m_nNumTimePearson; i++) {
		for (int i = start; i <= end; i++) {
			//for (int i = 0; i < m_nVolumeSize; i++) {
			if (mnVariable == DATA_CHI) {
				if (mpVolume[i] > 0)
					mpVolume[i] = (log((float)mpVolume[i] / 10400.0f)/ log(10.0f) - min) / (max - min);
				if (mpVolume[i] < 0)
					mpVolume[i] = 0;
				if (mpVolume[i] > 1)
					mpVolume[i] = 1;
			}
			else {
#if defined (CLIMATE) 
				if (mpVolume[i] != dummy)
				{
					float value = (mpVolume[i] - min) / (max - min);
					//if (value>1.0 || value<0.0) printf("value = %f out of range!\n", value);
					// [0, 1] -> [0.1, 1]
					value = 0.9 * value + 0.1;
					mpVolume[i] = value;
				}
				else 
					mpVolume[i] = 0.0;
#else
				float value = (mpVolume[i] - min) / (max - min);
				mpVolume[i] = value;
#endif                                                        
					//*/
			}
		}                        
		if (debug)
		{
			FILE *out;
			out = fopen(fn, "wb");
			for (int i=0; i<m_nVolumeSize; i++)
				fprintf(out, "%f\n", mpVolume[i]);
			fclose(out);
		}
	}

#endif        
}

#endif //PEARSONCPU 

#ifdef POINT_CLOUD
void cxVolumeModel::ReadFlexible(NetCDF* netcdf,		//opened NetCDF file pointer
			int time_in,			//initial timestep 
			int count,			//number of timesteps requested (no less than 1)
			const char* var_name	//requested variable name
			)
{
	int x, y, z, t;
	x = y = z = t = 1;
	float* pdata;
	int start, end;
	FILE *out;			// for debugging
	char fn[512];		// for debugging
	bool debug = false;  // for debugging
	assert(netcdf != NULL);
	assert(m_netcdf != NULL);
	assert(count >= 1);
	int orig_count=count;
	if(debug){
		sprintf(fn,"d:\\jeffs\\corr_%s_debug.txt",var_name);
		out = fopen(fn, "w");
		assert(out);
	}
	if (m_pVolume != NULL){
		delete [] (float*) m_pVolume;
		m_pVolume = NULL;
	}
	ClearVolTex();
	if(!netcdf)
		netcdf = m_netcdf;
	if(m_netcdf != netcdf){
		delete m_netcdf;
		m_netcdf = netcdf;
	}
	NetCFD_var* vinfo = netcdf->get_varinfo(var_name);
	assert(vinfo);
	ReadGrid(vinfo);
	if (vinfo->rh_ndims == 4)
	{
		t = vinfo->dim[0]->length;
		z = vinfo->dim[1]->length;
		y = vinfo->dim[2]->length;
		x = vinfo->dim[3]->length;
	}
	m_nVolumeSize = x * y * z;
	assert(m_nVolumeSize > 0);

	//SetTimeLen(t);
	SetSize(x, y, z);
	SetCenter( x/2, y/2, z/2);
	
	// allocate memory for NetCDF data for all timesteps and and all selected variables
	m_pVolume = new float[m_nVolumeSize * orig_count*2]; // should be 2 
	assert(m_pVolume);
	pdata = m_pVolume;
	try{
		NetCFD_var* var_info;
		m_nVariable = DATA_CORR;
		// loop through the list of correlated variables 
		//fprintf(out, "test = %d\n",i);
		start = 0; end=m_nVolumeSize;
		//printf("i=%d, start=%d, end=%d\n",i,start,end);
		var_info = netcdf->get_varinfo(var_name);
		assert(var_info);
		assert(var_info->rh_ndims == 4);

		if (strcmp(var_info->var_name,"mixfrac") == 0)
			m_nVariable = DATA_MIXFRAC;
		else if (strcmp(var_info->var_name,"HO2") == 0)
			m_nVariable = DATA_HO2;

		else if (strcmp(var_info->var_name,"OH") == 0)
			m_nVariable = DATA_OH;

		else if (strcmp(var_info->var_name,"chi") == 0)
			m_nVariable = DATA_CHI;

		else if (strcmp(var_info->var_name,"temp") == 0)
					m_nVariable = DATA_TEMP;

		else if (strcmp(var_info->var_name,"salt") == 0)
			m_nVariable = DATA_SALT;
		else if (strcmp(var_info->var_name,"temp_a") == 0)
			m_nVariable = DATA_TEMP_A;

		else if (strcmp(var_info->var_name,"salt_a") == 0)
			m_nVariable = DATA_SALT_A;

		else if (strcmp(var_info->var_name,"wt") == 0)
			m_nVariable = DATA_WT;
		else
			m_nVariable = DATA_UNKNOWN;
		for(int j=0; j< count; j++){
			if ( var_info->rh_type == NC_FLOAT) {
				m_nVolumeType = VOLUME_TYPE_FLOAT;
			} else {
				assert(0); // some other data type
				m_nVolumeType = VOLUME_TYPE_FLOAT;
			}

			if (m_nVolumeType == VOLUME_TYPE_FLOAT) {
				// read data starting at timestep "time_in" and 
				// for the duration of "count" timesteps
				netcdf->get_vardata(const_cast<char *>(var_name), 
								time_in+j, 1, 0, 1, 
								reinterpret_cast<char **>(&pdata), true);
			}

			// process the data
			float min, max, dummy = -10000000000.0f;
#ifdef QUANTIZATION

			if (m_nVariable == DATA_HO2) {
				min = 0;
				//max = 0.000343621;
				max = 0.000316;
			}            
			else if (m_nVariable == DATA_OH) {
				min = 0;
				max = 0.0207485;
			}            
			else if (m_nVariable == DATA_CHI) {
				//min = 0;
				//max = 12.2035;
				min = -2.0;
				//max = 0.75;
				max = 0.5;
			}
			else if (m_nVariable == DATA_TEMP) {
				min = 20.0f;
				max = 30.0f;
				//min = 100.0f;
				//max = -100.0f;
				//printf("TEMP i=%d, start=%d, end=%d\n",i,start,end);
				for (int k = start; k < end; k++) {
				//assert(pdata[k]);
					if ( min > pdata[k] )
						pdata[k] = min;
					if ( max < pdata[k])
						pdata[k] = max;
				}
				printf("min = %f, max = %f\n", min, max);
			}
			else if (m_nVariable == DATA_SALT) {
				min = 30.0f;
				max = 37.0f;  
				//min = 100.0f;
				//max = -100.0f;
				//min = max = pdata[start];
				//printf("SALT i=%d, start=%d, end=%d\n",i,start,end);
				for (int k = start; k < end; k++) {
					//assert(pdata[k]);
					if ( min > pdata[k] )
						pdata[k] = min;
					if ( max < pdata[k])
						pdata[k] = max;
				}

				//printf("min = %f, max = %f\n", min, max);
			}
			else {

				min = max = pdata[start];
				//for (int i = 1; i < m_nVolumeSize*m_nNumTimePearson; i++) {
				for (int k = start; k < end; k++) {
				//for (int i = 1; i < m_nVolumeSize; i++) {
					if (min > pdata[k])
						min = pdata[k];
					if (max < pdata[k])
						max = pdata[k];
				}
				cout << "min " << min << " max " << max << endl;                
			}

			m_fMin = min;
			m_fMax = max;    
						//for (int i = 0; i < m_nVolumeSize*m_nNumTimePearson; i++) {
			for (int k = start; k < end; k++) {
				//for (int i = 0; i < m_nVolumeSize; i++) {
				if (m_nVariable == DATA_CHI) {
					if (pdata[k] > 0)
						pdata[k] = (log((float)pdata[k] / 10400.0f)/ log(10.0f) - min) / (max - min);

					if (pdata[k] < 0)
						pdata[k] = 0;
					if (pdata[k] > 1)
						pdata[k] = 1;
				}
				else {
					//#if defined (CLIMATE) 
					/*
					   if (pdata[i] != dummy)
					   {
					   float value = (pdata[i] - min) / (max - min);
					//if (value>1.0 || value<0.0) printf("value = %f out of range!\n", value);
					// [0, 1] -> [0.1, 1]
					value = 0.9 * value + 0.1;
					pdata[i] = value;
					}
					else 
						pdata[i] = 0.0;
								*/
								//#else
								//						float value = (pdata[i] - min) / (max - min);
								//						pdata[i] = value;
								//#endif                                                        
				}
			}

			m_fMin = min;
			m_fMax = max;

			for (int k = 0; k < m_nVolumeSize; k++) {
				if ( m_nVariable == DATA_CHI) {
					if (pdata[k] > 0)
						pdata[k] = ((float)log((float)m_pVolume[k] / 10400.0)/ log((float)10) - min) / (max - min);
					if (pdata[k] < 0)
						pdata[k] = 0;
					if (pdata[k] > 1)
						pdata[k] = 1;
				}else {
					float value = (pdata[k] - min) / (max - min);

#if defined(CLIMATE) 
					if (pdata[k] < -100)
						pdata[k] = 0;
					else
						pdata[k] = (value < 0) ? 0.1 : value;
#else
						pdata[k] = value;
#endif    
				}
			}



#if defined(CLIMATE)            


			// reverse the z direction  
			for (int z = 0; z < m_vSize[2] / 2; z++){
				for (int y = 0; y < m_vSize[1]; y++) { 
					for (int x = 0; x < m_vSize[0]; x++) {
						int org_id = x + y * m_vSize[0] + z * m_vSize[0] * m_vSize[1];
						int new_id = x + y * m_vSize[0] + (m_vSize[2] - 1 - z) * m_vSize[0] * m_vSize[1];
						float temp = pdata[new_id];
						pdata[new_id] = pdata[org_id];            
						pdata[org_id] = temp;
					}
				}
			}
#endif            
			if(debug)
			{
				assert(out);
				fprintf(out, "____________timestep %d__________\n",j);
				for (int k=0; k<m_nVolumeSize; k++){
					if(k<20 || k > m_nVolumeSize-20){
						fprintf(out, "%f\n", pdata[k]);
					}
				}

			}

#endif			


			pdata += m_nVolumeSize; 
		} //for timesteps

		if(strcmp(var_name,"temp")==0)
			m_nVariable = DATA_TEMP_TEMP;
		else if(strcmp(var_name,"salt")==0)
			m_nVariable = DATA_SALT_SALT;
		else if(strcmp(var_name,"wt")==0)
			m_nVariable = DATA_WT_WT;


		if(debug && out)
			fclose(out);

	}catch(...)
	{
		m_nVolumeSize = 0;
		if(m_pVolume)
			delete [] m_pVolume;
		if(debug && out)
			fclose(out);
	}
}
#endif


#define MAX_NUM_CORR_VAR	2  //maximum number of correlated variables
void cxVolumeModel::ReadNetCDF2(NetCDF* netcdf,		//opened NetCDF file pointer
			int time_in,			//initial timestep 
			int count,			//number of timesteps requested (no less than 1)
			string var_names[],  //requested variable names
			int num_vars			//number of variables requested (no more than 2)
			)
{
	int x, y, z, t;
	x = y = z = t = 1;
	float* pdata;
	int start, end;
	FILE *out;			// for debugging
	char fn[512];		// for debugging
	bool debug = false;  // for debugging
	assert(netcdf != NULL);
	assert(m_netcdf != NULL);
	assert(count >= 1);
	assert(num_vars <= 2 && num_vars >= 1);
	int orig_count=count;

	float arr_fMin[2];
	float arr_fMax[2];
	arr_fMin[0] = m_fMin1;
	arr_fMin[1] = m_fMin2;
	arr_fMax[0] = m_fMax1;
	arr_fMax[1] = m_fMax2;

	if(num_vars > 1)
		count = count/2;

	if(debug){
		if(num_vars == 1)
			sprintf(fn,"d:\\jeffs\\corr_%s_%s_debug.txt",var_names[0].c_str(),var_names[0].c_str());
		else
			sprintf(fn,"d:\\jeffs\\corr_%s_%s_debug.txt",var_names[0].c_str(),var_names[1].c_str());
		out = fopen(fn, "w");
		assert(out);
	}

	if (m_pVolume != NULL){
		delete [] (float*) m_pVolume;
		m_pVolume = NULL;
	}

	ClearVolTex();

	if(!netcdf)
		netcdf = m_netcdf;
	if(m_netcdf != netcdf){
		delete m_netcdf;
		m_netcdf = netcdf;
	}

	NetCFD_var* vinfo = netcdf->get_varinfo(var_names[0].c_str());
	assert(vinfo);
	ReadGrid(vinfo);

	if (vinfo->rh_ndims == 4)
	{
		t = vinfo->dim[0]->length;
		z = vinfo->dim[1]->length;
		y = vinfo->dim[2]->length;
		x = vinfo->dim[3]->length;
	}
	m_nVolumeSize = x * y * z;
	assert(m_nVolumeSize > 0);

	//SetTimeLen(t);
	SetSize(x, y, z);
	SetCenter( x/2, y/2, z/2);

	// allocate memory for NetCDF data for all timesteps and and all selected variables
	m_pVolume = new float[m_nVolumeSize * orig_count*2]; // should be 2 
	assert(m_pVolume);

	pdata = m_pVolume;
	try{

		NetCFD_var* var_info[MAX_NUM_CORR_VAR];
		m_nVariable = DATA_CORR;

		// loop through the list of correlated variables 
		for(int i=0; i < num_vars; i++){
			//fprintf(out, "test = %d\n",i);
			start = i*m_nVolumeSize; end=(i+1)*m_nVolumeSize;
			//printf("i=%d, start=%d, end=%d\n",i,start,end);
			var_info[i] = netcdf->get_varinfo(var_names[i].c_str());
			assert(var_info[i]);
			assert(var_info[i]->rh_ndims == 4);


			if (strcmp(var_info[i]->var_name,"mixfrac") == 0)
				m_nVariable = DATA_MIXFRAC;

			else if (strcmp(var_info[i]->var_name,"HO2") == 0)
				m_nVariable = DATA_HO2;

			else if (strcmp(var_info[i]->var_name,"OH") == 0)
				m_nVariable = DATA_OH;

			else if (strcmp(var_info[i]->var_name,"chi") == 0)
				m_nVariable = DATA_CHI;

			else if (strcmp(var_info[i]->var_name,"temp") == 0)
				m_nVariable = DATA_TEMP;


			else if (strcmp(var_info[i]->var_name,"salt") == 0)
				m_nVariable = DATA_SALT;
			else if (strcmp(var_info[i]->var_name,"temp_a") == 0)
				m_nVariable = DATA_TEMP_A;

			else if (strcmp(var_info[i]->var_name,"salt_a") == 0)
				m_nVariable = DATA_SALT_A;

			else if (strcmp(var_info[i]->var_name,"wt") == 0)
				m_nVariable = DATA_WT;
			else
				m_nVariable = DATA_UNKNOWN;

			for(int j=0; j< count; j++){
				if ( var_info[i]->rh_type == NC_FLOAT) {
					m_nVolumeType = VOLUME_TYPE_FLOAT;
				} else {
					assert(0); // some other data type
					m_nVolumeType = VOLUME_TYPE_FLOAT;
				}

				if (m_nVolumeType == VOLUME_TYPE_FLOAT) {
					// read data starting at timestep "time_in" and 
					// for the duration of "count" timesteps
					netcdf->get_vardata(const_cast<char *>(var_names[i].c_str()), 
							time_in+j, 1, 0, 1, 
							reinterpret_cast<char **>(&pdata), true);
				}

				// process the data
				float min, max, dummy = -10000000000.0f;
#ifdef QUANTIZATION

				if (m_nVariable == DATA_HO2) {
					min = 0;
					//max = 0.000343621;
					max = 0.000316;
				}            
				else if (m_nVariable == DATA_OH) {
					min = 0;
					max = 0.0207485;
				}            
				else if (m_nVariable == DATA_CHI) {
					//min = 0;
					//max = 12.2035;
					min = -2.0;
					//max = 0.75;
					max = 0.5;
				}
				else if (m_nVariable == DATA_TEMP) {
					min = 10.0f;
					max = 30.0f;
					//min = 100.0f;
					//max = -100.0f;
					//printf("TEMP i=%d, start=%d, end=%d\n",i,start,end);

				}
				else if (m_nVariable == DATA_SALT) {
					min = 30.0f;
					max = 37.0f;  
					//min = 100.0f;
					//max = -100.0f;
					//min = max = pdata[start];
					//printf("SALT i=%d, start=%d, end=%d\n",i,start,end);


					//printf("min = %f, max = %f\n", min, max);
				}
				/*
				   else if (m_nVariable == DATA_AMP) {
				   min = 0.0f;
				   max = 1.0f;            
				   }
				   else if (m_nVariable == DATA_VORTS) {
				   min = 0.002292f;
				   max = 12.446266f;
				   }
				   else if (m_nVariable == DATA_QVAPOR) {
				   min = 0.0f;
				   max = 0.02368f;
				   }
				   else if (m_nVariable == DATA_Y_OH) {
				   min = 0.0f;
				   max = 0.008756f;
				   }
				   */
				else {
					if(arr_fMin[i] < arr_fMax[i]){
						min = arr_fMin[i];
						max = arr_fMax[i];
					}
					else{
						min = max = pdata[start];

						//for (int i = 1; i < m_nVolumeSize*m_nNumTimePearson; i++) {
						for (int k = start; k < end; k++) {
							//for (int i = 1; i < m_nVolumeSize; i++) {
							if (min > pdata[k])
								min = pdata[k];
							if (max < pdata[k])
								max = pdata[k];
						}
						cout << "min " << min << " max " << max << endl; 
						arr_fMin[i] = min;
						arr_fMax[i] = max;
					}
				}

				if(arr_fMin[i] < arr_fMax[i]){
					min = arr_fMin[i];
					max = arr_fMax[i];
				}
				else{
					arr_fMin[i] = min;
					arr_fMax[i] = max; 
				}

				for (int k = start; k < end; k++) {
					//assert(pdata[k]);
					if ( min > pdata[k] )
						pdata[k] = min;
					if ( max < pdata[k])
						pdata[k] = max;
				}


				//for (int i = 0; i < m_nVolumeSize*m_nNumTimePearson; i++) {
				for (int k = start; k < end; k++) {
					//for (int i = 0; i < m_nVolumeSize; i++) {
					if (m_nVariable == DATA_CHI) {
						if (pdata[k] > 0)
							pdata[k] = (log((float)pdata[k] / 10400.0f)/ log(10.0f) - min) / (max - min);

						if (pdata[k] < 0)
							pdata[k] = 0;
						if (pdata[k] > 1)
							pdata[k] = 1;
					}
					else {
						//#if defined (CLIMATE) 
						/*
						   if (pdata[i] != dummy)
						   {
						   float value = (pdata[i] - min) / (max - min);
						//if (value>1.0 || value<0.0) printf("value = %f out of range!\n", value);
						// [0, 1] -> [0.1, 1]
						value = 0.9 * value + 0.1;
						pdata[i] = value;
						}
						else 
						pdata[i] = 0.0;
						*/
						//#else
						//						float value = (pdata[i] - min) / (max - min);
						//						pdata[i] = value;
						//#endif                                                        
					}
				}                        

				for (int k = 0; k < m_nVolumeSize; k++) {
					if ( m_nVariable == DATA_CHI) {
						if (pdata[k] > 0)
							pdata[k] = ((float)log((float)m_pVolume[k] / 10400.0)/ log((float)10) - min) / (max - min);

						if (pdata[k] < 0)
							pdata[k] = 0;
						if (pdata[k] > 1)
							pdata[k] = 1;
					}else {
						float value = (pdata[k] - min) / (max - min);

#if defined(CLIMATE) 
						if (pdata[k] < -100)
							pdata[k] = 0;
						else
							pdata[k] = (value < 0) ? 0.1 : value;
#else
						pdata[k] = value;
#endif    
					}
				}



#if defined(CLIMATE)            


				// reverse the z direction  
				for (int z = 0; z < m_vSize[2] / 2; z++){
					for (int y = 0; y < m_vSize[1]; y++) { 
						for (int x = 0; x < m_vSize[0]; x++) {
							int org_id = x + y * m_vSize[0] + z * m_vSize[0] * m_vSize[1];
							int new_id = x + y * m_vSize[0] + (m_vSize[2] - 1 - z) * m_vSize[0] * m_vSize[1];
							float temp = pdata[new_id];
							pdata[new_id] = pdata[org_id];            
							pdata[org_id] = temp;
						}
					}
				}
#endif            

				if(debug)
				{
					assert(out);
					fprintf(out, "____________timestep %d__________\n",j);
					for (int k=0; k<m_nVolumeSize; k++){
						if(k<20 || k > m_nVolumeSize-20){
							fprintf(out, "%f\n", pdata[k]);
						}
					}


				}

#endif			


				pdata += m_nVolumeSize; 


			} //for timesteps
		} //for variables
			// case of only one variable num_vars == 1

		/*
		   if(num_vars == 1){
		   int i = 0;
		   while(i < m_nVolumeSize){
		   m_pVolume[i+m_nVolumeSize] = m_pVolume[i];
		   i++;
		   }
		   }
		   */

		if(num_vars == 1){
			if(strcmp(var_names[0].c_str(),"temp")==0)
				m_nVariable = DATA_TEMP_TEMP;
			else if(strcmp(var_names[0].c_str(),"salt")==0)
				m_nVariable = DATA_SALT_SALT;
			else if(strcmp(var_names[0].c_str(),"wt")==0)
				m_nVariable = DATA_WT_WT;
		}
		else if(num_vars == 2){
			if((strcmp(var_names[0].c_str(),"temp")==0 && strcmp(var_names[1].c_str(),"salt")==0)||
					(strcmp(var_names[0].c_str(),"salt")==0 && strcmp(var_names[1].c_str(),"temp")==0))
				m_nVariable = DATA_SALT_TEMP;
			else if((strcmp(var_names[0].c_str(),"temp")==0 && strcmp(var_names[1].c_str(),"wt")==0)||
					(strcmp(var_names[0].c_str(),"wt")==0 && strcmp(var_names[1].c_str(),"temp")==0))
				m_nVariable = DATA_WT_TEMP;
			else if((strcmp(var_names[0].c_str(),"salt")==0 && strcmp(var_names[1].c_str(),"wt")==0)||
					(strcmp(var_names[0].c_str(),"wt")==0 && strcmp(var_names[1].c_str(),"salt")==0))
				m_nVariable = DATA_WT_SALT;
		}

		if(debug && out)
			fclose(out);

	}catch(...)
	{
		m_nVolumeSize = 0;
		if(m_pVolume)
			delete [] m_pVolume;
		if(debug && out)
			fclose(out);
	}
}


#ifdef CUDA_CORR


void cxVolumeModel::ReadNetCDF3(NetCDF* netcdf,	//opened NetCDF file pointer
			int time_in,		//initial timestep
			int timesteps,		//number of timesteps requested (no less than 1)
			string var_names[], 	//requested variable names
			int num_vars,		//number of variables requested (no more than 2)
			bool savetac)
{
	int x, y, z, t;
	x = y = z = t = 1;
	float* pdata;
	int start, end;
	FILE *out;			// for debugging
	char fn[512];		// for debugging
	assert(netcdf != NULL);
	assert(m_netcdf != NULL);
	assert(timesteps >= 1);
	assert(num_vars <= 2 && num_vars >= 1);
	int orig_count=timesteps;
	float arr_fMin[2];
	float arr_fMax[2];
	float min=0, max = 0;
	arr_fMin[0] = m_fMin1;
	arr_fMin[1] = m_fMin2;
	arr_fMax[0] = m_fMax1;
	arr_fMax[1] = m_fMax2;

	//if(num_vars > 1)
	//	timesteps = timesteps/2;


	if (m_pVolume != NULL){
		delete [] (float*) m_pVolume;
		m_pVolume = NULL;
	}

	ClearVolTex();

	if(!netcdf)
		netcdf = m_netcdf;
	if(m_netcdf != netcdf){
		delete m_netcdf;
		m_netcdf = netcdf;
	}

	NetCFD_var* vinfo = netcdf->get_varinfo(var_names[0].c_str());
	assert(vinfo);


	if (vinfo->rh_ndims == 4)
	{
		t = vinfo->dim[0]->length;
		z = vinfo->dim[1]->length;
		y = vinfo->dim[2]->length;
		x = vinfo->dim[3]->length;
	}
	m_nVolumeSize = x * y * z;
	assert(m_nVolumeSize > 0);

	//SetTimeLen(t);
	SetSize(x, y, z);
	SetCenter( x/2, y/2, z/2);

	ReadGrid(vinfo);
	// allocate memory for NetCDF data for all timesteps and and all selected variables
	m_pVolume = new float[m_nVolumeSize];// * orig_count*2]; // should be 2
	assert(m_pVolume);

	pdata = m_pVolume;
	try{

		NetCFD_var* var_info[MAX_NUM_CORR_VAR];
		m_nVariable = DATA_CORR;

		// loop through the list of correlated variables
		for(int i=0; i < num_vars; i++){
			//fprintf(out, "test = %d\n",i);
			//start = i*m_nVolumeSize*timesteps; end=(i+1)*m_nVolumeSize*timesteps;
			//printf("i=%d, start=%d, end=%d\n",i,start,end);
			var_info[i] = netcdf->get_varinfo(var_names[i].c_str());
			assert(var_info[i]);
			assert(var_info[i]->rh_ndims == 4);


			nametovar(var_info[i]->var_name,m_nVariable);
			for(int j=0; j< timesteps; j++){
				rhtypetotype(var_info[i]->rh_type, m_nVolumeType);

				if (m_nVolumeType == VOLUME_TYPE_FLOAT) {
					// read data starting at timestep "time_in" and
					// for the duration of "timesteps" timesteps
					netcdf->get_vardata(const_cast<char *>(var_names[i].c_str()),
									time_in+j, 1, 0, 1,
									reinterpret_cast<char **>(&pdata), true);
					
				}
				if(savetac){
					//save timestep to binary file
					if(!netcdf->savetimestep(netcdf->m_filename, var_names[i].c_str(), time_in+j, pdata, m_nVolumeSize))
						cout << "ERROR: ReadNetCDF3(): cannot save timestep " << j << endl;
				}
				//else{
				//pdata += m_nVolumeSize;
				//}
			} //for timesteps
			

			if(savetac){
				//create TAC file out of timestep files
				netcdf->createtacfile(netcdf->m_filename, var_names[i].c_str(), 0, timesteps-1, m_nVolumeSize);
			}
			//else{
			//	vartominmax(m_nVolumeType, pdata, arr_fMin, arr_fMax, i, timesteps);
			//}
			
		} //for variables
		// case of only one variable num_vars == 1

		if(num_vars == 1){
			if(strcmp(var_names[0].c_str(),"temp")==0 || strcmp(var_names[0].c_str(),"temp_a")==0)
				m_nVariable = DATA_TEMP_TEMP;
			else if(strcmp(var_names[0].c_str(),"salt")==0 || strcmp(var_names[0].c_str(),"salt_a")==0)
				m_nVariable = DATA_SALT_SALT;
			else if(strcmp(var_names[0].c_str(),"wt")==0)
				m_nVariable = DATA_WT_WT;
		}
		else if(num_vars == 2){
			if((strcmp(var_names[0].c_str(),"temp")==0 && strcmp(var_names[1].c_str(),"salt")==0)||
						 (strcmp(var_names[0].c_str(),"salt")==0 && strcmp(var_names[1].c_str(),"temp")==0))
				m_nVariable = DATA_SALT_TEMP;
			else if((strcmp(var_names[0].c_str(),"temp")==0 && strcmp(var_names[1].c_str(),"wt")==0)||
						      (strcmp(var_names[0].c_str(),"wt")==0 && strcmp(var_names[1].c_str(),"temp")==0))
				m_nVariable = DATA_WT_TEMP;
			else if((strcmp(var_names[0].c_str(),"salt")==0 && strcmp(var_names[1].c_str(),"wt")==0)||
						      (strcmp(var_names[0].c_str(),"wt")==0 && strcmp(var_names[1].c_str(),"salt")==0))
				m_nVariable = DATA_WT_SALT;
		}

	}catch(...)
	{
		m_nVolumeSize = 0;
		if(m_pVolume)
			delete [] m_pVolume;
	}


}

#endif

void cxVolumeModel::ReadNetCDF(NetCDF* netcdf, const char* index, int time_in, int time_out)
{
	// read header info
	if(!netcdf && !m_netcdf)
		return;
	ClearVolume();
	if(!netcdf && m_netcdf)
		netcdf = m_netcdf;
	if(m_netcdf != netcdf){
		delete m_netcdf;
		m_netcdf = netcdf;
	}
	try{

		strcpy(m_curnetcdf_var,index);

		int x, y, z, t;
		x = y = z = t = 1;
		string type;

		NetCFD_var* var_info = netcdf->get_varinfo(m_curnetcdf_var);
		ReadGrid(var_info);

		if (strcmp(var_info->var_name,"mixfrac") == 0)
			m_nVariable = DATA_MIXFRAC;

		else if (strcmp(var_info->var_name,"HO2") == 0)
			m_nVariable = DATA_HO2;

		else if (strcmp(var_info->var_name,"OH") == 0)
			m_nVariable = DATA_OH;

		else if (strcmp(var_info->var_name,"chi") == 0)
			m_nVariable = DATA_CHI;

		else if (strcmp(var_info->var_name,"temp") == 0)
			m_nVariable = DATA_TEMP;


		else if (strcmp(var_info->var_name,"salt") == 0)
			m_nVariable = DATA_SALT;
		else if (strcmp(var_info->var_name,"temp_a") == 0)
			m_nVariable = DATA_TEMP_A;

		else if (strcmp(var_info->var_name,"salt_a") == 0)
			m_nVariable = DATA_SALT_A;

		else if (strcmp(var_info->var_name,"wt") == 0)
			m_nVariable = DATA_WT;
		else
			m_nVariable = DATA_UNKNOWN;
		if (var_info->rh_ndims == 4)
		{
			t = var_info->dim[0]->length;
			z = var_info->dim[1]->length;
			y = var_info->dim[2]->length;
			x = var_info->dim[3]->length;
		}

		if ( var_info->rh_type == NC_FLOAT) {
			m_nVolumeType = VOLUME_TYPE_FLOAT;
		} else if (var_info->rh_type == NC_SHORT) {
			m_nVolumeType = VOLUME_TYPE_SHORT;       
		} else if (var_info->rh_type == NC_BYTE) {
			m_nVolumeType = VOLUME_TYPE_BYTE;       
		} else {
			//assert(0); // unknow data type
			m_nVolumeType = VOLUME_TYPE_FLOAT;
		}

		m_nVolumeSize = x * y * z;

		SetTimeLen(t);
		SetSize(x, y, z);
		SetCenter( x/2, y/2, z/2);

		//TODO: needs to be re-written and moved from here
		// maybe it needs be merged with this function (ReadNetCDF)...
#ifdef PEARSONCPU
		// read pearson correllation data:
		char pearson_data_file[256];
		sprintf(pearson_data_file, "%s", m_sFilename.c_str());
		m_pVolume = new float[m_nVolumeSize*m_nNumTimePearson];
		int start, end;
		if (!m_bPearsonVariable) {
			start = 0;
			end = (m_nVolumeSize*m_nNumTimePearson)-1;
			ReadDataPearson(m_pVolume, pearson_data_file, m_nVariable, m_nTexVolPearson, false, "test.txt", start, end);
		}
		else {
			start = 0;
			end = (m_nVolumeSize*m_nNumTimePearson/2)-1;
			ReadDataPearson(m_pVolume, pearson_data_file, m_nVariable, m_nTexVolPearson, false, "test.txt", start, end);
			ReadDataPearson(m_pVolume+(m_nVolumeSize*m_nNumTimePearson/2),
					"C:\\jeffs\\uc_davis\\src\\chaoli\\climate\\salt\\zorder\\salt_%s", DATA_SALT, m_nTexVolPearson,
					false, "salt.txt", start, end);
		}
#endif 

		// read data
		if ( m_nTexVol!= 0)  
			glDeleteTextures(1, (const GLuint*)&m_nTexVol);

		if ( m_nIndTexVol!= 0)
			glDeleteTextures(1, (const GLuint*)&m_nIndTexVol);

#ifdef SHOW_CLUSTER
		if ( m_nTexClus!= 0){
			glDeleteTextures(1, (const GLuint*)&m_nTexClus);
		}
#endif

		m_pVolume = new float[m_nVolumeSize];
		m_pIndVolume = new float[m_nVolumeSize];
		if( m_pVolume == NULL || m_pIndVolume == NULL) {
			cerr << "Memory leak" << endl;
			return;
		}

		if (m_nVolumeType == VOLUME_TYPE_FLOAT) {

			netcdf->get_vardata(const_cast<char *>(m_curnetcdf_var), 
					m_time_in, m_time_out, 0, 1, 
					reinterpret_cast<char **>(&m_pVolume), true);
			/*
			   FILE *out;
			   out = fopen("test1.bin", "wb");
			   fwrite(m_pVolume, sizeof(float), m_vSize[0]*m_vSize[1]*m_vSize[2], out);

			   fclose(out);  
			   */
			float min, max;

#ifdef QUANTIZATION
			if (m_nVariable != DATA_MIXFRAC ) {

				if (m_nVariable == DATA_HO2) {
					min = 0;
					//max = 0.000343621;
					max = 0.000316;
				}            
				else if (m_nVariable == DATA_OH) {
					min = 0;
					max = 0.0207485;
				}            
				else if (m_nVariable == DATA_CHI) {
					//min = 0;
					//max = 12.2035;
					min = -2.0;
					//max = 0.75;
					max = 0.5;
				}
				else if (m_nVariable == DATA_TEMP) {
					min = 20.0f;
					max = 30.0f;
					//min = max = m_pVolume[0];


				}
				else if (m_nVariable == DATA_SALT) {
					min = 30.0f;
					max = 37.0f;
					//min = max = m_pVolume[0];


				}
				else if (m_nVariable == DATA_SALT_A) {
					min = 0.0f;
					max = 1.9649162f;
					//min = max = m_pVolume[0];


				}

				else if (m_nVariable == DATA_TEMP_A) {
					min = 0.00f;
					max = 3.69f;
					//min = max = m_pVolume[0];


				}
				else if (m_nVariable == DATA_WT) {
					//min = -0.00001f;
					//max = 0.00001f;    
					min = -0.00000001f;
					max = 0.00006;

					//min = max = m_pVolume[0];


				}
				else { 
					if(m_fMin1 < m_fMax1){
						min = m_fMin1;
						max = m_fMax1;
					}
					else{
#ifdef SUPERNOVA
						min = 0.0f;
						max = 204.728 * 1.5;
#else
						min = max = m_pVolume[0];

						for (int i = 1; i < m_nVolumeSize; i++) {
							if ( min > m_pVolume[i] )
								min = m_pVolume[i];
							if ( max < m_pVolume[i])
								max = m_pVolume[i];
						}

						cout << "min " << min << " max " << max << endl;
#endif                
						m_fMin1 = min;
						m_fMax1 = max;
					}
				}

				if(m_fMin1 == m_fMax1){
					m_fMin1 = min;
					m_fMax1 = max;
				}
				else{
					min = m_fMin1;
					max = m_fMax1;
				}

				for (int i = 1; i < m_nVolumeSize; i++) {
					if ( min > m_pVolume[i] )
						m_pVolume[i] = min;
					if ( max < m_pVolume[i])
						m_pVolume[i] = max;
				}


				for (int i = 0; i < m_nVolumeSize; i++) {
					if ( m_nVariable == DATA_CHI) {
						if (m_pVolume[i] > 0)
							m_pVolume[i] = ((float)log((float)m_pVolume[i] / 10400.0)/ log((float)10) - min) / (max - min);

						if (m_pVolume[i] < 0)
							m_pVolume[i] = 0;
						if (m_pVolume[i] > 1)
							m_pVolume[i] = 1;
					}else {
						float value = (m_pVolume[i] - min) / (max - min);

#if defined(CLIMATE) 
						//m_fMin = 0.0;
						//m_fMax = 1.0;
						if (m_pVolume[i] < -100) // should this be user defined
							m_pVolume[i] = min;//0;
						else
							m_pVolume[i] = value;
						//	m_pVolume[i] = (value < 0) ? 0.1 : value;
#else
						m_pVolume[i] = value;
#endif    
					}
				}


				if (m_stopIndX == m_startIndX &&  m_startIndX == 0){
					m_stopIndX = m_vSize[0];
					m_startIndX = 0;
				} 

				if (m_stopIndY == m_startIndY &&  m_startIndY == 0){
					m_stopIndY = m_vSize[1];
					m_startIndY = 0;
				} 
#if defined(CLIMATE)            


				// reverse the z direction  
				for (int z = 0; z < m_vSize[2] / 2; z++)
					for (int y = 0; y < m_vSize[1]; y++) 
						for (int x = 0; x < m_vSize[0]; x++) {
							int org_id = x + y * m_vSize[0] + z * m_vSize[0] * m_vSize[1];
							int new_id = x + y * m_vSize[0] + (m_vSize[2] - 1 - z) * m_vSize[0] * m_vSize[1];
							float temp = m_pVolume[new_id];
							m_pVolume[new_id] = m_pVolume[org_id];            
							m_pVolume[org_id] = temp;

							if (	(x <= (m_stopIndX *m_vSize[0] /100) && x >= (m_startIndX *m_vSize[0] /100)) && 
									(y <= (m_stopIndY *m_vSize[1] /100) && y >= (m_startIndY*m_vSize[1] /100)))
								m_pIndVolume[new_id] = m_pVolume[new_id];
							else 
								m_pIndVolume[new_id] = 0; //30

						}

#endif            

			}

#endif

		} else if (m_nVolumeType == VOLUME_TYPE_SHORT) {

			unsigned short * pTempVolume = new unsigned short[m_nVolumeSize];
			assert(pTempVolume);
			netcdf->get_vardata(const_cast<char *>(m_curnetcdf_var), m_time_in, m_time_out, 0,1, reinterpret_cast<char **>(&pTempVolume), true);

			float max = pTempVolume[0];
			float min = pTempVolume[0];

			//find min and max
			for (int i = 0; i < m_nVolumeSize; i++) {
				if ( max < pTempVolume[i])
					max = pTempVolume[i];
				if ( min > pTempVolume[i])
					min = pTempVolume[i];
			}

			max *= 0.01;
			cout << "Max " << max << endl;
			cout << "Min " << min << endl;

			for (int i = 0; i < m_nVolumeSize; i++) 
				m_pVolume[i] = ((float) pTempVolume[i] - min) / (max - min);           

			//
			//for (int i = 0; i < m_nVolumeSize; i++) 
			//    m_pVolume[i] = ((float) pTempVolume[i] ) / 0xffff;            
			//

			delete [] pTempVolume;

		} else if (m_nVolumeType == VOLUME_TYPE_BYTE) {

			unsigned char * pTempVolume = new unsigned char[m_nVolumeSize];
			assert(pTempVolume);
			netcdf->get_vardata(const_cast<char *>(m_curnetcdf_var), m_time_in, m_time_out, 0,1,reinterpret_cast<char **>(&pTempVolume), true);

			for (int i = 0; i < m_nVolumeSize; i++) {
				m_pVolume[i] = ((float)pTempVolume[i]) / 0xff;
			}

			delete [] pTempVolume;
		}

#ifdef HISTOGRAM
		int wid = m_pHistogram->w();
		int hei = m_pHistogram->h();

		int * count = new int[wid];
		assert(count);

		memset(count, 0, sizeof(int) * wid);

		int max = 0;
		for (int i = 0; i < m_nVolumeSize; i++) {
			float value = m_pVolume[i];

			int id = int(value * float(wid - 1));
			if (id > wid - 1)
				id = wid - 1;

			count[id]++;

			if (count[id] > max)
				max = count[id];

		}

		int sum = 0;
		for ( int i = 0; i < wid; i++)
			sum += count[i];

		int avg = sum / wid;

		uchar * histogram = m_pHistogram->m_pImage;
		memset(histogram, 0, wid * hei * 3);

		for (int x = 0; x < wid; x++) {
			int max_id = int(float(count[x]) / float(avg) * float(hei)); 
			if (max_id > hei - 1)
				max_id = hei - 1;

			for (int y = hei - 1; y > hei -1 - max_id; y--) {
				int id = x + y * wid;
				histogram[id * 3 + 0] = 255;
				histogram[id * 3 + 1] = 255;
				histogram[id * 3 + 2] = 255;
			}
		}    
		delete count; 
#endif

		char fn[512];
		sprintf(fn,"d:\\jeffs\\1var_debug_%s.txt",var_info->var_name);
		bool debug = false;
		if (debug)
		{
			FILE *out;
			out = fopen(fn, "w");
			for (int i=0; i<m_nVolumeSize; i++)
				if(i<20 || i > m_nVolumeSize-20)
					fprintf(out, "%f\n", m_pVolume[i]);
			fclose(out);
		}
	}
	catch(...)
	{
		m_nVolumeSize = 0;
	}
}




void cxVolumeModel::ReadGrid(NetCFD_var* var_info)
{
	assert(var_info);
	/* allocate memory */ 
	int i = 0;
	for (int n = 0; n < var_info->rh_ndims; n++) {
		if(strcmp(var_info->dim[n]->dim_name,"time")==0)
			continue;
		int size = (int)var_info->dim[n]->length;

		if (m_pGrid[i] != NULL)
			delete [] m_pGrid[i];

		m_pGrid[i] = new float[size];
		assert(m_pGrid[i]);

		m_netcdf->get_vardata(const_cast<char *>(var_info->dim[n]->dim_name), 
				0, 1, 0, 1, 
				reinterpret_cast<char **>(&m_pGrid[i]), true);
		/* normalize */
		float min, max;

		min = m_pGrid[i][0];
		max = m_pGrid[i][size - 1];

		for (int j = 0; j < size; j++) {
			m_pGrid[i][j] = (m_pGrid[i][j] - min) / (max - min);
		}
		i++;
	}

	//rearrange m_pGrid
	float* temp = m_pGrid[2];
	m_pGrid[2] = m_pGrid[0];
	m_pGrid[0] = temp;

	for (int axis = 0; axis < 3; axis++) {        

		int size = m_vSize[axis];
		if(m_GridMap[axis]){
			//delete [] m_GridMap[axis];
			delete m_GridMap[axis];
		}

		m_GridMap[axis] = new float[size];

		for (int j = 0; j < size; j++) {
			m_GridMap[axis][j] = m_pGrid[axis][j];
		}
	}
}

void cxVolumeModel::ReadGrid(char * filename)
{
	/* read the grid file */
	ifstream inf(filename, ios_base::binary);
	assert(inf);

	/* allocate memory */        
	for (int i = 0; i < 3; i++) {

		int size = (int)m_vSize[i];

		if (m_pGrid[i] != NULL)
			delete [] m_pGrid[i];

		m_pGrid[i] = new float[size];
		assert(m_pGrid[i]);

		inf.read(reinterpret_cast<char *>(m_pGrid[i]), sizeof(float) * size);

		/* normalize */
		float min, max;

		min = m_pGrid[i][0];
		max = m_pGrid[i][size - 1];

		for (int j = 0; j < size; j++) {
			m_pGrid[i][j] = (m_pGrid[i][j] - min) / (max - min);
		}
	}

	inf.close();    


	/*
	   for (int axis = 0; axis < 3; axis++) {        

	   int size = m_vSize[axis];
	   m_GridMap[axis] = new float[GRID_SAMPLE]; 	
	// fill the look up table
	for (int i = 0; i < size - 1; i++) {
	int start = m_pGrid[axis][i] * GRID_SAMPLE;            
	int end = m_pGrid[axis][i+1] * GRID_SAMPLE;

	float start_v = (float) i / size;            
	float end_v = (float) (i+1) / size;
	for (int j = start; j <= end; j++) {
	float factor = ((float) (j - start)) / (end - start);
	float value = start_v * (1-factor) + end_v * (factor);

	m_GridMap[axis][j] = value;

	}
	}
	}
	*/
	for (int axis = 0; axis < 3; axis++) {        

		int size = m_vSize[axis];
		if(m_GridMap[axis]){
			//delete [] m_GridMap[axis];
			delete m_GridMap[axis];
		}

		m_GridMap[axis] = new float[size];

		for (int j = 0; j < size; j++) {
			m_GridMap[axis][j] = m_pGrid[axis][j];
		}
	}

}


void cxVolumeModel::ReadTime(char *filename)
{
	int totaltime = m_nEndTime - m_nStartTime + 1;

	if (m_pTime == NULL) {

		m_pTime = new float[totaltime];
		assert(m_pTime);
	}

	/* read the time file */
	ifstream inf(filename);
	assert(inf);

	inf.read(reinterpret_cast<char *>(m_pTime), sizeof(float) * totaltime);

	inf.close();
}

#ifdef OLD_CODE
void cxVolumeModel::ReadFile()
{
	char head_file[1024];
	char data_file[1024];
	char temp[1024];

	if (m_nStartTime == -1){
		sprintf(head_file, "%s.hdr", m_sFilename.c_str());
		sprintf(data_file, "%s.dat", m_sFilename.c_str());
	}
	else {
		cout << "Load timestep " << m_nCurTime << endl;

		ostringstream timestep;
		timestep << setw(4) << setfill('0') << m_nCurTime;

		sprintf(temp, "%s.hdr", m_sFilename.c_str());
		sprintf(head_file, temp, timestep.str().c_str(), timestep.str().c_str());

		sprintf(temp, "%s.dat", m_sFilename.c_str());
		sprintf(data_file, temp, timestep.str().c_str(), timestep.str().c_str());

	}

	if (m_sFilename.find("mixfrac") != string::npos)
		m_nVariable = DATA_MIXFRAC;

	if (m_sFilename.find("HO2") != string::npos)
		m_nVariable = DATA_HO2;

	if (m_sFilename.find("OH") != string::npos)
		m_nVariable = DATA_OH;

	if (m_sFilename.find("chi") != string::npos)
		m_nVariable = DATA_CHI;

	if (m_sFilename.find("temp") != string::npos)
		m_nVariable = DATA_TEMP;

	if (m_sFilename.find("salt") != string::npos)
		m_nVariable = DATA_SALT;     
	if (m_sFilename.find("salt_a") != string::npos)
		m_nVariable = DATA_SALT_A;  

	ReadHeader(head_file);       
	ReadData(data_file);    
}

void cxVolumeModel::ReadHeader(char * filename)
{
	int x, y, z;
	string type;

#ifdef READ_HEADFILE    
	ifstream headfile(filename); 
	if (headfile == NULL) {
		cerr << "Can not open " << filename << endl;
		return;
	}          

	headfile >> x >> y >> z;
	headfile >> type;
	headfile.close();    

	if (type == "FLOAT" || type == "Float" || type == "float") {
		m_nVolumeType = VOLUME_TYPE_FLOAT;
	} else if (type == "SHORT" || type == "Short" || type == "short") {
		m_nVolumeType = VOLUME_TYPE_SHORT;        
	} else if (type == "BYTE" || type == "Byte" || type == "byte") {
		m_nVolumeType = VOLUME_TYPE_BYTE;        
	} else {
		//assert(0); // unknow data type
		m_nVolumeType = VOLUME_TYPE_FLOAT;
	}
#else
	x = 800;
	y = 686;
	z = 215;    
	m_nVolumeType = VOLUME_TYPE_FLOAT;
#endif

	m_nVolumeSize = x * y * z;

	SetSize(x, y, z);
	SetCenter( x/2, y/2, z/2);
}

void cxVolumeModel::ReadData(char * filename)
{
	ifstream datafile(filename, ios_base::binary);

	if (datafile == NULL) {
		cerr << "Can not open " << filename << endl;
		return;
	}    

	if ( m_nTexVol!= 0)  
		glDeleteTextures(1, (const GLuint*)&m_nTexVol);

	if ( m_nIndTexVol!= 0)  
		glDeleteTextures(1, (const GLuint*)&m_nIndTexVol);

#ifdef SHOW_CLUSTER
	if ( m_nTexClus!= 0){
		glDeleteTextures(1, (const GLuint*)&m_nTexClus);
	}
#endif

#ifdef SV_CORRELATION
	if (m_nTexCorr != 0) {
		glDeleteTextures(1, (const GLuint*)&m_nTexCorr);
		m_nTexCorr = 0;
	}
#endif

	m_pVolume = new float[m_nVolumeSize];
	if( m_pVolume == NULL) {
		cerr << "Memory leak" << endl;
		return;
	}

	if (m_nVolumeType == VOLUME_TYPE_FLOAT) {

		datafile.read(reinterpret_cast<char *>(m_pVolume), sizeof(float) * m_nVolumeSize);

		float min, max;

#ifdef QUANTIZATION
		if (m_nVariable != DATA_MIXFRAC ) {

			if (m_nVariable == DATA_HO2) {
				min = 0;
				//max = 0.000343621;
				max = 0.000316;
			}            
			else if (m_nVariable == DATA_OH) {
				min = 0;
				max = 0.0207485;
			}            
			else if (m_nVariable == DATA_CHI) {
				//min = 0;
				//max = 12.2035;
				min = -2.0;
				//max = 0.75;
				max = 0.5;
			}
			else if (m_nVariable == DATA_TEMP) {
				min = 20.0f;
				max = 30.0f;            
			}
			else if (m_nVariable == DATA_SALT) {
				min = 30.0f;
				max = 37.0f;            
			}
			else if (m_nVariable == DATA_WT) {
				min = 30.0f;
				max = 37.0f;            
			}
			else { 

#ifdef SUPERNOVA
				min = 0.0f;
				max = 204.728 * 1.5;
#else
				min = max = m_pVolume[0];

				for (int i = 1; i < m_nVolumeSize; i++) {
					if ( min > m_pVolume[i] )
						min = m_pVolume[i];
					if ( max < m_pVolume[i])
						max = m_pVolume[i];
				}

				cout << "min " << min << " max " << max << endl;
#endif                

			}


			for (int i = 0; i < m_nVolumeSize; i++) {
				if ( m_nVariable == DATA_CHI) {
					if (m_pVolume[i] > 0)
						m_pVolume[i] = ((float)log((float)m_pVolume[i] / 10400.0)/ log((float)10) - min) / (max - min);

					if (m_pVolume[i] < 0)
						m_pVolume[i] = 0;
					if (m_pVolume[i] > 1)
						m_pVolume[i] = 1;
				}else {


					float value = (m_pVolume[i] - min) / (max - min);
					m_fMin = 0.0;
					m_fMax = 1.0;
#if defined(CLIMATE) 

					if (m_pVolume[i] < -100)
						m_pVolume[i] = 0;
					else
						m_pVolume[i] = (value < 0) ? 0.1 : value;
#else
					m_pVolume[i] = value;
#endif                                    

				}
			}

#if defined(CLIMATE)            

			/* reverse the z direction */ 
			for (int z = 0; z < m_vSize[2] / 2; z++)
				for (int y = 0; y < m_vSize[1]; y++) 
					for (int x = 0; x < m_vSize[0]; x++) {
						int org_id = x + y * m_vSize[0] + z * m_vSize[0] * m_vSize[1];
						int new_id = x + y * m_vSize[0] + (m_vSize[2] - 1 - z) * m_vSize[0] * m_vSize[1];
						float temp = m_pVolume[new_id];
						m_pVolume[new_id] = m_pVolume[org_id];            
						m_pVolume[org_id] = temp;
					}

#endif            

		}

#endif        
	} else if (m_nVolumeType == VOLUME_TYPE_SHORT) {

		unsigned short * pTempVolume = new unsigned short[m_nVolumeSize];
		assert(pTempVolume);
		datafile.read(reinterpret_cast<char *>(pTempVolume), sizeof(unsigned short) * m_nVolumeSize);

		float max = pTempVolume[0];
		float min = pTempVolume[0];

		//find min and max
		for (int i = 0; i < m_nVolumeSize; i++) {
			if ( max < pTempVolume[i])
				max = pTempVolume[i];
			if ( min > pTempVolume[i])
				min = pTempVolume[i];
		}

		max *= 0.01;
		cout << "Max " << max << endl;
		cout << "Min " << min << endl;

		for (int i = 0; i < m_nVolumeSize; i++) 
			m_pVolume[i] = ((float) pTempVolume[i] - min) / (max - min);           

		/*
		   for (int i = 0; i < m_nVolumeSize; i++) 
		   m_pVolume[i] = ((float) pTempVolume[i] ) / 0xffff;            
		   */

		delete [] pTempVolume;

	} else if (m_nVolumeType == VOLUME_TYPE_BYTE) {

		unsigned char * pTempVolume = new unsigned char[m_nVolumeSize];
		assert(pTempVolume);
		datafile.read(reinterpret_cast<char *>(pTempVolume), sizeof(unsigned char) * m_nVolumeSize);

		for (int i = 0; i < m_nVolumeSize; i++) {
			m_pVolume[i] = ((float)pTempVolume[i]) / 0xff;
		}

		delete [] pTempVolume;
	}

#ifdef HISTOGRAM
	int wid = m_pHistogram->w();
	int hei = m_pHistogram->h();

	int * count = new int[wid];
	assert(count);

	memset(count, 0, sizeof(int) * wid);

	int max = 0;
	for (int i = 0; i < m_nVolumeSize; i++) {
		float value = m_pVolume[i];

		int id = int(value * float(wid - 1));
		if (id > wid - 1)
			id = wid - 1;

		count[id]++;

		if (count[id] > max)
			max = count[id];

	}

	int sum = 0;
	for ( int i = 0; i < wid; i++)
		sum += count[i];

	int avg = sum / wid;

	uchar * histogram = m_pHistogram->m_pImage;
	memset(histogram, 0, wid * hei * 3);

	for (int x = 0; x < wid; x++) {
		int max_id = int(float(count[x]) / float(avg) * float(hei)); 
		if (max_id > hei - 1)
			max_id = hei - 1;

		for (int y = hei - 1; y > hei -1 - max_id; y--) {
			int id = x + y * wid;
			histogram[id * 3 + 0] = 255;
			histogram[id * 3 + 1] = 255;
			histogram[id * 3 + 2] = 255;
		}
	}    
	delete count; 
#endif

	datafile.close();

}
#endif



void cxVolumeModel::ClearVolume()
{
	if (m_pVolume != NULL)
	{
		delete [] (float*) m_pVolume;
		m_pVolume = NULL;
	}

	if (m_pIndVolume != NULL)
	{
		delete [] (float*) m_pIndVolume;
		m_pIndVolume = NULL;
	}

}

void cxVolumeModel::ClearVolTex()
{
	if (m_nTexVol != 0) {
		glDeleteTextures(1, (const GLuint*)&m_nTexVol);
		m_nTexVol = 0;
	}

	if (m_nIndTexVol != 0) {
		glDeleteTextures(1, (const GLuint*)&m_nIndTexVol);
		m_nIndTexVol = 0;
	}
#ifdef SHOW_CLUSTER
	if (m_nTexClus != 0) {
		glDeleteTextures(1, (const GLuint*)&m_nTexClus);
		m_nTexClus = 0;
	}
#endif

#ifdef SV_CORRELATION
	if (m_nTexCorr != 0) {
		glDeleteTextures(1, (const GLuint*)&m_nTexCorr);
		m_nTexCorr = 0;
	}
#endif

	if (m_nTexVolPearson != 0) {
		glDeleteTextures(1, (const GLuint*)&m_nTexVolPearson);
		m_nTexVolPearson = 0;
	}
}

void cxVolumeModel::ClearVolTexPearson()
{
	if (m_nTexVolPearson != 0) {
		glDeleteTextures(1, (const GLuint*)&m_nTexVolPearson);
		m_nTexVolPearson = 0;
	}
}

void cxVolumeModel::InitVolume()
{
	extern string cfgPath;
	const char *cfgpath = cfgPath.c_str();
	char volumerender_CFG[100] = "";
	strcat(volumerender_CFG,cfgpath);
	strcat(volumerender_CFG,"volumerender.cfg");
	//read parameters from file
	cxParameters parameters;
	parameters.ParseFile(volumerender_CFG);
	m_sFilename = parameters.GetInputFile(m_nVolumeID); 
	m_nStartTime = parameters.m_nStartTime;
	m_nEndTime = parameters.m_nEndTime;
	m_nCurTime = m_nStartTime;  

	m_pVisStatus->ReadFromFile();
}

void cxVolumeModel::CreateDataTexPearson()
{
	//#ifdef PEARSONCPU
	if(m_nTexVolPearson==0)
	{
		glGenTextures(1, (GLuint*)&m_nTexVolPearson);
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D,m_nTexVolPearson);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// GPU data loading

		if (m_pVolume == NULL) {
			cerr << "The Volume is empty " << endl;
			return;
		}
#ifdef CUDA_CORR
		glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE16, (int)m_vSize[0],
				(int)m_vSize[1], (int)m_vSize[2], 0, GL_LUMINANCE, GL_FLOAT,
				//(int)m_vSize[1], (int)m_vSize[2], 0, GL_LUMINANCE, GL_FLOAT,
				m_pVolume);
#else
		glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE16, (int)m_vSize[0],
				(int)m_vSize[1], (int)m_vSize[2]*m_nNumTimePearson, 0, GL_LUMINANCE, GL_FLOAT,
				//(int)m_vSize[1], (int)m_vSize[2], 0, GL_LUMINANCE, GL_FLOAT,
				m_pVolume);
#endif
		glDisable(GL_TEXTURE_3D);
	}else{

		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D, m_nTexVolPearson);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glDisable(GL_TEXTURE_3D);
	}
	//#endif //PEARSONCPU
}


void cxVolumeModel::CreateDataTex()
{
	if(m_nTexVol==0)
	{
		glGenTextures(1, (GLuint*)&m_nTexVol);
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D,m_nTexVol);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		if (m_pVolume == NULL) {
			cerr << "The Volume is empty " << endl;
			return;
		}

		glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE16, (int)m_vSize[0],
				(int)m_vSize[1], (int)m_vSize[2], 0, GL_LUMINANCE, GL_FLOAT,
				m_pVolume);

		glDisable(GL_TEXTURE_3D);
	}else{

		glActiveTexture( GL_TEXTURE0);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D, m_nTexVol);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glDisable(GL_TEXTURE_3D);
	}
}

#ifdef SHOW_CLUSTER
void cxVolumeModel::CreateClusterTex()
{

	if(m_nTexClus==0 && volumeView->m_pClusView && volumeView->m_pClusView->isInit())
	{
		int dimx = (int)m_vSize[0]/volumeView->m_pClusView->m_xblock;
		int dimy = (int)m_vSize[1]/volumeView->m_pClusView->m_yblock;
		int dimz = (int)m_vSize[2]/volumeView->m_pClusView->m_zblock;
		int dimb =  dimx*dimy*dimz;
		int real_size = m_vSize[0]* m_vSize[1] * m_vSize[2];
		static float *ClusterMap = new float[real_size];
		int** clusdata = volumeView->m_pClusView->getClusterVol();
		int cluste_item_count = 0;
		int n = 0;

		float val;
		int count = 0;
		bool validtimestep = (volumeView->m_pClusView->getDim() <= m_time_in) ? false : true;
		int ind = m_time_in / volumeView->m_pClusView->m_ts_chunk;

		int xblocksize = volumeView->m_pClusView->m_xblock;
		int yblocksize = volumeView->m_pClusView->m_yblock;
		int zblocksize = volumeView->m_pClusView->m_zblock;
		for(int x1=0; x1 < m_vSize[0]-xblocksize+1; x1+=xblocksize){
			for(int y1=0; y1 < m_vSize[1]-yblocksize+1; y1+=yblocksize){
				for(int z1=0; z1 < m_vSize[2]-zblocksize+1; z1+=zblocksize){
					if (volumeView->m_pClusView->currCluster() != -1 && 
							validtimestep && clusdata[ind][count] != volumeView->m_pClusView->currCluster())
						val = 0.0;
					else
						val = 1.0;
					for(int x = x1; x< (x1+xblocksize); x++){
						for(int y = y1; y<(y1+yblocksize); y++){
							for(int z = z1; z<(z1+zblocksize); z++){
								int id = x + y * m_vSize[0] + z * m_vSize[0] * m_vSize[1];
								ClusterMap[id] = val;
							}
						}
					}
					count++;

				}
			}
		}
		/*
		   for (int i = 0; i < dimb; i++)
		   {
		   if (clusdata[i] != m_pClusView->currCluster()){ 
		   for(int j = 0; j < m_pClusView->m_xblock; j++){
		   ClusterMap[n+j] = 0.0;
		   }
		   }
		   else{
		   for(int j = 0; j < m_pClusView->m_xblock; j++){
		   ClusterMap[n+j] = 1.0;
		   }
		   cluste_item_count++;
		   }
		   n+=m_pClusView->m_xblock;
		   }
		   */
		glGenTextures(1, (GLuint*)&m_nTexClus);
		glActiveTexture(GL_TEXTURE7);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D,m_nTexClus);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);    

		if (m_pVolume == NULL) {
			cerr << "The Volume is empty " << endl;
			return;
		}
		/*
		   glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE16,
		   (int)m_vSize[0]/m_pClusView->m_xblock,
		   (int)m_vSize[1]/m_pClusView->m_yblock,
		   (int)m_vSize[2]/m_pClusView->m_zblock, 0, GL_LUMINANCE, GL_FLOAT,
		   ClusterMap);
		   */
		glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE16,
				(int)m_vSize[0],
				(int)m_vSize[1],
				(int)m_vSize[2], 0, GL_LUMINANCE, GL_FLOAT,
				ClusterMap);
		glDisable(GL_TEXTURE_3D);
	}else{

		glActiveTexture( GL_TEXTURE7);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D, m_nTexClus);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);    
		glDisable(GL_TEXTURE_3D);
	}
}
#endif

#ifdef SV_CORRELATION
void cxVolumeModel::CreateCorrTex()
{

	if(m_bcorr && m_corrmap){

		glGenTextures(1, (GLuint*)&m_nTexCorr);
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D,m_nTexCorr);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);    

		if (m_pVolume == NULL) {
			cerr << "The Volume is empty " << endl;
			return;
		}

		glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE16,
				(int)m_vSize[0],
				(int)m_vSize[1],
				(int)m_vSize[2], 0, GL_LUMINANCE, GL_FLOAT,
				m_corrmap);
		glDisable(GL_TEXTURE_3D);
	}else{

		glActiveTexture( GL_TEXTURE1);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D, m_nTexCorr);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);    
		glDisable(GL_TEXTURE_3D);
	}
}
#endif

void cxVolumeModel::CreateIndexDataTex()
{
	if(m_nIndTexVol==0)
	{
		glGenTextures(1, (GLuint*)&m_nIndTexVol);
		glActiveTexture(GL_TEXTURE6);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D,m_nIndTexVol);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		if (m_pVolume == NULL) {
			cerr << "The Volume is empty " << endl;
			return;
		}
		glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE16, (int)m_vSize[0],
				(int)m_vSize[1], (int)m_vSize[2], 0, GL_LUMINANCE, GL_FLOAT,
				m_pIndVolume);

		glDisable(GL_TEXTURE_3D);
	}else{

		glActiveTexture( GL_TEXTURE6);
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D, m_nIndTexVol);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glDisable(GL_TEXTURE_3D);

	}
}

void cxVolumeModel::InitProgram()
{
	extern string cfgPath;
	const char *cfgpath = cfgPath.c_str();
	char volpath[1024] = "";
	strcat(volpath,cfgpath);
	strcat(volpath,"myVOL");
	GLchar *VertexShaderSource, *FragmentShaderSource;
	readShaderSource(volpath, &VertexShaderSource, &FragmentShaderSource);
	int success = installShaders(VertexShaderSource, FragmentShaderSource, (GLuint*)&(m_nVolProgram));        
	if ( !success)
	{
		fprintf(stderr, "Error: Can not install shaders.\n");   
	}
	free(VertexShaderSource);
	free(FragmentShaderSource);

	m_bReload = false;
}

void cxVolumeModel::CreateRayDirTex()
{    
	if (m_nTexBack == 0)
		glGenTextures(1, (GLuint*)&m_nTexBack);

	int width = volumeView->m_pVisView->w();
	int height = volumeView->m_pVisView->h();



	//Back face    
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	volumeView->DrawCube();
	glActiveTexture( GL_TEXTURE1 );    
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_nTexBack);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);    
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB , 0, 0, width, height, 0);    

	//     int precision;    
	//     glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &precision);    
	//     cout << "precision " << precision << endl;

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void cxVolumeModel::CreateTfTex()
{
	if (m_nTexTF == 0)
		glGenTextures(1, (GLuint*)&m_nTexTF);


	glActiveTextureARB( GL_TEXTURE2_ARB );
	glEnable(GL_TEXTURE_1D);
	glBindTexture(GL_TEXTURE_1D, m_nTexTF);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);    

	static float * spacingAdjustedTF = new float[TF_SIZE*4];

	for(int i = 0;i < TF_SIZE; i++)
	{
		float alpha = m_pVisStatus->m_Colors[i].a();

		if (volumeView->m_pVisView->m_bOperating)
			alpha = 1 - pow((1-alpha), 0.01f / m_fNormalSpacing );
		else
			alpha = 1 - pow((1-alpha), m_pVisStatus->m_fSampleSpacing / m_fNormalSpacing );

		spacingAdjustedTF[i*4+0] = m_pVisStatus->m_Colors[i].r() * alpha;
		spacingAdjustedTF[i*4+1] = m_pVisStatus->m_Colors[i].g() * alpha;
		spacingAdjustedTF[i*4+2] = m_pVisStatus->m_Colors[i].b() * alpha;
		spacingAdjustedTF[i*4+3] = alpha;
	}

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA_FLOAT16_ATI, TF_SIZE, 0, GL_RGBA, GL_FLOAT, spacingAdjustedTF);

	glDisable(GL_TEXTURE_1D);

}

void cxVolumeModel::SaveTAC_vol()
{
	//SaveTACSubSet(  m_selectx1,m_selectx2,m_selecty1,m_selecty2,m_selectz1,m_selectz2, m_TAC_start, m_TAC_finish);
}


#define X_BLOCK_SIZE	 3
#define Y_BLOCK_SIZE	 3
#define Z_BLOCK_SIZE	 3
void cxVolumeModel::SaveGTAC_vol()
{
	if(!m_netcdf)
		return;
	NetCFD_var* var_info = m_netcdf->get_varinfo(m_curnetcdf_var);
	Filter* fltr = new Filter();
	fltr->m_dim = 4;
	NetCDFUtils* datautils = new NetCDFUtils(m_netcdf, fltr);
	char filename[MAX_PATH] = "c://jeffs/temp/test.dat";
	datautils->Save_volumeTACs(	0, m_vSize[0]-1, 
			0, m_vSize[1]-1,
			0, m_vSize[2]-1,
			var_info->var_name, 
			//m_TAC_start, m_TAC_finish,
			0, 30,/*m_timelen,*/ 
			X_BLOCK_SIZE, Y_BLOCK_SIZE, Z_BLOCK_SIZE, filename);


	int nparams = 11;
	char** params;
	params = new char*[12];
	for(int i=0; i < nparams; i++)
		params[i] = new char[128];
	strcpy(params[0],"");
	strcpy(params[1],"-t");			// algorithm def: 0
	strcpy(params[2],"0");
	strcpy(params[3],"-tch");		// number of timestamps used in calculating KMeans def: 0 (All timestamps)
	strcpy(params[4],"10");
	strcpy(params[5],"-tac");		// saved tac filename
	strcpy(params[6],filename);
	strcpy(params[7],"-k");			// number of cluster centers def: 4
	strcpy(params[8],"3");
	strcpy(params[9],"-s");		    // number of stages def: 1000
	strcpy(params[10],"300");
	//main_cluster(nparams, params);

	for(int i=0; i < nparams; i++)
		delete [] params[i];
	delete [] params;

	delete datautils;
	delete fltr;


}


void cxVolumeModel::SaveCustomTAC_vol(char* filename, int start, int count)
{
	if(!m_netcdf)
		return;
	NetCFD_var* var_info = m_netcdf->get_varinfo(m_curnetcdf_var);
	Filter* fltr = new Filter();
	fltr->m_dim = 4;
	NetCDFUtils* datautils = new NetCDFUtils(m_netcdf, fltr);

	datautils->Save_volumeTACs(	0, m_vSize[0]-1, 
			0, m_vSize[1]-1,
			0, m_vSize[2]-1,
			var_info->var_name, 
			start, count,
			X_BLOCK_SIZE, Y_BLOCK_SIZE, Z_BLOCK_SIZE,
			filename);
	delete datautils;
	delete fltr;
}

#ifdef CUDA_CORR
bool cxVolumeModel::vartominmax(int var, float* pdata, float* arr_fMin, 
						   float* arr_fMax, int var_index, int timesteps, int sizevol)
{
	float min, max;
	int i = var_index;
	int start = 0;//i*timesteps*sizevol;
	int end = timesteps*sizevol;//(i+1)*timesteps*sizevol;
	bool bRet = true;
	if (var == DATA_HO2) {
		min = 0;
		//max = 0.000343621;
		max = 0.000316;
	}
	else if (var == DATA_OH) {
		min = 0;
		max = 0.0207485;
	}
	else if (var == DATA_CHI) {
		//min = 0;
		//max = 12.2035;
		min = -2.0;
		//max = 0.75;
		max = 0.5;
	}
	else if (var == DATA_TEMP) {
		min = 10.0f;
		max = 30.0f;
		//min = 100.0f;
		//max = -100.0f;
		//printf("TEMP i=%d, start=%d, end=%d\n",i,start,end);

	}
	else if (var == DATA_SALT) {
		min = 30.0f;
		max = 37.0f;
		//min = 100.0f;
		//max = -100.0f;
		//min = max = pdata[start];
		//printf("SALT i=%d, start=%d, end=%d\n",i,start,end);


		//printf("min = %f, max = %f\n", min, max);
	}
	else{
		if(arr_fMin[i] < arr_fMax[i]){
			min = arr_fMin[i];
			max = arr_fMax[i];
		}
		else{
			min = max = pdata[start];
			//for (int i = 1; i < m_nVolumeSize*m_nNumTimePearson; i++) {
			for (int k = start; k < end; k++) {
				//for (int i = 1; i < m_nVolumeSize; i++) {
				if (min > pdata[k])
					min = pdata[k];
				if (max < pdata[k])
					max = pdata[k];
			}
			cout << "min " << min << " max " << max << endl;
			arr_fMin[i] = min;
			arr_fMax[i] = max;
		}
	}

	if(arr_fMin[i] < arr_fMax[i]){
		min = arr_fMin[i];
		max = arr_fMax[i];
	}
	else{
		arr_fMin[i] = min;
		arr_fMax[i] = max;
	}
	for (int k = start; k < end; k++) {
		if ( min > pdata[k] )
			pdata[k] = min;
		if ( max < pdata[k])
			pdata[k] = max;
	}
	return bRet;
}

void cxVolumeModel::nametovar(char* name, int & var)
{
	if (strcmp(name,"mixfrac") == 0)
		var = DATA_MIXFRAC;
	else if (strcmp(name,"HO2") == 0)
		var = DATA_HO2;
	else if (strcmp(name,"OH") == 0)
		var = DATA_OH;
	else if (strcmp(name,"chi") == 0)
		var = DATA_CHI;
	else if (strcmp(name,"temp") == 0)
		var = DATA_TEMP;
	else if (strcmp(name,"salt") == 0)
		var = DATA_SALT;
	else if (strcmp(name,"temp_a") == 0)
		var = DATA_TEMP_A;
	else if (strcmp(name,"salt_a") == 0)
		var = DATA_SALT_A;
	else if (strcmp(name,"wt") == 0)
		var = DATA_WT;
	else
		var = DATA_UNKNOWN;
}

void cxVolumeModel::rhtypetotype(nc_type rht, int & type)
{
	if ( rht == NC_FLOAT) {
		type = VOLUME_TYPE_FLOAT;
	} else {
		assert(0); // some other data type
		type = VOLUME_TYPE_FLOAT;
	}
}

void printDevProp(cudaDeviceProp devProp)
{
    printf("Major revision number:         %d\n",  devProp.major);
    printf("Minor revision number:         %d\n",  devProp.minor);
    printf("Name:                          %s\n",  devProp.name);
    printf("Total global memory:           %u\n",  devProp.totalGlobalMem);
    printf("Total shared memory per block: %u\n",  devProp.sharedMemPerBlock);
    printf("Total registers per block:     %d\n",  devProp.regsPerBlock);
    printf("Warp size:                     %d\n",  devProp.warpSize);
    printf("Maximum memory pitch:          %u\n",  devProp.memPitch);
    printf("Maximum threads per block:     %d\n",  devProp.maxThreadsPerBlock);
    for (int i = 0; i < 3; ++i)
    	printf("Maximum dimension %d of block:  %d\n", i, devProp.maxThreadsDim[i]);
    for (int i = 0; i < 3; ++i)
    	printf("Maximum dimension %d of grid:   %d\n", i, devProp.maxGridSize[i]);
    printf("Clock rate:                    %d\n",  devProp.clockRate);
    printf("Total constant memory:         %u\n",  devProp.totalConstMem);
    printf("Texture alignment:             %u\n",  devProp.textureAlignment);
    printf("Concurrent copy and execution: %s\n",  (devProp.deviceOverlap ? "Yes" : "No"));
    printf("Number of multiprocessors:     %d\n",  devProp.multiProcessorCount);
    printf("Kernel execution timeout:      %s\n",  (devProp.kernelExecTimeoutEnabled ? "Yes" : "No"));
    return;
}

uint getCUDA_globalmemsize(int & threads_per_block)
{
    // Number of CUDA devices
    int devCount;
    cudaGetDeviceCount(&devCount);
    printf("CUDA Device Query...\n");
    printf("There are %d CUDA devices.\n", devCount);
 
    // Iterate through devices
    int i = 0;
    //for (int i = 0; i < devCount; ++i)
    //{
    // Get device properties
    printf("\nCUDA Device #%d\n", i);
    cudaDeviceProp devProp;
    cudaGetDeviceProperties(&devProp, i);
    //printDevProp(devProp);
    //}
    threads_per_block = devProp.maxThreadsPerBlock;
    return devProp.totalGlobalMem;
}


bool cxVolumeModel::calculate_corr(const char* filename, string var_names[],  //requested variable names
						   int num_vars)			//number of variables requested (no more than 2)
{
	float* points = NULL;
	int max_threads_per_block = 0;
	uint CUDAmemsize = 0;
	bool bLowMem_MAIN = false;
	bool bLowMem_CUDA = false;
	//*************************************************************************
	//m_netcdf, m_time_in, m_nNumTimePearson, m_nVolumeSize
	// need a method to figure out which path to follow in-core or out-of-core
	//*************************************************************************
	float arr_fMin[2];
	float arr_fMax[2];
	arr_fMin[0] = m_fMin1;
	arr_fMin[1] = m_fMin2;
	arr_fMax[0] = m_fMax1;
	arr_fMax[1] = m_fMax2;

	bool bUseInCore = true;
	int xlocation, ylocation, zlocation;
	xlocation = (int)(volumeView->m_corrx * (float)(m_vSize[0]-1));
	ylocation = (int)(volumeView->m_corry * (float)(m_vSize[1]-1));
	zlocation = (int)(volumeView->m_corrz * (float)(m_vSize[2]-1));

	//*************************************************************************
	//in-core
	//*************************************************************************
	if(num_vars == 1){ //self-correlation
		//*************************************************************************
		//loop through spatial grid points
		//*************************************************************************
		int refid = xlocation + ylocation * m_vSize[0] + zlocation * m_vSize[0] * m_vSize[1];

		//*************************************************************************
		//check maximum available MAIN memory
		//*************************************************************************
		if(bUseInCore){
			points = new float[m_nNumTimePearson*m_nVolumeSize];
			if(!points){
				bUseInCore = false; // out of core due to shortage of main memory
				bLowMem_MAIN = true;
			}
		}

		//*************************************************************************
		//check CUDA device properties including maximum available memory
		//*************************************************************************
		CUDAmemsize = getCUDA_globalmemsize(max_threads_per_block);
		CUDAmemsize = 128304000;
		if(CUDAmemsize < m_nNumTimePearson*m_nVolumeSize*sizeof(float)){
			bUseInCore = false; // out of core due to shortage of CUDA memory
			bLowMem_CUDA = true;
		}


		if(bUseInCore){
			float* refpt = new float[m_nNumTimePearson];

			//*************************************************************************
			//read all TACs from all grid locations at once
			//this will not be possible for large number of timesteps (in that case
			//use out-of-core path.
			//*************************************************************************
			m_netcdf->resettachandle(filename, var_names[0].c_str(), m_total_timesteps);
			bUseInCore = m_netcdf->readtac(filename, var_names[0].c_str(), refpt, 
								refid,	refid, 
								m_total_timesteps, m_nNumTimePearson);
			bUseInCore = m_netcdf->readtac(filename, var_names[0].c_str(), points, 
								0,	m_nVolumeSize-1,
								m_total_timesteps, m_nNumTimePearson);
			
			//*************************************************************************
			//make sure that the data lies between min and max
			//*************************************************************************
			NetCFD_var* var_info = m_netcdf->get_varinfo(var_names[0].c_str());
			nametovar(var_info->var_name,m_nVariable);
			rhtypetotype(var_info->rh_type,m_nVolumeType);
			vartominmax(m_nVolumeType, points, arr_fMin, arr_fMax, 0, m_nNumTimePearson,m_nVolumeSize);
			vartominmax(m_nVolumeType, refpt, arr_fMin, arr_fMax, 0, m_nNumTimePearson,1);

			if(bUseInCore){
				//CUDA Host code
				// run the device part of the program
				runTest1((int)m_nNumTimePearson, m_nVolumeSize, points, refpt, m_pVolume, max_threads_per_block);
				delete [] refpt;
				delete [] points;
			}
			
			/*
			cerr << endl;
			for(int i=0; i<100;i++){
				cerr << m_pVolume[i] << ",";
			}
			cerr << endl;
			*/
			//*************************************************************************
			//need to have reference point
			//int current = 0;
			//while(current < m_time_in + m_nNumTimePearson){
			//m_netcdf->readtac()
			//TS_USED_IN_CORREL_CALC
			//}
			//*************************************************************************
		}
	
		//*************************************************************************
		//out-of-core (handle CUDA low memory case for now)
		//*************************************************************************
		if(!bUseInCore && !bLowMem_MAIN){
			float* refpt = new float[m_nNumTimePearson];
	
			//*************************************************************************
			//read all TACs from all grid locations at once
			//this will not be possible for large number of timesteps (in that case
			//use out-of-core path.
			//*************************************************************************
			m_netcdf->resettachandle(filename, var_names[0].c_str(), m_total_timesteps);
			bUseInCore = m_netcdf->readtac(filename, var_names[0].c_str(), refpt, 
								refid,	refid, 
								m_total_timesteps, m_nNumTimePearson);


			//*************************************************************************
			//loop through the volume piece by piece until all correlations get calculated
			//increment the main memory pointer as we go along
			//*************************************************************************
			
					
			bUseInCore = m_netcdf->readtac(filename, var_names[0].c_str(), points, 
								0,	m_nVolumeSize-1,
								m_total_timesteps, m_nNumTimePearson);
				
			//*************************************************************************
			//make sure that the data lies between min and max
			//*************************************************************************
			NetCFD_var* var_info = m_netcdf->get_varinfo(var_names[0].c_str());
			nametovar(var_info->var_name,m_nVariable);
			rhtypetotype(var_info->rh_type,m_nVolumeType);
			vartominmax(m_nVolumeType, points, arr_fMin, arr_fMax, 0, m_nNumTimePearson,m_nVolumeSize);
			vartominmax(m_nVolumeType, refpt, arr_fMin, arr_fMax, 0, m_nNumTimePearson,1);
			
			//find the suitable memory partitioning
			int div = 2;
			int new_size = m_nVolumeSize / div;  //TODO make sure you handle odd case as well as even 
			while(CUDAmemsize < new_size*m_nNumTimePearson*sizeof(float)){
				div++;
				new_size = m_nVolumeSize / div;
			}
			int curr_size = 0;
			int curr_start = 0;
			int curr_end = new_size-1;
			while(curr_size < m_nVolumeSize){ 
				//CUDA Host code
				// run the device part of the program
				if((curr_size + new_size) > m_nVolumeSize )
					new_size = m_nVolumeSize - curr_size;
				runTest1((int)m_nNumTimePearson, new_size, points+curr_size*m_total_timesteps, refpt, m_pVolume+curr_size, max_threads_per_block);
				curr_size += new_size;
			}
			delete [] refpt;
			delete [] points;
		}
	}
	return true;
}
#endif