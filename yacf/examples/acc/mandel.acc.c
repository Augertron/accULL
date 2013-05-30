
#include <stdio.h>
#include <stdlib.h> 
#include <math.h> 


#include "mytime.h"



#define LLC_printMaster printf
#define LLC_NUMPROCESSORS 1
#define LLC_NAME 

# define MAXITER 100000
# define THRESOLD 2.0

double uni (void) {
	return (double) random() / RAND_MAX;
}

void rinit (int seed) {
	srandom(seed);
}

typedef   struct {
      double creal;
      double cimag;
  } complex;


  int min(int a, int b) {
   return((a < b) ? a : b); 
  }

int main(int argc, char **argv){
    int i, j, numinside;
    int num_threads = 1;
    double area_seq, error_seq;
    double area_llc, error_llc;
    double area_mpi, error_mpi;
    complex z; // , * c; // [npoints];
    int npoints;

    double ztemp;
    int numoutside, gnumoutside;
    int nt;                 /* No. of threads */
		int MPI_NUMPROCESSORS, MPI_NAME;

		CLOCK_TYPE chrono;
		double t_llc, t_seq, t_openmp;

		LLC_printMaster ("\n\n\n*************** NUMPROCESSORS = %d ***************************\n\n",
				LLC_NUMPROCESSORS);

    if (argc != 2) {
         printf("*** Incorrect number of parameters \n");
         exit(-1);
   }
    npoints = atoi(argv[1]);
    complex * c = malloc(npoints *sizeof(complex));

    rinit (54321);
    for (i=0; i<npoints; i++) {
        c[i].creal = -2.0+2.5*uni();
        c[i].cimag = 1.125*uni();
    }

	CLOCK_Start(chrono);

//#pragma llc context name("mandelbrot_cuda") copy_in(c, numoutside) copy_out(numoutside)
{
//   printf ("** Loop **");
	numoutside = 0;

  //  #pragma llc for reduction(+:numoutside) private(i,j,ztemp,z) shared(nt,c)
    for(i = 0; i<npoints; i++) {
      z.creal = c[i].creal;
      z.cimag = c[i].cimag;
      for (j = 0; j < MAXITER; j++) {
        ztemp = (z.creal * z.creal) - (z.cimag * z.cimag) + c[i].creal;
        z.cimag = z.creal * z.cimag * 2 + c[i].cimag;
        z.creal = ztemp;
        if (z.creal * z.creal + z.cimag * z.cimag > THRESOLD) {
          numoutside++;
          break;
        } 
      } /* for j */
    } /* for i */
  
//   printf ("** End Loop **");
}
	numinside = npoints - numoutside;

/* *  5. PARALLEL llc: Calculate area and error */
  area_seq = 2.0 * 2.5 * 1.125 * numinside / npoints;
  error_seq = area_seq / sqrt(npoints);
	
	CLOCK_End(chrono, t_seq);

	CLOCK_Start(chrono);

{
	numoutside = 0;

//    #pragma omp parallel for reduction(+:numoutside) private(i,j,ztemp,z) shared(nt,c)
    for(i = 0; i<npoints; i++) {
      z.creal = c[i].creal;
      z.cimag = c[i].cimag;
      for (j = 0; j < MAXITER; j++) {
        ztemp = (z.creal * z.creal) - (z.cimag * z.cimag) + c[i].creal;
        z.cimag = z.creal * z.cimag * 2 + c[i].cimag;
        z.creal = ztemp;
        if (z.creal * z.creal + z.cimag * z.cimag > THRESOLD) {
          numoutside++;
          break;
        } 
      } /* for j */
    } /* for i */
  
//   printf ("** End Loop **");
}
	numinside = npoints - numoutside;

/* *  5. PARALLEL llc: Calculate area and error */
  area_seq = 2.0 * 2.5 * 1.125 * numinside / npoints;
  error_seq = area_seq / sqrt(npoints);
	
	CLOCK_End(chrono, t_openmp);




	CLOCK_Start(chrono);
 
	numoutside = 0;
#pragma acc kernels loop reduction(+:numoutside) private(i,j) copyin(npoints, c[npoints]) copy(numoutside)
    for(i = 0; i<npoints; i++) {
       complex z;
       double ztemp;
       z.creal = c[i].creal;
       z.cimag = c[i].cimag;
      for (j = 0; j < MAXITER; j++) {
        ztemp = (z.creal * z.creal) - (z.cimag * z.cimag) + c[i].creal;
        z.cimag = z.creal * z.cimag * 2 + c[i].cimag;
        z.creal = ztemp;
        if (z.creal * z.creal + z.cimag * z.cimag > THRESOLD) {
          numoutside++;
          break;
        } 
      } /* for j */
    } /* for i */
  
//   printf ("** End Loop **");
	numinside = npoints - numoutside;

/* *  5. PARALLEL llc: Calculate area and error */
  area_llc = 2.0 * 2.5 * 1.125 * numinside / npoints;
  error_llc = area_llc / sqrt(npoints);
	
	CLOCK_End(chrono, t_llc);



  printf(" Error seq: %e , Error llc: %e \n ", error_seq, error_llc);
	
  LLC_printMaster ("%d\t%d\t%g\t#llc_plot0 MANDEL: N = %d. [seq_time(%g)/llc_time(%g)]\n", num_threads, 
			              LLC_NUMPROCESSORS, (t_seq/t_llc), npoints, t_seq, t_llc);
  LLC_printMaster ("%d\t%d\t%g\t#llc_plot1 MANDEL: N = %d. [seq_time(%g)/openmp_time(%g)]\n", num_threads,
			              LLC_NUMPROCESSORS, (t_seq/t_openmp), npoints, t_seq, t_openmp);
  LLC_printMaster ("%d\t%d\t%g\t#llc_plot2 MANDEL: N = %d. [openmp_time(%g)/llc_time(%g)]\n", num_threads, 
			              LLC_NUMPROCESSORS, (t_openmp/t_llc), npoints, t_openmp, t_llc);


	
  }
