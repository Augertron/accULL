#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define N 100


int main () {
    int n = N;
    int ppn = N;
    uint8_t * a = (uint8_t *) malloc(sizeof(uint8_t) * N);
  int i;
   {
    uint8_t * z = NULL;   
    z = (uint8_t *) malloc(sizeof(uint8_t) * N);


    for (i = 0; i < N; i++) {
	z[i] = 1;
        a[i] = 0;
    }

  #pragma acc parallel loop copyout(a[0:100]), private(z[0:100]) num_gangs(256)
  for (i=0;i<n;i++){
    a[i] = i;
    z[i] = i;
  }

  for (i = 0; i < N; i++)
	if (a[i] != i) {
           printf("Error: a[%d] should be %d but it is %d ", i, z[i], a[i]);
           return 1;
        }


    for (i = 0; i < N; i++) {
	z[i] = 1;
        a[i] = 0;
    }


  /* This private will be moved to create */
  #pragma acc parallel loop create(a[0:n]), private(z[0:n]), num_gangs(256)
  for (i=0;i<n;i++){
    a[i] = i;
    z[i] = i;
  }

  #pragma acc parallel loop create(a[0:ppn], z[0:ppn]),  num_gangs(256)
  for (i=0;i<ppn;i++){
    a[i] = i;
    z[i] = i;
  }


    // Check results
   for (i = 0; i < N; i++) {
	if (a[i] != 0) {
           printf("Error: a[%d] should be %d but it is %d ", i, 0, a[i]);
           return 1;
        }
        if (z[i] != 1) {
	    printf("Error: z[%d] should be %d but it is %d ", i, 1, z[i]);
           return 1;
        }
    }

   }

   {
    int z;
  #pragma acc parallel loop create(a[0:ppn]) reduction(+ : z)
  for (i=0;i<ppn;i++){
    z += i;
  }

  }

    return 0;
}
