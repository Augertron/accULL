

int main () {
    int i,j;
    int * a;
    a = (int *) malloc(100 * sizeof(int));

    #pragma acc data copyout(a[0:100])
    {
        j = i;
        #pragma acc kernels loop 
        for (i = 0; i < 100; i++)
            a[i] = i;
    }

    // Check
    for (i = 0; i < 100; i++) {
        if (a[i] != i) {
            printf("Error: pos %d not correct\n");
            return 1;
        }
    }
    return 0;
}
