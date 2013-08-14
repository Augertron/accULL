#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <openacc.h>

#define EPSILON 1.0e-11

int main ( int argc, char *argv[] )
{
	int id;
	int i, j;
	int size;
	int *a;
	int *b;
	int *c;

	printf("Check async clause in OpenACC\n");


	if (argc != 2) {
  		printf("** Use %s vector size \n", argv[0]);
		return EXIT_FAILURE;
	}
	
	acc_init(acc_device_host);

	size = atoi(argv[1]);
	a = (int*)malloc(sizeof(int)*size);
	b = (int*)malloc(sizeof(int)*size);
	c = (int*)malloc(sizeof(int)*size);

	// Initialize arrays
	for (i = 0; i < size; i++) {
		a[i] = i;
		b[i] = i;
	}

	#pragma acc data copyin(a[0:size], b[0:size]) copy(c[0:size]) 
	{
		#pragma acc kernels loop private(i) async independent
		for (i = 0; i < size/2; i++) {
			c[i] = a[i] * b[i];
		}

		#pragma acc kernels loop private(j) independent
		for (j = size/2; j < size; j++) {
			c[j] = a[j] * b[j];
		}

	}


	int result = 1;
	// Check the result
	for (i = 0; i < size; i++) {
		if (c[i] != i*i) {
            result = 0;
            break;
        }
	}

    if (result == 1) {
        printf("Result is correct\n");
    }
    else {
        printf("Result is not correct\n");
    }

	free(a);
	free(b);
	free(c);

	return 0;
}

