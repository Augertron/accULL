#include <stdio.h>
#include <stdlib.h>

#define uint8_t  unsigned int


int main(int argc, char *argv[] ) {

	int n = 1000;
	int i = 0;
	uint8_t * a;
	a = (uint8_t *) malloc( n * sizeof (uint8_t ) );
	int state = 0;
	
	// Initialization
	for (i = 0; i < n; i++) {
  		a[i] = rand();
	}

	// Computation
	#pragma acc data copyin(a[0:n])
  	{
		#pragma acc kernels loop if(0)
		for (i = 0; i < n; i++) {
    		a[i] = (uint8_t) i + 100;
  		}
	}

	// Check if the result is correct	
	for (i = 0; i < n; i++) {
  		if (a[i]  != i + 100) {
			state = 1;
			printf("a[%d] should be %d (a[%d] = %d)\n", i, i+100, i, a[i]);
			break;
		}
	}

	free(a);
	return state;
}
