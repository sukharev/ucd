/***************************************************************************
                          cxparameters.cpp  -  description
                             -------------------
    begin                : Thu Jul 28 2005
    copyright            : (C) 2005 by Hongfeng Yu
    email                : hfyu@ucdavis.edu
 ***************************************************************************/
 
#include "cxparameters.h"
#include <stdlib.h>

cxParameters::cxParameters()
{
	m_nStartTime = 0;
	m_nEndTime = 0;
}



void cxParameters::ParseFile(char * filename)
{
	ifstream inf(filename);
	if(!inf)
	{
		cerr << "Error: can not find the configuration file." << endl;
		exit(0);
	}

	string term;
	while(getline(inf, term))
	{
#ifndef MULTI_VARIABLES   
		if(term == "[Input Data File]")
		{
			inf >>  term;
            m_szInputFiles[0] = term;         
			getline(inf, term);
		}
#else
        if(term == "[Input Data File 0]")
        {
            inf >>  term;
            m_szInputFiles[0] = term;
            getline(inf, term);
        }
        if(term == "[Input Data File 1]")
        {
            inf >>  term;
            m_szInputFiles[1] = term;
            getline(inf, term);
        }
        if(term == "[Input Data File 2]")
        {
            inf >>  term;
            m_szInputFiles[2] = term;
            getline(inf, term);
        }
        if(term == "[Input Data File 3]")
        {
            inf >>  term;
            m_szInputFiles[3] = term;
            getline(inf, term);
        }
        if(term == "[Input Data File 4]")
        {
            inf >>  term;
            m_szInputFiles[4] = term;
            getline(inf, term);
        }              
#endif
        if(term == "[Start Timestep]")
		{
			inf >> m_nStartTime;
			getline(inf, term);
		}
		
        if(term == "[End Timestep]")
		{
			inf >> m_nEndTime;
			getline(inf, term);
		}
	}//end of while
}

