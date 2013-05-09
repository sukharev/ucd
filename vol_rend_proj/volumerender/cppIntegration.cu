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
 * Host part of the device code.
 * Compiled with Cuda compiler.
 */

// includes, system
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>
// includes, project
#include <cutil_inline.h>

// includes, kernels
#include <cppIntegration_kernel.cu>
//
////////////////////////////////////////////////////////////////////////////////
// declaration, forward

//extern "C" void
//computeGold(char* reference, char* idata, const unsigned int len);
//extern "C" void
//computeGold2(int2* reference, int2* idata, const unsigned int len);

extern "C"
void corr_cpu( float* C, /* out - correlations */
			  float* A, /* array of vectors stored in 1D array */
			  float* B, /* reference vector */
			  int length, /* length of vector */ 
			  int num /* number of vectors */);


// Host code
// N number of timesteps
// M volume size
extern "C" void
runTest1(int N, int M, float* h_A, float* h_B, float* h_C, int max_num_threads )
{
	// Allocate vectors in device memory
	float* h_Cref = new float[M];
	
	size_t sizeA = N * M * sizeof(float);
	size_t sizeB = N * 1 * sizeof(float);
	size_t sizeC = M * sizeof(float);
	
	float* d_A;
	CUDA_SAFE_CALL(cudaMalloc((void**)&d_A, sizeA));
	float* d_B;
	CUDA_SAFE_CALL(cudaMalloc((void**)&d_B, sizeB));
	float* d_C;
	CUDA_SAFE_CALL(cudaMalloc((void**)&d_C, sizeC));
	// Copy vectors from host memory to device memory
	// h_A and h_B are input vectors stored in host memory
	CUDA_SAFE_CALL(cudaMemcpy(d_A, h_A, sizeA, cudaMemcpyHostToDevice));
	CUDA_SAFE_CALL(cudaMemcpy(d_B, h_B, sizeB, cudaMemcpyHostToDevice));
	// Invoke kernel
	//int threadsPerBlock = 198;//256;
	//int blocksPerGrid = 3240;//(N + threadsPerBlock-1)/ threadsPerBlock;
	int threadsPerBlock = max_num_threads;
	int blocksPerGrid = (int)M/(int)threadsPerBlock + 2;

	//VecAdd<<<blocksPerGrid, threadsPerBlock>>>(N,d_A, d_B, d_C);
	corr_cuda<<<blocksPerGrid, threadsPerBlock>>>( d_C, d_A, d_B, N, M);
	
	// check if kernel execution generated and error
    CUT_CHECK_ERROR("Kernel execution failed");
    
    cudaThreadSynchronize();
    
    // Copy result from device memory to host memory
	// h_C contains the result in host memory
	CUDA_SAFE_CALL(cudaMemcpy(h_C, d_C, sizeC, cudaMemcpyDeviceToHost));
	
	bool success = true;
	/*
    // compute reference solutions
    corr_cpu( h_Cref, h_A, h_B, N, M);
	
	// check result
    for( int i = 0; i < M; i++ )
    {
        if( abs(h_Cref[i] - h_C[i]) > SMALL_ERR)
            success = false;
    }
    
    if(!success){
		for( int i = 0; i < M; i++ )
		{
			std::cout << h_Cref[i] << ", ";
		}
		std::cout << std::endl;
    }
   
    printf("Test %s\n", success ? "PASSED" : "FAILED");
    */ 
	// Free device memory
	CUDA_SAFE_CALL(cudaFree(d_A));
	CUDA_SAFE_CALL(cudaFree(d_B));
	CUDA_SAFE_CALL(cudaFree(d_C));
	
	delete [] h_Cref;
}