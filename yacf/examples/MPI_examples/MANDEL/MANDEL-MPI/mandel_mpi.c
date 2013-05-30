#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include "mytime.h"

#define MAXITER 100000
#define THRESOLD 2.0
#define npoints 4092

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
    double area_mpi, error_mpi;
    struct complex z, c[npoints];

    double ztemp;
    int numoutside, gnumoutside;
	int MPI_NUMPROCESSORS, MPI_NAME;
    CLOCK_TYPE chrono;
	double t_mpi;
    int namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

/*
 *  Generate npoints random points in the complex plane
 */

    rinit (54321);
    for (i = 0; i < npoints; i++) {
        c[i].creal = -2.0 + 2.5 * uni();
        c[i].cimag = 1.125 * uni();
    }

/*
 *  Parallel (with MPI)
 */

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &MPI_NUMPROCESSORS);
	MPI_Comm_rank(MPI_COMM_WORLD, &MPI_NAME);
    MPI_Get_processor_name(processor_name, &namelen);
	
	CLOCK_Start(chrono);
 
	numoutside = 0;
    for(i = MPI_NAME; i < npoints; i += MPI_NUMPROCESSORS) {
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

    MPI_Allreduce(&numoutside, &gnumoutside, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);        
	numinside = npoints - gnumoutside;

/* *  PARALLEL MPI: Calculate area and error */
    area_mpi = 2.0 * 2.5 * 1.125 * numinside / npoints;
    error_mpi = area_mpi / sqrt(npoints);
	
	CLOCK_End(chrono, t_mpi);

    printf("%d:%g:%16.12f +/- %16.12f\n", MPI_NAME, t_mpi, area_mpi, error_mpi);

    MPI_Finalize();

    return 0;
}
