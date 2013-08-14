
int main() {
    int cols = 2;
    double z[10];
    double p[10];
    double alpha = 4.0;
    int i;
    #pragma omp parallel for private(i)
    #pragma llc nc_result (&z[i], 1, z[0])
    for (i = 0; i < cols; i++) {
            z[i] += alpha*p[i];
    }
    printf("Hello World!");
}
