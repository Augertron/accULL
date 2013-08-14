#define TYPE int


void update(void) {
    for (int i = 0; i < 100; i++);
}

int main () {
    TYPE * a;
    int i, state = 0;
    int n = 100;
    a = malloc(sizeof(TYPE) * n);
    a = memset(a, 0, sizeof(TYPE) * n);

    #pragma acc kernels copyin(a[0:n])
    {
        #pragma acc loop
        for (i = 0; i < n; i++)
            a[i] = 1;

       #pragma acc update host(a[0:n])

       #pragma acc update device(a[0:n])
    }

    update();

        for (i = 0; (i < n); i++) {
            if (a[i] != 1) {
		state = 1;
		break;
            }
	}

    if (!state)
        printf("OK\n");

    free(a);
    return state;

}
