//----------------------------------------------------------------------
//	File:		kmlsample.cpp
//	Programmer:	David Mount
//	Last modified:	05/14/04
//	Description:	Sample program for kmeans
//----------------------------------------------------------------------
// Copyright (C) 2004-2005 David M. Mount and University of Maryland
// All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at
// your option) any later version.  See the file Copyright.txt in the
// main directory.
//
// The University of Maryland and the authors make no representations
// about the suitability or fitness of this software for any purpose.
// It is provided "as is" without express or implied warranty.
//----------------------------------------------------------------------

#include <cstdlib>			// C standard includes
#include <iostream>			// C++ I/O
#include <string.h>			// C++ strings
#include "KMlocal.h"			// k-means algorithms

#include <sys/types.h>
#include <sys/timeb.h>

using namespace std;			// make std:: available

//----------------------------------------------------------------------
// kmlsample
//
// This is a simple sample program for the kmeans local search on each
// of the four methods.  After compiling, it can be run as follows.
//
//   kmlsample [-d dim] [-k nctrs] [-max mpts] [-df data] [-s stages]
//
// where
//	dim		Dimension of the space (default = 2)
//	nctrs		The number of centers (default = 4)
//	mpts		Maximum number of data points (default = 1000)
//	data		File containing data points
//			(If omitted mpts points are randomly generated.)
//	stages		Number of stages to run (default = 100)
//
// Results are sent to the standard output.
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//  Global entry points
//----------------------------------------------------------------------
int getArgs(int argc, char **argv);	// get command-line arguments

void printSummary(			// print final summary
    const KMlocal&	theAlg,		// the algorithm
    const KMdata&	dataPts,	// the points
    KMfilterCenters&	ctrs);		// the centers

bool readPt(				// read a point
    istream&		in,		// input stream
    KMpoint&		p);		// point (returned)

void printPt(				// print a point
    ostream&		out,		// output stream
    const KMpoint&	p);		// the point

bool readTACFileHeader(ifstream& inf,
						int *p_xsize,
						int *p_ysize,
						int *p_zsize,
						int *p_time_count);

bool readTACFileHeader1(ifstream& inf,
						int *p_xsize,
						int *p_ysize,
						int *p_zsize,
						int *p_time_count);


bool readTACData(	ifstream& inf, 
					KMdata& dataPts, 
					int nPts, 
					int ntime_steps);

struct ClusterCentroid
{
	float val;
	int ind;
};

bool writeClusterData(ofstream& outf, 
					  KMfilterCenters& ctrs, 
					  const KMdata&	dataPts, 
					  int ntime_steps, 
					  ClusterCentroid* first, ClusterCentroid* last);
bool testClusterData( KMfilterCenters& ctrs, 
					  const KMdata&	dataPts, 
					  int ntime_steps, 
					  ClusterCentroid* first, ClusterCentroid* last);

bool writeClusterHeader(ofstream& outf,
						int chunk_size,
						int num_clusters);

int insertionSort(ClusterCentroid* A, int len);
int compare( const void *arg1, const void *arg2 );

//----------------------------------------------------------------------
//  Global parameters (some are set in getArgs())
//----------------------------------------------------------------------
int tch     = 0;        // use all timesteps in calculating KMeans (use only 1 chunk)
int alg     = 0;        // use Lloyd by default
int	k		= 4;		// number of centers
int	dim		= 2;		// dimension
int	maxPts		= 1000000; //100;		// max number of data points
int	stages		= 1000;		// number of stages
istream* dataIn		= NULL;		// input data stream
ifstream* tacdataIn = NULL;
ofstream* tacdataOut = NULL;

//----------------------------------------------------------------------
//  Termination conditions
//	These are explained in the file KMterm.h and KMlocal.h.  Unless
//	you are into fine tuning, don't worry about changing these.
//----------------------------------------------------------------------
KMterm	term(100, 0, 0, 0,		// run for 100 stages
		0.10,			// min consec RDL
		0.10,			// min accum RDL
		3,			// max run stages
		0.50,			// init. prob. of acceptance
		10,			// temp. run length
		0.95);			// temp. reduction factor

FILE* fpdeb;
//----------------------------------------------------------------------
//  Main program
//----------------------------------------------------------------------

#ifndef STANDALONE
int main_cluster(int argc, char **argv)
{
#ifdef DEBUG_INFO
	fpdeb = fopen("c://test.txt","w");
	if(!fpdeb)
		return -1;
#endif
    if(getArgs(argc, argv)<0)			// read command-line arguments
		return -1;
	term.setAbsMaxTotStage(stages);		// set number of stages

    //KMdata dataPts(dim, maxPts);		// allocate data storage
    int nPts = 0;				// actual number of points

	ClusterCentroid* first = NULL; 
	ClusterCentroid *last = NULL;
	ClusterCentroid *prev_last =NULL;

    int xsize,ysize,zsize,time_count;
    if(readTACFileHeader1(*tacdataIn, &xsize, &ysize, &zsize, &time_count)){
		dim = time_count;
		nPts = xsize * ysize * zsize;
		
		writeClusterHeader(*tacdataOut, tch, k);

		int count = 0;
		while(count < dim){
			KMdata dataPts(tch, maxPts);
			dataPts.resize(tch, nPts);
			readTACData(*tacdataIn,dataPts,nPts,tch);


	

			cout << "Data Points:\n";			// echo data points
			//for (int i = 0; i < nPts; i++)
			//printPt(cout, dataPts[i]);

			dataPts.setNPts(nPts);			// set actual number of pts
			dataPts.buildKcTree();			// build filtering structure

			KMfilterCenters ctrs(k, dataPts);		// allocate centers


			// run each of the algorithms
			switch(alg){
				case 0:
					{
					cout << "\nExecuting Clustering Algorithm: Lloyd's\n";
					KMlocalLloyds kmLloyds(ctrs, term);		// repeated Lloyd's
					ctrs = kmLloyds.execute();			// execute
					//printSummary(kmLloyds, dataPts, ctrs);	// print summary			
					break;
					}
				case 1:
					{
					cout << "\nExecuting Clustering Algorithm: Swap\n";
					KMlocalSwap kmSwap(ctrs, term);		// Swap heuristic
					ctrs = kmSwap.execute();
					//printSummary(kmSwap, dataPts, ctrs);
					break;
					}
				case 2:
					{
					cout << "\nExecuting Clustering Algorithm: EZ-Hybrid\n";
					KMlocalEZ_Hybrid kmEZ_Hybrid(ctrs, term);	// EZ-Hybrid heuristic
					ctrs = kmEZ_Hybrid.execute();
					//printSummary(kmEZ_Hybrid, dataPts, ctrs);
					break;
					}
				case 3:
					{
					cout << "\nExecuting Clustering Algorithm: Hybrid\n";
					KMlocalHybrid kmHybrid(ctrs, term);		// Hybrid heuristic
					ctrs = kmHybrid.execute();
					//printSummary(kmHybrid, dataPts, ctrs);
					break;
					}
				default:
					// we should not get here
					break;
			}

			if(first)
				delete [] first;

			first = new ClusterCentroid[k];
			last = new ClusterCentroid[k];
			testClusterData(ctrs, dataPts, tch, first, last);
			writeClusterData(*tacdataOut, ctrs, dataPts, tch, first, last);

			if(prev_last)
				delete [] prev_last;

			prev_last = last;

			cout << "\ncount = " << count << endl;
			count+=tch;
		}
	}
	if(first)
		delete [] first;
	if(prev_last)
		delete [] prev_last;

	if(tacdataIn)
		tacdataIn->close();
	if(tacdataOut)
		tacdataOut->close();
#ifdef DEBUG_INFO
	fclose(fpdeb);
#endif    
	
	return 0;
}

#else

int main(int argc, char **argv)
{
#ifdef DEBUG_INFO
	fpdeb = fopen("c://test.txt","w");
	if(!fpdeb)
		return -1;
#endif
    if(getArgs(argc, argv)<0)			// read command-line arguments
		return -1;
	term.setAbsMaxTotStage(stages);		// set number of stages

    //KMdata dataPts(dim, maxPts);		// allocate data storage
    int nPts = 0;				// actual number of points

	ClusterCentroid* first = NULL; 
	ClusterCentroid *last = NULL;
	ClusterCentroid *prev_last =NULL;

    int xsize,ysize,zsize,time_count;
    if(readTACFileHeader1(*tacdataIn, &xsize, &ysize, &zsize, &time_count)){
		dim = time_count;
		nPts = xsize * ysize * zsize;
		
		writeClusterHeader(*tacdataOut, tch, k);

		int count = 0;
		//___________________________________________
		 char tmpbuf[128], ampm[] = "AM";
		__time64_t ltime;
		struct __timeb64 tstruct;
		struct tm *today;

		/* Set time zone from TZ environment variable. If TZ is not set,
		 * the operating system is queried to obtain the default value 
		 * for the variable. 
		 */
		_tzset();

		/* Get UNIX-style time and display as number and string. */
		_time64( &ltime );
		//printf( "Time in seconds since UTC 1/1/70:\t%ld\n", ltime );
		//printf( "UNIX time and date:\t\t\t%s", _ctime64( &ltime ) );

		/* Convert to time structure and adjust for PM if necessary. */
		today = _localtime64( &ltime );
		if( today->tm_hour >= 12 )
		{
		strcpy( ampm, "PM" );
		today->tm_hour -= 12;
		}
		if( today->tm_hour == 0 )  /* Adjust if midnight hour. */
		today->tm_hour = 12;

		/* Note how pointer addition is used to skip the first 11 
		 * characters and printf is used to trim off terminating 
		 * characters.
		 */
		printf( "12-hour time:\t\t\t\t%.8s %s\n",
		   asctime( today ) + 11, ampm );
		//___________________________________________

		while(count < dim){
			KMdata dataPts(tch, maxPts);
			dataPts.resize(tch, nPts);
			readTACData(*tacdataIn,dataPts,nPts,tch);


	

			cout << "Data Points:\n";			// echo data points
			//for (int i = 0; i < nPts; i++)
			//printPt(cout, dataPts[i]);

			dataPts.setNPts(nPts);			// set actual number of pts
			dataPts.buildKcTree();			// build filtering structure

			KMfilterCenters ctrs(k, dataPts);		// allocate centers


			// run each of the algorithms
			switch(alg){
				case 0:
					{
					cout << "\nExecuting Clustering Algorithm: Lloyd's\n";
					KMlocalLloyds kmLloyds(ctrs, term);		// repeated Lloyd's
					ctrs = kmLloyds.execute();			// execute
					//printSummary(kmLloyds, dataPts, ctrs);	// print summary			
					break;
					}
				case 1:
					{
					cout << "\nExecuting Clustering Algorithm: Swap\n";
					KMlocalSwap kmSwap(ctrs, term);		// Swap heuristic
					ctrs = kmSwap.execute();
					//printSummary(kmSwap, dataPts, ctrs);
					break;
					}
				case 2:
					{
					cout << "\nExecuting Clustering Algorithm: EZ-Hybrid\n";
					KMlocalEZ_Hybrid kmEZ_Hybrid(ctrs, term);	// EZ-Hybrid heuristic
					ctrs = kmEZ_Hybrid.execute();
					//printSummary(kmEZ_Hybrid, dataPts, ctrs);
					break;
					}
				case 3:
					{
					cout << "\nExecuting Clustering Algorithm: Hybrid\n";
					KMlocalHybrid kmHybrid(ctrs, term);		// Hybrid heuristic
					ctrs = kmHybrid.execute();
					//printSummary(kmHybrid, dataPts, ctrs);
					break;
					}
				default:
					// we should not get here
					break;
			}

			if(first)
				delete [] first;

			first = new ClusterCentroid[k];
			last = new ClusterCentroid[k];
			testClusterData(ctrs, dataPts, tch, first, last);
			writeClusterData(*tacdataOut, ctrs, dataPts, tch, first, last);

			if(prev_last)
				delete [] prev_last;

			prev_last = last;

			cout << "\ncount = " << count << endl;
			count+=tch;
		}

			//___________________________________________
			 char tmpbuf1[128];
			__time64_t ltime1;
			struct __timeb64 tstruct1;
			struct tm *today1;

			/* Set time zone from TZ environment variable. If TZ is not set,
			 * the operating system is queried to obtain the default value 
			 * for the variable. 
			 */
			_tzset();

			/* Get UNIX-style time and display as number and string. */
			_time64( &ltime1 );
			//printf( "Time in seconds since UTC 1/1/70:\t%ld\n", ltime );
			//printf( "UNIX time and date:\t\t\t%s", _ctime64( &ltime ) );

			/* Convert to time structure and adjust for PM if necessary. */
			today1 = _localtime64( &ltime1 );
			if( today1->tm_hour >= 12 )
			{
		   strcpy( ampm, "PM" );
		   today1->tm_hour -= 12;
			}
			if( today1->tm_hour == 0 )  /* Adjust if midnight hour. */
		   today1->tm_hour = 12;

			/* Note how pointer addition is used to skip the first 11 
			 * characters and printf is used to trim off terminating 
			 * characters.
			 */
			printf( "12-hour time:\t\t\t\t%.8s %s\n",
			   asctime( today1 ) + 11, ampm );
			//___________________________________________

	}
	if(first)
		delete [] first;
	if(prev_last)
		delete [] prev_last;

#ifdef DEBUG_INFO
	fclose(fpdeb);
#endif    
	return 0;
}
#endif //STANDALONE


//----------------------------------------------------------------------
//  getArgs - get command line arguments
//----------------------------------------------------------------------

int getArgs(int argc, char **argv)
{
    static ifstream dataStream;			// data file stream
    static ifstream queryStream;		// query file stream
	static ofstream resultStream;

    if (argc <= 1) {				// no arguments
  	cerr << "Usage:\n\n"
        << "   kmlsample [-d dim] [-k nctrs] [-t number][-max mpts] [-df data] [-tac data][-s stages]\n"
        << "\n"
        << " where\n"
        << "    dim             Dimension of the space (default = 2)\n"
        << "    nctrs           The number of centers (default = 4)\n"
		<< "    t               Name the algortithm by number 0 to 3\n"
		<< "					where 0-Lloyd,1-Swap,2-EZ-Hybrid,3-Hybrid\n"
		<< "    tch             The number of time steps in each time block - default: all 0\n"
        << "    mpts            Maximum number of data points (default = 1000)\n"
        << "    data            File containing data points\n"
	<< "                    (If omitted mpts points are randomly generated.)\n"
        << "    stages          Number of stages to run (default = 100)\n"
        << "\n"
        << " Results are sent to the standard output.\n"
	<< "\n"
	<< " The simplest way to run it is:\n"
	<< "    kmlsample -df data_pts\n"
	<< "  or\n"
	<< "    kmlsample -max 50\n";
	//kmExit(0);
    return -1;
	}
    int i = 1;
    while (i < argc) {				// read arguments
	if (!strcmp(argv[i], "-d")) {		// -d option
	    dim = atoi(argv[++i]);
	}
	else if (!strcmp(argv[i], "-k")) {	// -k option
	    k = atoi(argv[++i]);
	}
	else if (!strcmp(argv[i], "-t")) {	// -k option
	    alg = atoi(argv[++i]);
	}
	else if (!strcmp(argv[i], "-tch")) {	// -k option
	    tch = atoi(argv[++i]);
	}
	else if (!strcmp(argv[i], "-max")) {	// -max option
	    maxPts = atoi(argv[++i]);
	}
	else if (!strcmp(argv[i], "-df")) {	// -df option
		dataStream.open(argv[++i], ios::in);
	    if (!dataStream) {
		cerr << "Cannot open data file\n";
		return -1;//kmExit(1);
	    }
	
	    dataIn = &dataStream;
	}
	else if (!strcmp(argv[i], "-tac")) {	// -tac option
	    dataStream.open(argv[++i], ios::in | ios::binary);
		if (!dataStream) {
			cerr << "Cannot open tac data file\n";
			return -1;//kmExit(1);
		}
		tacdataIn = &dataStream;

		
		char filename_out[1024];
		strcpy(filename_out, argv[i]);
		strcat(filename_out, ".cluster");
		resultStream.open(filename_out, /*ios::out |*/ ios::binary);
		if (!resultStream) {
		cerr << "Cannot open data file\n";
		return -1;//kmExit(1);
	    }

		tacdataOut = &resultStream;
		
	}
	else if (!strcmp(argv[i], "-s")) {	// -s option
	    stages = atoi(argv[++i]);
	}
	else {					// illegal syntax
	    cerr << "Unrecognized option.\n";
	    return -1;//kmExit(1);
	}
	i++;
    }
	return 0;
}

//----------------------------------------------------------------------
//  Reading/Printing utilities
//	readPt - read a point from input stream into data storage
//		at position i.  Returns false on error or EOF.
//	printPt - prints a points to output file
//----------------------------------------------------------------------
bool readPt(istream& in, KMpoint& p)
{
    for (int d = 0; d < dim; d++) {
	if(!(in >> p[d])) return false;
    }
    return true;
}



void printPt(ostream& out, const KMpoint& p)
{
    out << "(" << p[0];
    for (int i = 1; i < dim; i++) {
	out << ", " << p[i];
    }
    out << ")\n";
}


//----------------------------------------------------------------------
//
//	readTACFileHeader - read header of TAC data file.
//----------------------------------------------------------------------

bool readTACFileHeader(ifstream& inf, int *p_xsize, int *p_ysize, int *p_zsize, int *p_time_count)
{

    //ifstream inf(filename, ios::binary);

    if (!inf)
        return false;

    int xstart, xend, ystart, yend, zstart, zend;
    int zsize, ysize, xsize;
    int total_size, time_out, time_in;
    int time_step;

    inf.read(reinterpret_cast<char *>(&xstart), sizeof(int));
    inf.read(reinterpret_cast<char *>(&xend), sizeof(int));
    inf.read(reinterpret_cast<char *>(&ystart), sizeof(int));
    inf.read(reinterpret_cast<char *>(&yend), sizeof(int));
    inf.read(reinterpret_cast<char *>(&zstart), sizeof(int));
    inf.read(reinterpret_cast<char *>(&zend), sizeof(int));
    inf.read(reinterpret_cast<char *>(&time_in), sizeof(int));
    inf.read(reinterpret_cast<char *>(&time_out), sizeof(int));
    xsize = xend - xstart + 1;
    ysize = yend - ystart + 1;
    zsize = zend - zstart + 1;
	assert(xsize >= 1 && ysize >= 1 && zsize >= 1 && time_out >= 1);

    total_size = xsize * ysize * zsize;
    //time_out = time_out + time_in - 1;
    time_in = 1;

    *p_xsize = xsize;
	*p_ysize = ysize;
	*p_zsize = zsize;
    *p_time_count = time_out;
    return true;
}

bool readTACFileHeader1(ifstream& inf, int *p_xsize, int *p_ysize, int *p_zsize, int *p_time_count)
{

    //ifstream inf(filename, ios::binary);

    if (!inf)
        return false;

    int xstart, xend, ystart, yend, zstart, zend;
    int zsize, ysize, xsize;
    int total_size, time_out, time_in;
    int time_step;

	int xblock,yblock,zblock;
    inf.read(reinterpret_cast<char *>(&xstart), sizeof(int));
    inf.read(reinterpret_cast<char *>(&xend), sizeof(int));
    inf.read(reinterpret_cast<char *>(&ystart), sizeof(int));
    inf.read(reinterpret_cast<char *>(&yend), sizeof(int));
    inf.read(reinterpret_cast<char *>(&zstart), sizeof(int));
    inf.read(reinterpret_cast<char *>(&zend), sizeof(int));
    inf.read(reinterpret_cast<char *>(&time_in), sizeof(int));
    inf.read(reinterpret_cast<char *>(&time_out), sizeof(int));
	inf.read(reinterpret_cast<char *>(&xblock), sizeof(int));
	inf.read(reinterpret_cast<char *>(&yblock), sizeof(int));
	inf.read(reinterpret_cast<char *>(&zblock), sizeof(int));
    xsize = xend - xstart + 1;
    ysize = yend - ystart + 1;
    zsize = zend - zstart + 1;
	xsize = xsize / xblock;
	ysize = ysize / yblock;
	zsize = zsize / zblock;
	
	assert(xsize >= 1 && ysize >= 1 && zsize >= 1 && time_out >= 1);

    total_size = xsize * ysize * zsize;
    //time_out = time_out + time_in - 1;
    time_in = 1;

    *p_xsize = xsize;
	*p_ysize = ysize;
	*p_zsize = zsize;
    *p_time_count = time_out;
    return true;
}

bool readTACData(ifstream& inf, KMdata& dataPts, int nPts, int ntime_steps)
{
	if (!inf)
        return false;

    float value;
    int i = 0;

	for (i=0; i<dim && i < ntime_steps; i++){
    	for(int j=0; j < nPts; j++){
			inf.read(reinterpret_cast<char *>(&value), sizeof(float));
			dataPts[j][i] = value;
			
		}

    }
    return true;
}

#ifdef DEBUG_INFO
void printctrs(ClusterCentroid* arr, const char* str)
{
	fprintf(fpdeb,"%s:\n",str);
	for (int j = 0; j < k; j++){
		fprintf(fpdeb,"arr[%d].val = %f\t",j,arr[j].val);
		fprintf(fpdeb,"arr[%d].ind = %d\n",j,arr[j].ind);
	}
	fprintf(fpdeb,"____________________\n");
}

#endif

bool testClusterData( KMfilterCenters& ctrs, 
					  const KMdata&	dataPts, 
					  int ntime_steps, 
					  ClusterCentroid* first, ClusterCentroid* last)
{
	
	KMctrIdxArray closeCtr = new KMctrIdx[dataPts.getNPts()];
    double* sqDist = new double[dataPts.getNPts()];
    ctrs.getAssignments(closeCtr, sqDist);

	// write out number of clusters
	for (int j = 0; j < k; j++){
		first[j].val = ctrs[j][0]; 
		first[j].ind = j; 
		last[j].val  = ctrs[j][ctrs.getDim() - 1];
		last[j].ind = j;
	}
#ifdef DEBUG_INFO
	printctrs(first,"before");
#endif
	insertionSort(first, k);
	//qsort( (void *)first, (size_t)k, sizeof( ClusterCentroid ), compare );
	//qsort( (void *)last, (size_t)k, sizeof( ClusterCentroid ), compare );
#ifdef DEBUG_INFO
	printctrs(first,"after");
#endif
	delete [] closeCtr;
    delete [] sqDist;
	
	return true;
}


bool writeClusterHeader(ofstream& outf,
						int chunk_size,
						int num_clusters)
{
	if (!outf)
        return false;
	outf.write(reinterpret_cast<char *>(&num_clusters), sizeof(int));
	outf.write(reinterpret_cast<char *>(&chunk_size), sizeof(int));
	return true;
}

bool writeClusterData(ofstream& outf, 
					  KMfilterCenters& ctrs, 
					  const KMdata&	dataPts, 
					  int ntime_steps, 
					  ClusterCentroid* first, ClusterCentroid* last)
{
	if (!outf)
        return false;

	FILE* fp = fopen("c:\\jeffs\\uc_davis\\results\\cluster_centroids.txt","a");
	assert(fp!=NULL);
	KMctrIdxArray closeCtr = new KMctrIdx[dataPts.getNPts()];
    double* sqDist = new double[dataPts.getNPts()];
    ctrs.getAssignments(closeCtr, sqDist);

	int* map = new int[k];
	int n = 0;
	for(int i=0; i < k; i++){
		map[first[i].ind] = i;  
	}
/*
	fprintf(fpdeb,"_ _ _ _ _ \n");
	for(int i = 0;i< k; i++){
		fprintf(fpdeb,"map[%d] = %d\n",i,map[i]);
	}
	fprintf(fpdeb,"_ _ _ _ _ \n");
*/

    for(int i=0; i < dataPts.getNPts(); i++){
		int val = map[closeCtr[i]];
		outf.write(reinterpret_cast<char *>(&val), sizeof(int));
	}
	delete [] map;

	// write out number of clusters
	//outf.write(reinterpret_cast<char *>(&k), sizeof(int));
	fprintf(fp,"next: (centroids) k=%d, ctrs.getDim=%d\n", k, ctrs.getDim());
	for (int j = 0; j < k; j++){
		for (int d = 0; d < ctrs.getDim(); d++) {
			float fval = ctrs[first[j].ind][d];
			outf.write(reinterpret_cast<char *>(&fval), sizeof(float));
			fprintf(fp, "%f\n",ctrs[first[j].ind][d]);
		}
	}
	

	delete [] closeCtr;
    delete [] sqDist;
	
	fclose(fp);
	return true;
}


//------------------------------------------------------------------------
//  Print summary of execution
//------------------------------------------------------------------------
void printSummary(
    const KMlocal&		theAlg,		// the algorithm
    const KMdata&		dataPts,	// the points
    KMfilterCenters&		ctrs)		// the centers
{
    cout << "Number of stages: " << theAlg.getTotalStages() << "\n";
    cout << "Average distortion: " <<
	         ctrs.getDist(false)/double(ctrs.getNPts()) << "\n";
    					// print final center points
    cout << "(Final Center Points:\n";
    ctrs.print();
    cout << ")\n";
    					// get/print final cluster assignments
    KMctrIdxArray closeCtr = new KMctrIdx[dataPts.getNPts()];
    double* sqDist = new double[dataPts.getNPts()];
    ctrs.getAssignments(closeCtr, sqDist);

    *kmOut	<< "(Cluster assignments:\n"
		<< "    Point  Center  Squared Dist\n"
		<< "    -----  ------  ------------\n";
    for (int i = 0; i < dataPts.getNPts(); i++) {
	*kmOut	<< "   " << setw(5) << i
		<< "   " << setw(5) << closeCtr[i]
		<< "   " << setw(10) << sqDist[i]
		<< "\n";
    }
    *kmOut << ")\n";
    delete [] closeCtr;
    delete [] sqDist;
}

/*
int compare( const void *arg1, const void *arg2 )
{
   // Compare all of both strings:
   float diff = (((ClusterCentroid*)arg1)->val - ((ClusterCentroid*)arg2)->val);
   if (diff = 0.0) return 0; // could be a problem?
   else if (diff > 0.0) return 1;
   else return -1;
}
*/

int insertionSort(ClusterCentroid* A, int len)
{
	ClusterCentroid value;
	int j;
	for(int i = 0; i < len; i++){
        value = A[i];
		j = i-1;
		while(j >= 0 && A[j].val > value.val){
            A[j + 1] = A[j];
            j = j-1;
		}
        A[j+1] = value;
	}
	return 1;
}