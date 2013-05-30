#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char *argv[]);
void r8_mxm(int l, int m, int n);
double r8_uniform_01(int *seed);

/******************************************************************************
  Purpose:
    MAIN is the main program for MXM.

  Licensing:
    This code is distributed under the GNU LGPL license. 

  Modified:
    19 April 2009

  Author:
    John Burkardt
******************************************************************************/

int main(int argc, char *argv[]) {
  int l = 500, m = 500, n = 500;

  //printf("\n");
  //printf("MXM\n");
  //printf(" C/OpenMP version.\n");
  //printf("\n");
  //printf(" Matrix multiplication tests.\n");

  //printf("\n" );
  //printf(" Number of processors available = %d\n", omp_get_num_procs());
  //printf(" Number of threads =              %d\n", omp_get_max_threads());

  r8_mxm(l, m, n);

  //printf("\n");
  //printf("MXM:\n");
  //printf(" Normal end of execution.\n");

  return 0;
}

/******************************************************************************
  Purpose:
    R8_UNIFORM_01 is a unit pseudorandom R8.

  Discussion:
    This routine implements the recursion
      seed = 16807 * seed mod ( 2**31 - 1 )
      unif = seed / ( 2**31 - 1 )
    The integer arithmetic never requires more than 32 bits,
    including a sign bit.

  Licensing:
    This code is distributed under the GNU LGPL license. 

  Modified:
    11 August 2004

  Author:
    John Burkardt

  Reference:
    Paul Bratley, Bennett Fox, Linus Schrage,
    A Guide to Simulation,
    Springer Verlag, pages 201-202, 1983.
    Bennett Fox,
    Algorithm 647:
    Implementation and Relative Efficiency of Quasirandom
    Sequence Generators,
    ACM Transactions on Mathematical Software,
    Volume 12, Number 4, pages 362-376, 1986.

  Parameters:
    Input/output, int *SEED, a seed for the random number generator.
    Output, double R8_UNIFORM_01, a new pseudorandom variate, strictly between
    0 and 1.
******************************************************************************/

double r8_uniform_01(int *seed) {
  int k;
  double r;
  k = *seed / 127773;
  *seed = 16807 * (*seed - k * 127773) - k * 2836;
  if (*seed < 0)
    *seed = *seed + 2147483647;
  r = (double) (*seed) * 4.656612875E-10;
  return r;
}

/******************************************************************************
  Purpose:
    R8_MXM carries out a matrix-matrix multiplication in R8 arithmetic.

  Discussion:
    A(LxN) = B(LxM) * C(MxN).

  Licensing:
    This code is distributed under the GNU LGPL license. 

  Modified:
    13 February 2008

  Author:
    John Burkardt

  Parameters:
    Input, int L, M, N, the dimensions that specify the sizes of the
    A, B, and C matrices.
******************************************************************************/

void r8_mxm(int l, int m, int n) {
  double *a;
  double *b;
  double *c;
  int i;
  int j;
  int k;
  double R;
  //int ops;
  //double rate;
  int seed;
  struct timeval start_time, end_time;
  double time;

/* Allocate the matrices. */

  b = (double *) malloc(l * m * sizeof(double));
  a = (double *) malloc(m * n * sizeof(double));
  c = (double *) malloc(l * n * sizeof(double));

/* Assign values to the B and C matrices. */

  seed = 123456789;
  for (k = 0; k < (l * m); k++)
    a[k] = r8_uniform_01(&seed);
  for (k = 0; k < (m * n); k++)
    b[k] = r8_uniform_01 (&seed);

  /* Compute C = B * A. */

  /* SEQUENTIAL EXECUTION */

  gettimeofday (&start_time, 0);

  {

  for (j = 0; j < n; j++) {
    for (i = 0; i < l; i++) {
      R = 0.0;
      for (k = 0; k < m; k++)
          R += b[i + k * l] * a[k + j * m];
      c[i + j * l] = R;
    }
  }

  }

  gettimeofday (&end_time, 0);
  time = end_time.tv_sec - start_time.tv_sec;
  time = time + (end_time.tv_usec - start_time.tv_usec) / 1e6;

  /* END SEQUENTIAL EXECUTION */

  printf ("%d:%f\n", 0, time);

  /*
  printf("\nA");
  for (i = 0; i < m; i++) {
    printf("\n");
    for (j = 0; j < n; j++)
        printf("%f ", a[i + j * n]);
  }
  printf("\n");
   
  printf("\nB");
  for (i = 0; i < l; i++) {
    printf("\n");
    for (j = 0; j < m; j++)
        printf("%f ", b[i + j * m]);
  }
  printf("\n");

  printf("\nC = B * A");
  for (i = 0; i < l; i++) {
    printf("\n");
    for (j = 0; j < n; j++)
        printf("%f ", c[i + j * n]);
  }
  printf("\n");
  */

/* Report. */

  //ops = l * n * (2 * m);
  //rate = (double) (ops) / time / 1000000.0;
  //printf( "\n" );
  //printf( "R8_MXM matrix multiplication timing.\n" );
  //printf( " A(LxN) = B(LxM) * C(MxN).\n" );
  //printf( " L = %d\n", l );
  //printf( " M = %d\n", m );
  //printf( " N = %d\n", n );
  //printf( " Floating point OPS roughly %d\n", ops );
  //printf( " Time = %g\n", time );
  //printf( " Rate = MegaOPS/dT = %f\n", rate );

  free(a);
  free(b);
  free(c);

  return;
}

