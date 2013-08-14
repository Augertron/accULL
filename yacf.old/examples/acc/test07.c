
#include <stdio.h>


#define N 100


int main () {
    int i, j, size, state = 0;
    int * a;
    int * b;
    

    a = malloc(N * sizeof(int));
    b = malloc(N * sizeof(int));
    size = N;
    for (i = 0; i < size; i++) {
        a[i] = 0;
        b[i] = 33;
    }


    #pragma acc data copy(a[0:100])
    for (i = 0; i < size/2; i++) {
        #pragma acc kernels loop copy(b[0:100]) private(j) 
        for (j = 0; j < size; j++) {  
            a[j] = j;
            b[i] = 5;
        }  
        // This update should not be neccessary !
        // #pragma acc update device(b)
        // printf("b[2] = %g \n", b[2]);
	for (j = 0; j < i; j++) {
		if (b[j] != 5) {
			// The scope is not finished
			printf(" b[%d] != 5 (= %d) ", i, b[i]);
			state = 1;
			break;
		}
		if (j > 0 && (a[j] == j)) {
			// The scope defined by the data is beign flushed 
			//  incorrectly
			printf(" a[%d] == %d (should be = 0) ", a[j], j);
			state = 1;
			break;

		}
	}
	if (state) break;
    }

    for (j = 0; j < size; j++) {
		if (a[j] != j) {
			printf(" a[%d] != %d  ", a[j], j);
			state = 1;
			break;
		}
	}

    printf("OK\n");

    free(a);
    free(b);
    return state;
}
