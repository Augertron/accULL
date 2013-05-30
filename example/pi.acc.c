/* 
 * PI Computation in OpenACC 
 */

#include <stdio.h>
#include <sys/time.h>

#define N_ELEM 512000

typedef  struct timeval  CLOCK_TYPE;

int main (int argc) {
	int i;
  	int n = N_ELEM;
  	double pi = 0.0f;
  	double  sum = 0.0f;
   	double h;
  	double time_elapsed;
  	h = 1.0 / (double) n;   // local var to the device

  	CLOCK_TYPE chrono;
  	gettimeofday ((&chrono), NULL);
	#pragma acc kernels loop reduction(+:sum) copy(sum) 
  	for (i = 0; i < n; i ++) {
    	double x = h * ((double) i - 0.5);
      	sum += 4.0 / (1.0 + x * x);
    }
 	pi = h * sum;
 	
	CLOCK_TYPE ch2;

  	gettimeofday ((&ch2), NULL);
  	time_elapsed = (double )(ch2.tv_sec  - chrono.tv_sec ) + ((double )(ch2.tv_usec  - chrono.tv_usec ) / 1.0e6);
  	printf ("Pi %f \n", pi);
  	printf  ("Time: %g s\n"  ,time_elapsed  ) ;
	return 0;
}
