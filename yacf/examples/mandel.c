
#ifndef LLC_TRANSLATION
#include <stdio.h>
#include <stdlib.h> 
#include <math.h> 
#endif


#include "examples/mytime.h"



#define LLC_printMaster printf
#define LLC_NUMPROCESSORS 1
#define LLC_NAME 

# define MAXITER 100000
# define THRESOLD 2.0
# define npoints 4092

float uni (void) {
	return (float) random() / RAND_MAX;
}

void rinit (int seed) {
	srandom(seed);
}

  struct complex{
      double creal;
      double cimag;
  };

  int min(int a, int b) {
   return((a < b) ? a : b); 
  }

int main(int argc, char **argv){
    int i, j, numinside;
    int num_threads = 1;
    double area_seq, error_seq;
    double area_llc, error_llc;
    double area_mpi, error_mpi;
    struct complex z, c[npoints];

    double ztemp;
    int numoutside, gnumoutside;
    int nt;                 /* No. of threads */
		int MPI_NUMPROCESSORS, MPI_NAME;

		CLOCK_TYPE chrono;
		double t_llc, t_seq, t_mpi;

		LLC_printMaster ("\n\n\n*************** NUMPROCESSORS = %d ***************************\n\n",
				LLC_NUMPROCESSORS);
    rinit (54321);
    for (i=0; i<npoints; i++) {
        c[i].creal = -2.0+2.5*uni();
        c[i].cimag = 1.125*uni();
    }



	CLOCK_Start(chrono);
 
   printf ("** Loop **");
	numoutside = 0;
#pragma omp target device(cuda) copy_in(c) 
#pragma omp parallel for reduction(+:numoutside) private(i,j,ztemp,z) shared(nt,c)
    for(i = 0; i<npoints; i++) {
      z.creal = c[i].creal;
      z.cimag = c[i].cimag;
      for (j = 0; j < MAXITER; j++) {
        ztemp = (z.creal * z.creal) - (z.cimag * z.cimag) + c[i].creal;
        z.cimag = z.creal * z.cimag * 2 + c[i].cimag;
        z.creal = ztemp;
        if (z.creal * z.creal + z.cimag * z.cimag > THRESOLD) {
          numoutside++;
          // break;
        } 
      } /* for j */
    } /* for i */
  
   printf ("** End Loop **");
	numinside = npoints - numoutside;

/* *  5. PARALLEL llc: Calculate area and error */
  area_llc = 2.0 * 2.5 * 1.125 * numinside / npoints;
  error_llc = area_llc / sqrt(npoints);
	
	CLOCK_End(chrono, t_llc);




	
  LLC_printMaster ("%d\t%d\t%g\t#llc_plot0 MANDEL: N = %ld. [seq_time(%g)/llc_time(%g)]\n", num_threads, 
			              LLC_NUMPROCESSORS, (t_seq/t_llc), npoints, t_seq, t_llc);
  LLC_printMaster ("%d\t%d\t%g\t#llc_plot1 MANDEL: N = %ld. [seq_time(%g)/mpi_time(%g)]\n", num_threads,
			              LLC_NUMPROCESSORS, (t_seq/t_mpi), npoints, t_seq, t_mpi);
  LLC_printMaster ("%d\t%d\t%g\t#llc_plot2 MANDEL: N = %ld. [mpi_time(%g)/llc_time(%g)]\n", num_threads, 
			              LLC_NUMPROCESSORS, (t_mpi/t_llc), npoints, t_mpi, t_llc);


	
  }
