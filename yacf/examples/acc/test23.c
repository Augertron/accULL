
#define N 100

int main () {
    int n = N;
    int z = 0;
    int z_h = 0;
    int i, j;


  #pragma acc parallel loop reduction(+:z)
  for (i=0;i<n;i++){
    z += i;
  }

 for (i=0;i<n;i++){
    z_h += i;
  }
    // Check results
	if (z != z_h) {
           printf("Error: z %d != z_h %d \n", z, z_h);
           return 1;
        }

  z = z_h = 0;


 for (i=0;i<n;i++){
   for (j=0; j<n;j++) 
    z_h += 1;
  }

 #pragma acc parallel loop reduction(+:z)
 for (i=0;i<n;i++){
   for (j=0; j<n;j++) 
    z += 1;
  }

    // Check results
	if (z != z_h) {
           printf("Error: z %d != z_h %d \n", z, z_h);
           return 1;
        }

    return 0;
}
