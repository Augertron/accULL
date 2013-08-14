#define TYPE int
 
int main() {
    int ppn = 100;
    int i = 0;
    TYPE * a;

    a = malloc(ppn * sizeof(TYPE));

    #pragma acc parallel copyout(a[0:ppn])
    for (i=0;i<ppn;i++)
        a[i] = i;

    if ((a[1] != 1) || (a[ppn-1] != ppn-1)) {
        printf("Error a[1] = %d   a[ppn-1] = %d\n", a[1], a[ppn-1]);
        free(a);
        return 1;
    }

    printf("OK\n");

    free(a);

    return 0;
}
