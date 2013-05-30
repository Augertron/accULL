
int main () {
    double red;
    int i;

    red = 0;

    #pragma acc kernels 
    #pragma acc loop  reduction(+ : red)
        for (i = 0; i < 100; i++)
            red += 1.0f;

    // Check results
    if (red != 100) {
        printf("Error: Red should be %d but it is %d ", 100, red);
        return 1;
    }
    return 0;
}
