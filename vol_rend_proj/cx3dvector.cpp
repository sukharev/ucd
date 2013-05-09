//
// C++ Implementation: cx3DVector
//
// Description: 
//
//
// Author: Hongfeng Yu <hfyu@ucdavis.edu>, (C) 2006
//
// Copyright: All rights reserved.  May not be used, modified, or copied 
// without permission.
//
//

#include "cx3dvector.h"

cx3DVector::~cx3DVector(){
}

cx3DVector operator+ ( const cx3DVector& left, const cx3DVector & right){
	return cx3DVector(left.m_fX+right.m_fX, left.m_fY+right.m_fY, left.m_fZ+right.m_fZ);
}

cx3DVector operator- ( const cx3DVector& left, const cx3DVector & right){
	return cx3DVector(left.m_fX-right.m_fX, left.m_fY-right.m_fY, left.m_fZ-right.m_fZ);
}

cx3DVector operator* ( const cx3DVector& left, float right){
	return cx3DVector(left.m_fX*right, left.m_fY*right, left.m_fZ*right);
}

cx3DVector operator/ ( const cx3DVector& left, float right){
	return cx3DVector(left.m_fX/right, left.m_fY/right, left.m_fZ/right);
}

cx3DVector cx3DVector::normal()
{
	float r=len();
	if (r<0.00000001) 
		r=1.0;

	return cx3DVector(m_fX/r, m_fY/r, m_fZ/r);
}

void cx3DVector::normalize()
{
	float r=len();
	if (r<0.00000001) 
		r=1.0;

	m_fX /= r;
	m_fY /= r;
	m_fZ /= r;
}

const cx3DVector cx3DVector::CrossPuct(const cx3DVector & right)
{
	cx3DVector result;
	
	result[0] = m_fY * right.m_fZ - m_fZ * right.m_fY;
	result[1] = m_fZ * right.m_fX - m_fX * right.m_fZ;
	result[2] = m_fX * right.m_fY - m_fY * right.m_fX;
	
	return result;
}
