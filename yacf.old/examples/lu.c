#define CUDA_NUM_THREADS 64

#define K_LOOP_SIZE (size - 1)

#ifndef LLC_TRANSLATION
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>
#endif

double M[4096 * 4096];

;
double L[4096 * 4096];
;
double M2[4096 * 4096];
;


void lu(int nthreads)
{
    int size;
    int i;
    int j;
    int k;
    for (i = 0; i < size; i++) {
            for (j = 0; j < size; j++) {
                    if (i == j)
                            M[(j * 4096) + i] = 1.0;
                    else
                            M[(j * 4096) + i] = (1.0 + (i * size)) + j;
                    L[(j * 4096) + i] = 0.0;
            }
    }

    ;
    time_start = t();



   for (k = 0; k < K_LOOP_SIZE; k++)  {
		  #pragma omp target device(cuda) copy_in(M) copy_out(L, M)
        #pragma omp parallel for shared(M, L, size, k) private(i, j) schedule(static, 1)
        for (i = k + 1; i < size; i++) {
            L[(k * 4096) + i] = M[(k * 4096) + i] / M[(k * 4096) + k];
                for (j = k + 1; j < size; j++) {
                    M[(j * 4096) + i] = M[(j * 4096) + i] - (L[(k * 4096) + i] * M[(j * 4096) + k]);
                }
        }
    }

    ;
    time_end = t() - time_start;
    printf("** Lu OpenMP time %g \n", time_end);
}


int main() {
   lu(1);
}
