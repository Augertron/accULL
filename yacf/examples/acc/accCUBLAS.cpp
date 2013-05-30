/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
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

/* This example demonstrates how to use the CUBLAS library
 * by scaling an array of floating-point values on the device
 * and comparing the result to the same operation performed
 * on the host.
 */

/* Includes, system */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Includes, cuda */
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <shrQATest.h>

/* Matrix size */
#define N  (275)


#if 1

    #define RAND_MAX 9999999999999

/** Bool types are not defined ?? **/
    typedef unsigned bool;
    #define true 1 
   #define false 0
    #define EXIT_SUCCESS true
   #define EXIT_FAILURE false
    #define cudaSuccess 0


/**** remove me ****/
typedef enum{
    CUBLAS_STATUS_SUCCESS         =0,
    CUBLAS_STATUS_NOT_INITIALIZED =1,
    CUBLAS_STATUS_ALLOC_FAILED    =3,
    CUBLAS_STATUS_INVALID_VALUE   =7,
    CUBLAS_STATUS_ARCH_MISMATCH   =8,
    CUBLAS_STATUS_MAPPING_ERROR   =11,
    CUBLAS_STATUS_EXECUTION_FAILED=13,
    CUBLAS_STATUS_INTERNAL_ERROR  =14
} cublasStatus_t;

struct cublasContext;
typedef struct cublasContext *cublasHandle_t;

#define CUBLASAPI 

typedef enum {
    CUBLAS_FILL_MODE_LOWER=0, 
    CUBLAS_FILL_MODE_UPPER=1
} cublasFillMode_t;

typedef enum {
    CUBLAS_DIAG_NON_UNIT=0, 
    CUBLAS_DIAG_UNIT=1
} cublasDiagType_t; 

typedef enum {
    CUBLAS_SIDE_LEFT =0, 
    CUBLAS_SIDE_RIGHT=1
} cublasSideMode_t; 


typedef enum {
    CUBLAS_OP_N=0,  
    CUBLAS_OP_T=1,  
    CUBLAS_OP_C=2  
} cublasOperation_t;


typedef enum { 
    CUBLAS_POINTER_MODE_HOST   = 0,  
    CUBLAS_POINTER_MODE_DEVICE = 1        
} cublasPointerMode_t;



#define cublasSgemm          cublasSgemm_v2
cublasStatus_t CUBLASAPI cublasSgemm_v2 (cublasHandle_t handle, 
                                        cublasOperation_t transa,
                                        cublasOperation_t transb, 
                                        int m,
                                        int n,
                                        int k,
                                        const float *alpha, /* host or device pointer */  
                                        const float *A, 
                                        int lda,
                                        const float *B,
                                        int ldb, 
                                        const float *beta, /* host or device pointer */  
                                        float *C,
                                        int ldc);

#else
#include <cublas_v2.h>
#endif




/* Host implementation of a simple version of sgemm */
static void simple_sgemm(int n, float alpha, const float *A, const float *B,
                         float beta, float *C)
{
    int i;
    int j;
    int k;
    for (i = 0; i < n; ++i) {
        for (j = 0; j < n; ++j) {
            float prod = 0;
            for (k = 0; k < n; ++k) {
                prod += A[k * n + i] * B[j * n + k];
            }
            C[j * n + i] = alpha * prod + beta * C[j * n + i];
        }
    }
}



/* Main */
int main(int argc, char** argv)
{    
    cublasStatus_t status;
    float* h_A;
    float* h_B;
    float* h_C;
    float* h_C_ref;
    float alpha = 1.0f;
    float beta = 0.0f;
    int n2 = N * N;
    int i;
    float error_norm;
    float ref_norm;
    float diff;
    cublasHandle_t handle;

    /* shrQAStart(argc, argv);*/

    /* Initialize CUBLAS */
    printf("simpleCUBLAS test running..\n");

    status = cublasCreate(&handle);
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! CUBLAS initialization error\n");
        return EXIT_FAILURE;
    }

    /* Allocate host memory for the matrices */
    h_A = (float*)malloc(n2 * sizeof(h_A[0]));
    if (h_A == 0) {
        fprintf (stderr, "!!!! host memory allocation error (A)\n");
        return EXIT_FAILURE;
    }
    h_B = (float*)malloc(n2 * sizeof(h_B[0]));
    if (h_B == 0) {
        fprintf (stderr, "!!!! host memory allocation error (B)\n");
        return EXIT_FAILURE;
    }
    h_C = (float*)malloc(n2 * sizeof(h_C[0]));
    h_C_ref = (float*)malloc(n2 * sizeof(h_C_ref[0]));
    if (h_C == 0) {
        fprintf (stderr, "!!!! host memory allocation error (C)\n");
        return EXIT_FAILURE;
    }

    /* Fill the matrices with test data */
    for (i = 0; i < n2; i++) {
        h_A[i] = rand() / (float)RAND_MAX;
        h_B[i] = rand() / (float)RAND_MAX;
        h_C[i] = rand() / (float)RAND_MAX;
    }

           /* Performs operation using plain C code */
            simple_sgemm(N, alpha, h_A, h_B, beta, h_C);
//            h_C_ref = h_C;
            
    for (i = 0; i < n2; i++)
        h_C_ref[i] = h_C[i];

#pragma acc data copy(h_A[n2], h_B[n2], h_C[n2],alpha,beta) 
            {

                    if (acc_get_device_type() == acc_device_nvidia) {
                        #pragma acc host_data use_device(h_A, h_B, h_C)
                            /* Performs operation using cublas */
                            status = cublasSgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N, N, N, N, &alpha, h_A, N, h_B, N, &beta, h_C, N);
                            if (status != CUBLAS_STATUS_SUCCESS) {
                                    fprintf (stderr, "!!!! kernel execution error.\n");
                                    return EXIT_FAILURE;
                            } }
                    else {
                            /*** We cannot use CUBLAS */
                            fprintf(stderr, " Cannot use CUBLAS without NVIDIA device \n");
                            return EXIT_FAILURE;
                    }
            }

    /* Check result against reference */
    error_norm = 0;
    ref_norm = 0;
    for (i = 0; i < n2; ++i) {
        diff = h_C_ref[i] - h_C[i];
        error_norm += diff * diff;
        ref_norm += h_C_ref[i] * h_C_ref[i];
    }
    error_norm = (float)sqrt((double)error_norm);
    ref_norm = (float)sqrt((double)ref_norm);
    if (fabs(ref_norm) < 1e-7) {
        fprintf (stderr, "!!!! reference norm is 0\n");
        return EXIT_FAILURE;
    }

    /* Memory clean up */
    free(h_A);
    free(h_B);
    free(h_C);
    free(h_C_ref);

    /* Shutdown */
    status = cublasDestroy(handle);
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! shutdown error (A)\n");
        return EXIT_FAILURE;
    }
    if( (error_norm / ref_norm < 1e-6f) )
        printf("** Result OK \n");
    else
        printf("** Not OK \n");
    /* shrQAFinish(argc, (const char **)argv, (error_norm / ref_norm < 1e-6f) ? QA_PASSED : QA_FAILED );*/

    return EXIT_SUCCESS;
}
