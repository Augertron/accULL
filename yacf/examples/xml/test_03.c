// #include <stdio.h>
// #include <stdlib.h>
#define N 10


/*  test02.cpp
    ====================================

	This test code shows the code to create an llcRegion,
	associate an INOUT variable, launch a basic kernel
	and then finish the region.

	Equivalent llc source would be:


	Yacf must generate a kernel code for the kernel section. 
	   Name clause is optional always optional.

	"Kernel" means a zone of code suitable for SIMD execution.
	"Region" means a zone of code where code is not garanteed to be 
	runned on the host, the instructions could be reordered but preserving
		the semantics. Memory coherence is guaranteed by runtime.


	}
*/



int main() {
	int size = N * sizeof(int);
	int i = 0;

   // This initialization, although its parallelization is not required
	// should be defined inside the region to improve performance
	int a[N]; // = (int *) malloc(size);
    int b[N][N];

   // Parallelization of this loop is usually not a good choice in OpenMP
	// but including this loop inside an llc region will enhance performance
	// because this initialization will be done at device prior to parallelization
	for (i = 0; i < N; i++)
		a[i] = i;

	/* Region start */
	#pragma llc region name("test00") copy_in(a, b) copy_out(a) // weight(N)
   { 
		#pragma llc for shared(a, b) name("main")
		for (i = 0; i < N; i++)
		{
				a[i] = i;
                b[i][i] = i;
		}
		
	}
	
	for (i = 0; i < N; i++)
		if (a[i] != i) {
			printf("* Not valid %d\n", i);
		}

}
