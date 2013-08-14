/* Checks that two different regions on the same file works without problems 
*/
int main (int argc, int **argv[]) {
    int i;
    int * a;
    int * b;
    a = (int *) malloc(100 * sizeof(int));
    b = (int *) malloc(100 * sizeof(int));

    #pragma acc parallel loop num_gangs(256) num_workers(128) copyout(a[0:100]) 
        for (i = 0; i < 100; i++)
            a[i] = i;

    #pragma acc kernels loop  copyout(b[0:100]) 
        for (i = 0; i < 100; i++)
            b[i] = i;


    // Check result
    for (i = 0; i < 100; i++) {
        if (a[i] != b[i]) {
            printf("Error: pos %d not correct \n");
            return 1;
        }
    }
    printf("OK\n");
    return 0;
}
