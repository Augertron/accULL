#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mytime.h"

# define MAXITER 100000
# define THRESOLD 2.0
# define npoints 4092

float uni (void);
void rinit (int);

struct complex{
    double creal;
    double cimag;
};

int min (int a, int b) {
    return((a < b) ? a : b); 
}


int main (int argc, char **argv) {
    int i, j, numinside;
    double area_seq, error_seq;
    struct complex z, c[npoints];

    double ztemp;
    int numoutside;

	CLOCK_TYPE chrono;
	double t_seq;

/*
 *  1. Generate npoints random points in the complex plane
 */

    rinit (54321);
    for (i = 0; i < npoints; i++) {
        c[i].creal = -2.0 + 2.5 * uni();
        c[i].cimag = 1.125 * uni();
    }

/*
 *  2. Monte Carlo sampling 
 *
 *    2a. Outer loop runs over npoints, initilaise z=c
 *
 *    2b. Inner loop has the iteration z=z*z+c, and threshold test
 */
	CLOCK_Start(chrono);
	
	numoutside = 0;
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
        }
    }
    numinside = npoints - numoutside;

/*
 *  3. Calculate area and error and output the Results
 */

    area_seq = 2.0 * 2.5 * 1.125 * numinside / npoints;
    error_seq = area_seq / sqrt(npoints);
	CLOCK_End(chrono, t_seq);

    printf("%d:%g:%16.12f +/- %16.12f\n", 0, t_seq, area_seq, error_seq);

    return 0;
}
