//
// C++ Interface: cx3DVector
//
// Description: 
//
//
// Author: Hongfeng Yu <hfyu@ucdavis.edu>, (C) 2006
//
// Copyright: All rights reserved.  May not be used, modified, or copied 
// without permission.
//

#ifndef cx3DVector_H
#define cx3DVector_H

#include <iostream>
#include <cmath>

using namespace std;


/**
  *@author Hongfeng Yu
  */

class cx3DVector 
{
public:
	cx3DVector(float x=0, float y=0, float z=0){m_fX=x; m_fY=y; m_fZ=z;}
    cx3DVector(double v[3]){m_fX=v[0]; m_fY=v[1]; m_fZ=v[2];}
    
	~cx3DVector();

	float len(){return sqrt(m_fX*m_fX + m_fY*m_fY + m_fZ * m_fZ);}
	cx3DVector normal();
	void normalize();
    float distance(const cx3DVector &right){
        return sqrt( (m_fX - right.m_fX) * (m_fX - right.m_fX) +
                     (m_fY - right.m_fY) * (m_fY - right.m_fY) +
                     (m_fZ - right.m_fZ) * (m_fZ - right.m_fZ));
    }
	
	bool operator==(const cx3DVector &right){return m_fX==right.m_fX && m_fY==right.m_fY && m_fZ==right.m_fZ;}
	bool operator!=(const cx3DVector &right){return m_fX!=right.m_fX || m_fY!=right.m_fY || m_fZ!=right.m_fZ;}

	float & operator[](const unsigned int index)
	{
		if(index == 0)
			return m_fX;
		if(index == 1)
			return m_fY;
		return m_fZ;
	}
	
	float & x(){return m_fX;}
	float & y(){return m_fY;}
	float & z(){return m_fZ;}
	
	const cx3DVector & operator=(const cx3DVector &right){m_fX=right.m_fX; m_fY=right.m_fY; m_fZ=right.m_fZ; return *this;}
	const cx3DVector & operator+= ( const cx3DVector &v ) { m_fX+=v.m_fX; m_fY+=v.m_fY; m_fZ+=v.m_fZ; return *this; }
	const cx3DVector & operator-= ( const cx3DVector &v ) { m_fX-=v.m_fX; m_fY-=v.m_fY; m_fZ-=v.m_fZ; return *this; }
	const cx3DVector & operator*= ( float s ) { m_fX*=s; m_fY*=s; m_fZ*=s; return *this; }
	const cx3DVector & operator/= ( float s ) { m_fX/=s; m_fY/=s; m_fZ/=s; return *this; }
	const cx3DVector CrossPuct(const cx3DVector & right);

	friend cx3DVector operator+ ( const cx3DVector& left, const cx3DVector & right);
	friend cx3DVector operator- ( const cx3DVector& left, const cx3DVector & right);
	friend cx3DVector operator* ( const cx3DVector& left, float right);
	friend cx3DVector operator/ ( const cx3DVector& left, float right);
	
	friend ostream & operator << ( ostream & os, cx3DVector & right)
	{
		os << "( " << right.m_fX << " " << right.m_fY << " " << right.m_fZ << ")";
		return os;
	}

private:
	float m_fX;
	float m_fY;
	float m_fZ;

};

#endif
