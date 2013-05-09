/***************************************************************************
                          cxparameters.h  -  description
                             -------------------
    begin                : Thu Jul 28 2005
    copyright            : (C) 2005 by Hongfeng Yu
    email                : hfyu@ucdavis.edu
 ***************************************************************************/
 
#ifndef CPARAMETERS_H
#define CPARAMETERS_H


/**
  *@author Hongfeng Yu
  */

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

#define MAX_VARIABLES   2

class cxParameters {
public:
	cxParameters();
	
	//Read and parse the configure file
	void ParseFile(char * filename);
    
    char* GetInputFile(int i = 0) {
        return (char*)m_szInputFiles[i].c_str();
    }

public:
	//the parameters from the file
    string m_szInputFiles[MAX_VARIABLES];   

	int m_nStartTime;
	int m_nEndTime;
};

#endif
