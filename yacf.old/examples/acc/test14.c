
#include <stdio.h>


#define N 10

int main () {
    int size;
    double * a, * b;
    int i;

    a = malloc(N * sizeof(double));
    b = malloc(N * sizeof(double));
    size = N;
    
    for (i = 0; i < N; i++) {
       a[i] = i;
       b[i] = 0;
    }

    #pragma acc kernels loop copy(a[4:size],b[4:size]) 
    for ( i = 4; i < size; i++) {
            b[i] = a[i];
    }

    for (i = 0; i < N; i++) {
        if (i < 4) {
             if (b[i] != 0) { 
                #ifdef PRINT
                  printf(" First half incorrect ");
                #endif
                  return 1;
               }
        } else {
             if (b[i] != a[i]) {
                #ifdef PRINT
                  printf(" Second half incorrect %g != %g at %d ",b[i],a[i],i);
               #endif
                  return 1;
             }
       }
    }
   return 0;
}
