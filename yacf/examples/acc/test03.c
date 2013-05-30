
#include <stdio.h>


#define N 10

int main () {
    int i, size;
    double * a, * b;

    a = malloc(N * sizeof(double));
    b = malloc(N * sizeof(double));
    size = N;

    #pragma acc kernels loop private(i) copy(a[size],b[size])
    for (i = 0; i < size; i++) {
            a[i] = i;
            b[i] = a[i];
    }

    // Check results
    for (i = 0; i < size; i++) {
             // Despite the fact that a and b are double precision arrays
             // we use the inequality instead of a relative comparison
             // because they are supposed to be identical
             if (a[i] != b[i])  {
                printf("a not equal to b at %d", i);
                return 1;
             }
    }
    return 0;
}
