#include <stdio.h>
int main() {
    double z[10];
    int i;
    #pragma omp target device(mpi)
    #pragma omp parallel for private(i)
    #pragma llc result(&z[i], 1)
    for (i = 0; i < 2; i++) {
            z[i] = 4.0;
    }
    #pragma omp target device(mpi)
    #pragma omp parallel for default(shared)  // No puedo dejarlo sin clausulas por otro error ya reportado
    #pragma llc result(&z[i], 1)
    for (i = 0; i < 2; i++) {
            z[i] = 4.0;
    }
    printf("Hello World!");
}
