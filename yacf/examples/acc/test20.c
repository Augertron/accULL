
#define N 100


int main () {
    int n = N;
    int * a = (int *) malloc(sizeof(int) * N);
    int * z = (int *) malloc(sizeof(int) * N);
    int i;


    for (i = 0; i < N; i++) {
	z[i] = 1;
        a[i] = 0;
    }

  #pragma acc parallel loop copyout(a[0:n]) firstprivate(z[0:n])
  for (i=0;i<n;i++){
    a[i] = z[i];
  }

    // Check results

   for (i = 0; i < N; i++)
	if (a[i] != 1) {
           printf("Error: a[%d] should be %d but it is %d ", i, z[i], a[i]);
           return 1;
        }
    return 0;
}
