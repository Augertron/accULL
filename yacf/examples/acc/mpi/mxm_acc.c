/*
  CPP_NUM_CORES = 32
  CPP_PROCESSES_PER_NODE 4
  CPP_PROBLEM=D
*/

#include <omp.h>
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifdef LLC_TRANSLATION
typedef int MPI_Status;
#endif

int mm_block_sub(double * a, double * b, double * c, int n, int block)
{
  assert(a != NULL && b != NULL && c != NULL);
  assert(n > 0 && block > 0);
  assert(0 == n % block);
  
  int i,j,k;
  for (i = 0; i < block; i++)
    for (j = 0; j < block; j++)
      for (k = 0; k < block; k++)
	c[i * n + j] += a[i * n + k] * b[k * n + j];
  
  return 0;
}

int mm_block(double * a, double * b, double * c, int n, int m, int block)
{
  assert(a != NULL && b != NULL && c != NULL);
  assert(n > 0 && block >= 0);
  assert(0 == n % block);

  int res = -1;

  int i,j,k;
  for (i = 0; i < m; i += block)
    for (j = 0; j < n; j += block)
      for (k = 0; k < n; k += block)
	mm_block_sub(&a[i * n + k], &b[k * n + j], 
		     &c[i * n + j], n, block);
  
  
  return res;
}

void mm2(int t, double *a, double *b, double *c, int endx)
{
    int i, j, k;
#ifdef BLOCKED_MXM
//int mm_block(double * a, double * b, double * c, int n, int block)
if (endx != t) printf("** Endx %d T %d \n", endx, t);
   mm_block(a, b, c, t, endx, 128);
#else
// #pragma omp parallel for default(none) private(i,j,k,s) firstprivate(t,endx) shared(a,b,c)
#pragma acc kernels loop collapse(2) private(i,j,k) copyin(b[t*t],a[t*t], endx,t) copy(c[t*endx])
    for (i = 0; i < endx; i++) 
	for (j = 0; j < t; j++) {
	    double s = 0.;
	    for (k = 0; k < t; k++)
		s += a[i * t + k] * b[k * t + j];
	    c[i * t + j] = s;
	}
#endif
}

void multiplicaMPI(int t, double *a, double *b, double *c, int nodo,
		   int np)
{

    MPI_Status status;
    int i, j;
    MPI_Bcast(&t, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int block = t / np;
    int resto = t % np;
    double *temp1, *temp2;
    int start;
    int end = block;
    int tag_send = 10;
    int tag_receive = 30;

    if (nodo != 0) {
	a = (double *) malloc(sizeof(double) * t * block);
	b = (double *) malloc(sizeof(double) * t * t);
	c = (double *) malloc(sizeof(double) * t * block);
    }
    // Envío de la matriz A
    if (nodo == 0) {
	for (i = 1; i < np; i++) {
	    start = i * block + resto;
	    MPI_Send(&a[t * start], t * block, MPI_DOUBLE, i, tag_send,
		     MPI_COMM_WORLD);
	}
	end = block + resto;
    } else {
	MPI_Recv(a, t * block, MPI_DOUBLE, 0, tag_send, MPI_COMM_WORLD,
		 &status);
    }

    // Broadcast de B
    MPI_Bcast(b, t * t, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Multiplicación
    mm2(t, a, b, c, end);

    // Se compone la matriz completa en C
    if (nodo != 0) {
	MPI_Send(c, t * block, MPI_DOUBLE, 0, tag_receive, MPI_COMM_WORLD);
    } else {
	for (i = 1; i < np; i++) {
	    start = i * block + resto;
	    MPI_Recv(&c[t * start], t * block, MPI_DOUBLE, i, tag_receive,
		     MPI_COMM_WORLD, &status);
	}
    }
}

void mm(int t, double *a, double *b, double *c, double *d, double *r,
	int nodo, int np)
{
    MPI_Bcast(&t, 1, MPI_INT, 0, MPI_COMM_WORLD);
 //   double *temp = (double *) malloc(sizeof(double) * t * t);
    if ((t <= 10)) {
	if (nodo == 0) {
	    mm2(t, a, b, r, t);
/*         mm2(t,temp,c,a,t);
          mm2(t,a,d,r,t);*/
	} else {
	    r = (double *) malloc(sizeof(double) * t * t);
	}
	MPI_Bcast(r, t * t, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    } else if ((t > 10)) {
	multiplicaMPI(t, a, b, r, nodo, np);
/*      multiplicaMPI(t,temp,c,a,nodo,np);
      multiplicaMPI(t,a,d,r,nodo,np);*/
    }
 //   free(temp);
}
