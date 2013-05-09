//
// C++ Interface: cxcolor
//
// Description: 
//
//
// Author: Hongfeng Yu <hfyu@ucdavis.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

/**
	@author Hongfeng Yu <hfyu@ucdavis.edu>
*/


#ifndef CXCOLOR_H
#define CXCOLOR_H

#include <iostream>
#include <cmath>

using namespace std;

#define PI 3.14159265358979323846264338327950

class cxColor
{
public:
	cxColor(float r=0, float g=0, float b=0, float a=0){ color[0]=r; color[1]=g; color[2]=b; color[3]=a;}
	
	const cxColor & operator = ( const cxColor& color);	
	bool operator == (const cxColor & right) const;
	bool operator != (const cxColor & right) const;
	float& operator [](int i);
	
	int interpolate(cxColor a,cxColor b, const float fact);
	
	float& r(){return color[0];}
	float& g(){return color[1];}
	float& b(){return color[2];}
	float& a(){return color[3];}
	
	void convert2HSI(cxColor &hsi);
	void convert2RGB(cxColor &rgb);
	
private:
	float color[4];
};

#endif
