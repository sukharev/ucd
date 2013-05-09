/*
 * Copyright 1993-2007 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and
 * international Copyright laws.  Users and possessors of this source code
 * are hereby granted a nonexclusive, royalty-free license to use this code
 * in individual and commercial software.
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION,  ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOURCE CODE.
 *
 * U.S. Government End Users.   This source code is a "commercial item" as
 * that term is defined at  48 C.F.R. 2.101 (OCT 1995), consisting  of
 * "commercial computer  software"  and "commercial computer software
 * documentation" as such terms are  used in 48 C.F.R. 12.212 (SEPT 1995)
 * and is provided to the U.S. Government only as a commercial end item.
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
 * source code with only those rights set forth herein.
 *
 * Any use of this source code in individual and commercial software must
 * include, in the user documentation and internal comments to the code,
 * the above Disclaimer and U.S. Government End Users Notice.
 */

/* Example of integrating CUDA functions into an existing 
 * application / framework.
 * Device code.
 */

#ifndef _CPP_INTEGRATION_KERNEL_H_
#define _CPP_INTEGRATION_KERNEL_H_
#include "matrixMul.h"

#define CHECK_BANK_CONFLICTS 0
#if CHECK_BANK_CONFLICTS
#define AS(i, j) CUT_BANK_CHECKER(((float*)&As[0][0]), (BLOCK_SIZE * i + j))
#define BS(i, j) CUT_BANK_CHECKER(((float*)&Bs[0][0]), (BLOCK_SIZE * i + j))
#else
#define AS(i, j) As[i][j]
#define BS(i, j) Bs[i][j]
#endif

///////////////////////////////////////////////////////////////////////////////
//! Simple test kernel for device functionality
//! @param g_odata  memory to process (in and out)
///////////////////////////////////////////////////////////////////////////////
__global__ void
kernel( int* g_data )
{
    // write data to global memory
    const unsigned int tid = threadIdx.x;
    int data = g_data[tid];

    // use integer arithmetic to process all four bytes with one thread
    // this serializes the execution, but is the simplest solutions to avoid 
    // bank conflicts for this very low number of threads
    // in general it is more efficient to process each byte by a separate thread,
    // to avoid bank conflicts the access pattern should be 
    // g_data[4 * wtid + wid], where wtid is the thread id within the half warp 
    // and wid is the warp id
    // see also the programming guide for a more in depth discussion.
    g_data[tid] = ((((data <<  0) >> 24) - 10) << 24)
                | ((((data <<  8) >> 24) - 10) << 16)
                | ((((data << 16) >> 24) - 10) <<  8)
                | ((((data << 24) >> 24) - 10) <<  0);
}

///////////////////////////////////////////////////////////////////////////////
//! Demonstration that int2 data can be used in the cpp code
//! @param g_odata  memory to process (in and out)
///////////////////////////////////////////////////////////////////////////////
__global__ void
kernel2( int2* g_data )
{
    // write data to global memory
    const unsigned int tid = threadIdx.x;
    int2 data = g_data[tid];

    // use integer arithmetic to process all four bytes with one thread
    // this serializes the execution, but is the simplest solutions to avoid 
    // bank conflicts for this very low number of threads
    // in general it is more efficient to process each byte by a separate thread,
    // to avoid bank conflicts the access pattern should be 
    // g_data[4 * wtid + wid], where wtid is the thread id within the half warp 
    // and wid is the warp id
    // see also the programming guide for a more in depth discussion.
    g_data[tid].x = data.x - data.y;
}

// Device code
__global__ void VecAdd(int N, float* A, float* B, float* C)
{
	int i = threadIdx.x;
	if (i < N)
	C[i] = A[i] + B[i];
}


__global__ void
corr_cuda( float* C, float* A, float* B, int length, int num /* legnth of vector */)
{
    // Block index
    int x = blockIdx.x * blockDim.x + threadIdx.x;
	if (x >= num){
		return;
	}

	int a=0, b=0;
	float oneoversize = 1.0/length;
	
	

	// a faster version?
	float sum_sq_x, sum_sq_y, sum_coproduct, sweep, i_flt, delta_x, delta_y, mean_x, mean_y;
	float pop_sd_x, pop_sd_y, cov_x_y, p;

	sum_sq_x = 0.0;
	sum_sq_y = 0.0;
	sum_coproduct = 0.0;

	mean_x = A[x*length];
	mean_y = B[0];
	i_flt = (float)2.0;


	// Index of the first vector of A processed by the block
    int aBegin = x*length;
    // Index of the last vector of A processed by the block
    int aEnd   = aBegin + length - 1;
    for (a = aBegin, b = 0;
             a <= aEnd;
             a += 1, b += 1) {
		sweep = ( i_flt - (float)1.0 ) / i_flt;
		delta_x = A[a] - mean_x;
		delta_y = B[b] - mean_y;
		sum_sq_x += delta_x * delta_x * sweep;
		sum_sq_y += delta_y * delta_y * sweep;
		sum_coproduct += delta_x * delta_y * sweep;
		mean_x += delta_x / i_flt;
		mean_y += delta_y / i_flt;
		i_flt += (float)1.0;
	}

	pop_sd_x = sqrt ( sum_sq_x * oneoversize );
	pop_sd_y = sqrt ( sum_sq_y * oneoversize );
	cov_x_y = sum_coproduct * oneoversize;
	
	if ( pop_sd_x * pop_sd_y == 0.0 )
	{
		C[x] = dummy;
	}
	else
	{
		p = cov_x_y / ( pop_sd_x * pop_sd_y );
		C[x] = p;
	}
	
	float min = -1.0;
	float max = 1.0;
	float value = (C[x] - min) / (max - min);
	if(C[x]>max)
		C[x] = 1.0;
	else if(C[x] < min)
		C[x] = 0.0;
	else
		C[x] = value;

/*
    for (int a = aBegin, b = 0;
             a <= aEnd;
             a += 1, b += 1) {
		temp += A[a] -  B[b];
	}
	C[x]=temp;
*/
}

/*
> !--------------------------------------------------------
> ! Number of good (non-missing) data points in the index.
> let idx_ngd = idx_sel[l=@ngd]
> 
> ! Mask the plot variable in time, so that it has exactly the same
> ! missing time points as the anchor index.
> let pvar_mask = IF 1+0*idx_sel THEN pvar_sel
> 
> ! Average of the good data points.
> let idx_ave = idx_sel[l=@sum]/idx_ngd
> let pvar_ave = pvar_mask[l=@sum]/pvar_mask[l=@ngd]
> 
> ! Time series of deviations from the average.
> let idxdev = idx_sel - idx_ave
> let pvardev = pvar_mask - pvar_ave
> 
> ! Time series of cross-products.
> let crossprod = idxdev*pvardev
> 
> ! Time series of squares.
> let idxdev2 = idxdev^2
> let pvardev2 = pvardev^2
> 
> ! Standard deviations.
> let idxstd = (idxdev2[l=@sum]/idx_ngd)^.5
> let pvarstd = (pvardev2[l=@sum]/pvar_mask[l=@ngd])^.5
> 
> ! Covariance, correlation, and regression.
> let cov = crossprod[l=@sum]/crossprod[l=@ngd]
> let corr =  cov/(idxstd*pvarstd)
> let regr =  cov/idxstd^2
> !--------------------------------------------------------
*/

/*
vec4 getPearsonCoefColorLocation ( vec3 texPosition, bool vors, bool bval)
{
	//referenceY = 1 - referenceY;
	texPosition.x = texPosition.x + offsetX;
	if ( texPosition.x<0.0 ) texPosition.x = texPosition.x + 1.0;
	if ( texPosition.x>1.0 ) texPosition.x = texPosition.x - 1.0;

	int i, size = 6;//72
	float oneoversize = 1.0/6.0;//72
	float A[6];//72];
	float B[6];//72]

	float x, y, z;
	float a, zpos, refzpos;
	float zposcp;
	vec3 col;

	if ( grid )
	{
		x = texture1D ( gridTexX, texPosition.x ).r;
		y = texture1D ( gridTexY, texPosition.y ).r;
		z = texPosition.z;
		//z = texture1D(gridTexZ, texPosition.z).r;
	}
	else
	{
		x = texPosition.x;
		y = texPosition.y;
		z = texPosition.z;
	}

	//a = texture3D(volumeTexPearson, vec3(x,y,z)).r;
	//a = 0.05;
	a = 0.01;


	// a faster version?
	float sum_sq_x, sum_sq_y, sum_coproduct, sweep, iflt, delta_x, delta_y, mean_x, mean_y;
	float pop_sd_x, pop_sd_y, cov_x_y, p;

	sum_sq_x = 0.0;
	sum_sq_y = 0.0;
	sum_coproduct = 0.0;

	zpos = z * oneoversize;
	zposcp = zpos;
	refzpos = referenceZ * oneoversize;

	A[0] = lookUpValue ( texture3D ( volumeTex, vec3 ( x,y,zpos ) ).r );
	B[0] = lookUpValue ( texture3D ( volumeTex, vec3 ( referenceX, referenceY, refzpos ) ).r );

	mean_x = A[0];
	mean_y = B[0];
	zpos += oneoversize;
	refzpos += oneoversize;
	iflt = 2.0;


	for ( i=1; i<size; i++ )
	{
		sweep = ( iflt - 1.0 ) / iflt;
		A[i] = lookUpValue ( texture3D ( volumeTex, vec3 ( x,y,zpos ) ).r );
		B[i] = lookUpValue ( texture3D ( volumeTex, vec3 ( referenceX, referenceY, refzpos ) ).r );
		delta_x = A[i] - mean_x;
		delta_y = B[i] - mean_y;
		sum_sq_x += delta_x * delta_x * sweep;
		sum_sq_y += delta_y * delta_y * sweep;
		sum_coproduct += delta_x * delta_y * sweep;
		mean_x += delta_x / iflt;
		mean_y += delta_y / iflt;
		zpos += oneoversize;
		refzpos += oneoversize;
		iflt += 1.0;
	}

	pop_sd_x = sqrt ( sum_sq_x * oneoversize );
	pop_sd_y = sqrt ( sum_sq_y * oneoversize );
	cov_x_y = sum_coproduct * oneoversize;
	if ( pop_sd_x * pop_sd_y == 0.0 )
	{
		return vec4 ( 0.0, 0.0, 0.0, 0.0 );
	}
	else
	{
		p = cov_x_y / ( pop_sd_x * pop_sd_y );
		if(bval){
			return vec4(p, 0.0, 0.0, 0.0);
		}
		col = getCorrelColor ( p, false, vors );
		return vec4 ( col, a );
	}

}
*/




#endif // #ifndef _CPP_INTEGRATION_KERNEL_H_
