//
// C++ Implementation: cxcolor
//
// Description: 
//
//
// Author: Hongfeng Yu <hfyu@ucdavis.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "cxcolor.h"
#include <assert.h>

const cxColor & cxColor::operator = ( const cxColor& right)
{
	if(&right != this)
	{
		color[0] = right.color[0];
		color[1] = right.color[1];
		color[2] = right.color[2];
		color[3] = right.color[3];
	}
		
	return * this;
}

bool cxColor::operator == (const cxColor & right) const
{
	if( color[0] == right.color[0] && color[1] == right.color[1] && color[2] == right.color[2] && color[3] == right.color[3])
		return true;

	return false;
}

bool cxColor::operator != (const cxColor & right) const
{
	if( color[0] != right.color[0] || color[1] != right.color[1] || color[2] != right.color[2] || color[3] != right.color[3])
		return true;

	return false;
}

float& cxColor::operator [](int i)
{
	assert ( i >= 0 && i < 4);
	return color[i];
}

int cxColor::interpolate(cxColor a,cxColor b,const float fact)
{
	if (fact<0.0 || fact>1.0)
		return 0;
	
	float f = 1.0-fact;
	
	color[0] = a[0]*fact+b[0]*f;
	color[1] = a[1]*fact+b[1]*f;
	color[2] = a[2]*fact+b[2]*f;
	color[3] = a[3]*fact+b[3]*f;
	
	return 1;
}

void cxColor::convert2HSI(cxColor &hsi)
{
	float t=color[0]+color[1]+color[2];
	if(t==0) { hsi[0]=hsi[1]=hsi[2]=0.0; return; }

	hsi[2]=t/3.0;

	if(color[0]==color[1] && color[1]==color[2]) {
		hsi[1]=hsi[0]=0.0;
		return;
	}

	float minrgb=color[0];
	if(color[1]<=color[0]&&color[1]<=color[2]) minrgb=color[1];
	else if(color[0]<=color[1]&&color[0]<=color[2]) minrgb=color[0];
	else if(color[2]<=color[0]&&color[2]<=color[1]) minrgb=color[2];
	hsi[1]=1-3.0*minrgb/t;

	hsi[0]=(2*color[0]-color[1]-color[2])/2.0;
	hsi[0]/=sqrt((color[0]-color[1])*(color[0]-color[1])+(color[0]-color[2])*(color[1]-color[2]));
	hsi[0]=acos(hsi[0]);

	if(color[2]>color[1]) hsi[0]=2*PI-hsi[0];

	hsi[0]/=2*PI;
}

void cxColor::convert2RGB(cxColor &rgb)
{
	float hue=color[0]*360.0;

	if(hue>0 && hue<=120) {
		rgb[0]=(1+((color[1]*cos(color[0]*2*PI))/cos(PI/3.0-color[0]*2*PI)))/3.0;
		rgb[2]=(1-color[1])/3.0;
		rgb[1]=1-rgb[0]-rgb[2];
		rgb[0]*=3*color[2];
		rgb[1]*=3*color[2];
		rgb[2]*=3*color[2];
	} else if(hue>120 && hue<=240) {
		hue=color[0]*2*PI-2.0*PI/3.0;
		rgb[1]=(1+((color[1]*cos(hue))/cos(PI/3.0-hue)))/3.0;
		rgb[0]=(1-color[1])/3.0;
		rgb[2]=1-rgb[0]-rgb[1];
		rgb[0]*=3*color[2];
		rgb[1]*=3*color[2];
		rgb[2]*=3*color[2];
	} else if((hue>240 && hue<=360) || hue==0) {
		hue=color[0]*2*PI-4.0*PI/3.0;
		rgb[2]=(1+((color[1]*cos(hue))/cos(PI/3.0-hue)))/3.0;
		rgb[1]=(1-color[1])/3.0;
		rgb[0]=1-rgb[1]-rgb[2];
		rgb[0]*=3*color[2];
		rgb[1]*=3*color[2];
		rgb[2]*=3*color[2];
	} else {
		cerr<<"Convertion Error!"<<endl;
		rgb[0]=rgb[1]=rgb[2]=0.0;
	}
}



