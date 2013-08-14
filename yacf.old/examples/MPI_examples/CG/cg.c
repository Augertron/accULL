/*
 * Kernel CG: Solving and Unstructured Sparse Linear System by
 * the Conjugate Gradient Method (in NAS Parallel Benchmarks)
 */
/* this program was originally written by itakura@rccp.tukuba.ac.jp */
/* modified to seq'ed back by msato@trc.rwcp.or.jp */
/* this OpenMP program does not use orphan directives. */

#include <stdio.h>
#include <mpi.h>
#include "cg.h"

#define SEQ_TIME 0.0

double time1, time2;
double second();

double a[NNZ], x[NN], z[NN], r[NN], p[NN], q[NN];
int   colstr[NNP1], rowidx[NNZ];
char *datafile  = "data";

double cgsol();

int main(int argc, char **argv) {
  int nn, nnp1, nnz, lenwrk, ilnwrk, niter, nitcg;
  double rcond, shift;
    
  double resid, zeta, ztz, znorminv;
  int i, it, nnzcom, imax;
  double randlc();
    
  int  nnzchk;
  double zetchk, zettol, reschk;
    
  nn = NN;

 	LLC_printMaster ("\n************************ NUM PROCS = %d *****************************\n", 
			LLC_NUMPROCESSORS);
   
  tran    = 314159265.0;
  amult   = 1220703125.0;

	if (LLC_NAME == 0) {
		if(argc == 2) 
			datafile = argv[1];

    dataread(nn,a,rowidx,colstr);
	}

#ifndef SEQUENTIAL
	if (LLC_NUMPROCESSORS > 1) {
		MPI_Bcast (&nn, 		1, 					MPI_INT, 		0, 	MPI_COMM_WORLD);
		MPI_Bcast (colstr, 	(nn + 1), 	MPI_INT, 		0, 	MPI_COMM_WORLD);
		MPI_Bcast (rowidx, 	colstr[nn], MPI_INT, 		0, 	MPI_COMM_WORLD);
		MPI_Bcast (a, 			colstr[nn], MPI_DOUBLE, 0, 	MPI_COMM_WORLD);
	}
#endif 

  for(i=0; i< nn; i++)
  	x[i]=1.0;

  time1 = second();
  for(it=0; it < NITER; it++) {
		resid = cgsol(nn, a, rowidx, colstr, x, &zeta);
		/*printf("%5d  %12.4e  %20.14f\n", it, resid, zeta + SHIFT);*/
  }
  time2 = second();

	printf ("NAME = %d, NP = %d. SOL: resid = %g, zeta = %g\n", LLC_NAME, LLC_NUMPROCESSORS, resid, zeta);
	printf ("NAME = %d, NP = %d. TIME => total = %g, iter = %g [(%g - %g)/%d], NUM_ITER_CG=%d\n",
	   	LLC_NAME, LLC_NUMPROCESSORS, time2-time1, (time2-time1)/NITER, time2, time1, NITER, NITCG);
#ifdef SEQ_TIME
	LLC_printMaster ("%d\t%g\t#llc_plot0 CG:NONZEROS = %d, iter=%d, iterCG=%d (t_seq=%g/t_par=%g)\n", 
			LLC_NUMPROCESSORS, (SEQ_TIME / (time2-time1)), NNZCHK, NITER, NITCG, SEQ_TIME, (time2-time1));
#endif
 return (0);
}


double dotpro(int n, double *x, double *y) {
	double z = 0.0;
  int i;

#pragma omp target device(mpi)
#pragma omp parallel for reduction(+:z)
    for(i=0; i<n; i++)
      z += x[i] * y[i];
    return(z);
}

/* n: size of matrix 
 * colstr[n+1]: column vector, pointer to a and rowidx
 * a[nnz]: value vector
 * rowidx[nnz]: row index vector
 * x[n]: 
 * zeta:
 *
 * work:
 *	z[n],r[n],p[n],q[n]
 */
double cgsol(int n, double *a, int *rowidx, int *colstr, double *x, double *zeta) {
  int it, cols,i;
	double alpha, beta, rho, rho0, znorminv;
    
  cols = n;
#pragma omp target device(mpi)
#pragma omp parallel for
#pragma llc result (&p[i], 1, &z[i], 1, &r[i], 1) 
  for(i = 0; i < cols; i++){
    p[i] = r[i] = x[i];  /* r = x, p = x */
    z[i] = 0.0;
  }
  rho = dotpro(cols, x, x);	/* rho = x*x */
    
  for(it=0; it<NITCG; it++){
		/* q += a*p */
		matvec(a, colstr, rowidx, p, q, cols);
		/* alpha = rho/p*q */
		alpha = rho / dotpro(cols, p, q);
		/* z += alpha*p */
#pragma omp target device(mpi)
#pragma omp parallel for 
#pragma llc result (&z[i], 1, &r[i], 1) 
		for (i = 0; i < cols; i++) { 
			z[i] += alpha*p[i];
			r[i] += -alpha*q[i];
		}
		
		rho0 = rho;
		/* r += -alpha*q */

		/* rho = r*r */
		rho = dotpro(cols, r, r);
		beta = rho / rho0;
		
		/* p = r + beta*p; */
#pragma omp target device(mpi)
#pragma omp parallel for 
#pragma llc result (&p[i], 1) 
		for(i = 0; i < cols; i++)   
			p[i] = r[i] + beta*p[i];
  }
	
  /* r = a*z */
  matvec(a, colstr, rowidx, z, r, cols);

  /* r -= x */
#pragma omp target device(mpi)
#pragma omp parallel for 
#pragma llc result (&r[i], 1)  

	for(i = 0; i < cols; i++)   
		r[i] = x[i] - r[i];

  /* zeta = 1.0/x*z */
  *zeta = 1.0 / dotpro(cols, x, z);

  /* znorminv = 1.0/sqrt(z*z) */
  znorminv = 1.0 / (sqrt(dotpro(cols, z, z)));

    /* x = znorminv*z */
#pragma omp target device(mpi)
#pragma omp parallel for 
#pragma llc result (&x[i], 1) 
	for(i = 0; i < cols; i++) {
		x[i] = znorminv*z[i];
	}

  return sqrt(dotpro(cols, r, r));
}


int matvec(double *a, int *row_start, int *col_idx, double *x, double *y, int nn) {
  int i, j, start, end;
  double t;

#pragma omp target device(mpi)
#pragma omp parallel for private(i,j,t,start,end)
#pragma llc result (&y[i], 1) 
	for(i = 0; i < nn; i++) {
		start = row_start[i];
		end = row_start[i+1];
		t = 0.0;
		for(j= start; j < end; j++) {
	    t += a[j] * x[col_idx[j]];
		}
		y[i] = t;
  }
}

#ifdef not 
int matvec(double *a, int *colstr, int *rowidx, double *x, double *y, int cols) {
  int i, j, k;
    
  for(i=0; i< NN; i++) 
		y[i]=0.0;

  k=0;
  for(i = 0; i < cols; i++) {
		for(j = 0; j < colstr[i+1] - colstr[i]; j++) {
	    y[rowidx[k]] += a[k] * x[i];
	    k++;
		}
  }
}
#endif


int dataread(int nn, double *a, int *rowidx, int *colstr) { 
  FILE *fp;    
  int n, ok_flag=1;


  if((fp = fopen(datafile,"r")) == NULL) {
	  	printf("cannot open 'data' file\n");
	  	exit(1);
 	}
  ok_flag &= (fread(&n, sizeof(int), 1, fp) == 1);
  if(n != nn) {
		fprintf(stderr, "illegal data size.. file %d, prog %d\n", n, nn);
		exit(1);
  }
  ok_flag &= (fread(colstr, sizeof(int),  nn+1,         fp) == nn+1);
  ok_flag &= (fread(rowidx, sizeof(int),   colstr[nn],fp)== colstr[nn]);
  ok_flag &= (fread(a,      sizeof(double),colstr[nn],fp)== colstr[nn]);
  if(!ok_flag) {
		fprintf(stderr, "fread error\n");
		exit(1);
  }
  close(fp);

}

