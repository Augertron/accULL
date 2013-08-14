int main() {
    int cols = 2;
    double z[10];
    double p[10];
    double q[10];
    double alpha = 4.0;
    int i;
    #pragma omp target device(mpi)
    #pragma omp parallel for 
    for (i = 0; i < cols; i++) {
            z[i] += alpha*p[i];
    }
    
    #pragma omp target device(mpi)
    #pragma omp parallel for
    for (i = 0; i < cols; i++) {
            z[i] += alpha*q[i];
    }


    printf("Hello World!");
}

