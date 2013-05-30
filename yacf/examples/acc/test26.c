#include <stdio.h>


#define N 100


int main () {
    int i, j, size, state = 0;
    int * a;
    int * a_h;
    int * b;
    int k;

    a = malloc(N * N * sizeof(int));
    a_h = malloc(N * N * sizeof(int));
    size = N;

    {
            for (i = 0; i < size; i++) {   
                for (j = 0; j < size; j++) {
                    a_h[i*size+j] = i*size+j;
                }
            }

            for (i = 0; i < size; i++) {   
                for (j = 0; j < size; j++) {
                    a_h[i*size+j] = i*size+j;
                }
            }
    }



    /* This should produce a kernel grid of 32x32   */
    #pragma acc parallel num_gangs(32)  num_workers(32) copy(a[0:size*size]) private(k)
    {

             k = 0;

            #pragma acc loop 
            for (i = 0; i < size; i++) {   
                for (j = 0; j < size; j++) {
                    a[i*size+j] = k;
                }
            }

            k = size;

          #pragma acc loop 
            for (i = 0; i < k; i++) {   
                for (j = 0; j < k; j++) {
                    a[i*k+j] = i*k+j;
                }
            }
    }

    for (i = 0; i < size; i++) 
	for (j = 0; j < size; j++) 
	      if (a_h[i*size+j] != a[i*size+j])
		  state = 1;


    free(a);
    free(a_h);
   
    if (state)
	printf(" a_h != a \n ");

    return state;
}
