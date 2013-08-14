
#include <stdio.h>
#include <stdlib.h>

#include "llcFunctions.h"


#define N 10


/*  test02.cpp
    ====================================

	This test code shows the code to create an llcRegion,
	associate an INOUT variable, launch a basic kernel
	and then finish the region.

	Equivalent llc source would be:

	{
	for (i = 0; i < N; i++)
		a[i] = i;

	#pragma llc region name("test00") copy_in(a) copy_out(a) weight(N)
   { 
		#pragma llc for  shared(a) if(N>3)  name("main")
		for (int i = 0; i < N; i++)
		{
				a[i] = i;
		}
		
	}

	for (i = 0; i < N; i++)
		if (a[i] != i) {
			printf("* Not valid %d\n", i);
		}


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
	int * a = (int *) malloc(size);

   // Parallelization of this loop is usually not a good choice in OpenMP
	// but including this loop inside an llc region will enhance performance
	// because this initialization will be done at device prior to parallelization
	for (i = 0; i < N; i++)
		a[i] = i;

	/* Region start */
	int regionId = beginRegion("test00");
	createVar(a, regionId, size, VAR_INOUT);  //  Compiler should create here the device vars in a separate thread 
														   //   to improve performance
	/* No operation within the region means no transfer, just allocation */
	{

		tparams * params;
		params = malloc(sizeof(tparams));
		params->_base_ptr = a;  // Base address is used to identify the correspondant HostVar structure
		params->prev = NULL;
		// Kernel name is appended to region name
		launch_kernel(regionId, "test00_main", params);
	}
	/* Region end */
	endRegion("test00");  // End region will transfer back A vector

	for (i = 0; i < N; i++)
		if (a[i] != i) {
			printf("* Not valid %d\n", i);
		}

}
