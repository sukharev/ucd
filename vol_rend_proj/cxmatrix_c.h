//
// C Interface: cxmatrix_c
//
// Description: 
//
//
// Author: Hongfeng Yu <hfyu@ucdavis.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef CXMATRIX_C_H
#define CXMATRIX_C_H


#define MAT_TIME_VEC(matrix, in, out) \
((out)[0] = ((in)[0] * matrix[0] + (in)[1] * matrix[1] + (in)[2] * matrix[2] + matrix[3]), \
 (out)[1] = ((in)[0] * matrix[4] + (in)[1] * matrix[5] + (in)[2] * matrix[6] + matrix[7]), \
 (out)[2] = ((in)[0] * matrix[8] + (in)[1] * matrix[9] + (in)[2] * matrix[10] + matrix[11]))
 
 #define VEC_TIME_MAT(matrix, in, out) \
((out)[0] = ((in)[0] * matrix[0] + (in)[1] * matrix[4] + (in)[2] * matrix[8] + matrix[12]), \
 (out)[1] = ((in)[0] * matrix[1] + (in)[1] * matrix[5] + (in)[2] * matrix[9] + matrix[13]), \
 (out)[2] = ((in)[0] * matrix[2] + (in)[1] * matrix[6] + (in)[2] * matrix[10] + matrix[14]))
 
int inverse_mat(const double a[16], double b[16]);

#endif
