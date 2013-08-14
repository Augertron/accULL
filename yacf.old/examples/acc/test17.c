

int main (int argc, int **argv[]) {
    int i;
    int * a;
    a = (int *) malloc(100 * sizeof(int));

    #pragma acc parallel loop num_gangs(256) num_workers(128) copyout(a[0:100]) 
        for (i = 0; i < 100; i++)
            a[i] = i;

    // Check result
    for (i = 0; i < 100; i++) {
        if (a[i] != i) {
            printf("Error: pos %d not correct for a\n");
            return 1;
        }
    }
    printf("OK\n");
    return 0;
}
