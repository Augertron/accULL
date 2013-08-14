#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mytime.h"

#define MAXITER 100000
#define THRESOLD 2.0
#define npoints 4092

float uni (void);
void rinit (int);

typedef struct {
    double creal;
    double cimag;
} complex;

int min (int a, int b) {
    return((a < b) ? a : b); 
}


int main(int argc, char **argv) {
    int i, j, numinside;
    double area_llc, error_llc;
    complex z, c[npoints];

    double ztemp;
    int numoutside;
    CLOCK_TYPE chrono;
	double t_llc;

/*
 *  Generate npoints random points in the complex plane
 */

    rinit (54321);
    for (i = 0; i < npoints; i++) {
        c[i].creal = -2.0 + 2.5 * uni();
        c[i].cimag = 1.125 * uni();
    }

/*
 *  Parallel (with llc)
 */

	CLOCK_Start(chrono);
 
	numoutside = 0;
    #pragma omp target device(mpi)
    #pragma omp parallel for default(none) reduction(+:numoutside) private(i, j, ztemp, z)
    for (i = 0; i < npoints; i++) {
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
  
	numinside = npoints - numoutside;

/* *  PARALLEL llc: Calculate area and error */
    area_llc = 2.0 * 2.5 * 1.125 * numinside / npoints;
    error_llc = area_llc / sqrt(npoints);
	
	CLOCK_End(chrono, t_llc);

    printf("%d:%g:%16.12f +/- %16.12f\n", LLC_NAME, t_llc, area_llc, error_llc);

    return 0;
}
