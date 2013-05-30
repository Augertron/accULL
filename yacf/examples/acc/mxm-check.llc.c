

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "mytime.h"

#define min(a,b) (((a)>(b))?(b):(a))

typedef double T;

int main (int argc, char *argv[]);
void r8_mxm_llc (int l, int m, int n, T ** result, int bsize);
void r8_mxm_omp (int l, int m, int n, T ** result, int bsize);
void r8_mxm_gold (int l, int m, int n, T ** result, int bsize);
T r8_uniform_01 (int *seed);
int check_matrix (int l, int n, T * a, T * b, T * c);
int compare_T (T f1, T f2);

/******************************************************************************/

int
main (int argc, char *argv[])
/******************************************************************************/
/*
  Purpose:

    MAIN is the main program for MXM.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    19 April 2009

  Author:

    John Burkardt
*/
{
  int id;
  int l;
  int m;
  int n;
  int i;
  int bsize;
  T *a, *b, *c, *result;

  printf ("\n");
  printf ("MXM\n");
  printf ("  C/Llc version.\n");
  printf ("\n");
  printf ("  Matrix multiplication tests.\n");
  if (argc < 3)
    {
      printf ("Error! You must specify the matrix dimension and bsize\n");
      return 1;
    }
  i = strtol (argv[1], NULL, 10);
  bsize = strtol (argv[2], NULL, 10);
  printf ("\n");

  // Deberia llegar hasta 500
  //for (i =500; i <= 500; i+=500) {
  l = m = n = i;
  /*a = malloc (sizeof(T)*l*n);
     b = malloc (sizeof(T)*l*n);
     c = malloc (sizeof(T)*l*n); */

  r8_mxm_llc (l, m, n, &a, bsize);
  r8_mxm_omp (l, m, n, &b, bsize);
  r8_mxm_gold (l, m, n, &c, bsize);
  if (!check_matrix (l, n, a, b, c))
    {
      printf ("Error! With size of problem %d the matrix are not the same\n",
	      i);
      return 1;
    }
  else
    printf ("Verified results for size %d\n", i);
  //}

  printf ("\n");
  printf ("MXM:\n");
  printf ("  Normal end of execution.\n");

  return 0;
}

/******************************************************************************/


int
check_matrix (int l, int n, T * a, T * b, T * c)
{
  int i;
  for (i = 0; i < l * n; i++)
    {
      if (compare_T (a[i], c[i]) + compare_T (b[i], c[i]) < 2)
	{
	  printf
	    ("The element %d in the first matrix is equal to %f but in the second matrix is %f\n",
	     i, a[i], b[i]);
	  int j = i - 3;
	  if (j < 0)
	    j = 0;
	  for (j; j < i + 4; j++)
	    printf ("llc[%d] = %f    \tOpenMP[%d] = %f     \tGold[%d] = %f\n",
		    j, a[j], j, b[j], j, c[j]);
	  return 0;
	}
    }
  return 1;
}


int
compare_T (T f1, T f2)
{
  //T precision = 1e-15;
  T precision = 1e-5;

  if (((f1 - precision) < f2) && ((f1 + precision) > f2))
    {
      return 1;
    }
  else
    {
      return 0;
    }
}




void
r8_mxm_gold (int l, int m, int n, T ** result, int bsize)
/******************************************************************************/
/*
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
*/
{
  T *a;				// [l][n];
  T *b;				// [l][m];
  T *c;				// [m][n];
  int i;
  int j;
  int k;
  double ops;
  double rate;
  int seed;
  CLOCK_TYPE time_start, time_stop;
  double time_elapsed;
/*
  Allocate the matrices.
*/
  a = (T *) malloc (l * n * sizeof (T));
  b = (T *) malloc (l * m * sizeof (T));
  c = (T *) malloc (m * n * sizeof (T));
/*
  Assign values to the B and C matrices.
*/
  seed = 123456789;
  *result = a;

  for (k = 0; k < l * m; k++)
    {
      b[k] = r8_uniform_01 (&seed);
    }

  for (k = 0; k < m * n; k++)
    {
      c[k] = r8_uniform_01 (&seed);
    }
/*
  Compute A = B * C.
*/
//  time_begin = omp_get_wtime ( );

  CLOCK_Start (time_start);
  {
    int ii = 0;
    int jj = 0;
    int kk = 0;
    int tile_size = bsize;
    for (i = ii; i < l; (i++))
      for (j = jj; j < n; (j++))
	{
	  for (k = kk; k < m; (k++))
	    {
	      a[(i * l) + j] = 0.0;
	    }
	}

    /* Iterate over blocks */
    for (ii = 0; ii < l; ii += tile_size)
      {
	for (jj = 0; jj < n; jj += tile_size)
	  for (kk = 0; kk < m; kk += tile_size)
	    {
	      /* Iterate inside a block */
	      for (i = ii; i < min (l, ii + tile_size); (i++))
		for (j = jj; j < min (n, jj + tile_size); (j++))
		  {
		    for (k = kk; k < min (m, kk + tile_size); (k++))
		      {
			a[(i * l) + j] =
			  a[(i * l) + j] + (b[(i * l) + k] * c[(k * m) + j]);
		      }
		  }
	    }
      }

  }
//  time_stop = omp_get_wtime ( );
/*
  Report.
*/
  ops = l * n * (2 * (double) m);
  CLOCK_End (time_start, time_elapsed);
  //printf ("Vamos a ver. Ops %g \t Time %g\n", ops, time_elapsed);
//  time_elapsed = time_stop - time_begin;
  rate = (T) (ops) / time_elapsed / 1000000.0;

  printf ("## Gold Rate: %g Time: %g a[3,10] = %f N=%d\n", rate, time_elapsed,
	  a[3, 10], l);

  //free ( a );
  free (b);
  free (c);

  return;
}















void
r8_mxm_llc (int l, int m, int n, T ** result, int bsize)
/******************************************************************************/
/*
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
*/
{
  T *a;				// [l][n];
  T *b;				// [l][m];
  T *c;				// [m][n];
  int i;
  int j;
  int k;
  double ops;
  double rate;
  int seed;
  CLOCK_TYPE time_start, time_stop;
  double time_elapsed;
/*
  Allocate the matrices.
*/
  a = (T *) malloc (l * n * sizeof (T));
  b = (T *) malloc (l * m * sizeof (T));
  c = (T *) malloc (m * n * sizeof (T));
/*
  Assign values to the B and C matrices.
*/
  seed = 123456789;
  *result = a;

  for (k = 0; k < l * m; k++)
    {
      b[k] = r8_uniform_01 (&seed);
    }

  for (k = 0; k < m * n; k++)
    {
      c[k] = r8_uniform_01 (&seed);
    }
/*
  Compute A = B * C.
*/
//  time_begin = omp_get_wtime ( );

  CLOCK_Start (time_start);
//#pragma omp parallel for shared(a, b, c, l, m, n) private(i, j, k )

int ii,jj,kk;
i = j = k = ii = jj = kk = 0;
int tile_size = bsize;

#pragma acc kernels name("mxm") pcopy(a[n*l]) pcopyin(b[l * m],c[m * n], l, m, n, tile_size, i, j, k, ii,jj,kk)
{
    #pragma acc loop private(i, j) collapse(2)
    for (i = 0; i < l; (i++))
      for (j = 0; j < n; (j++))
	      a[(i * l) + j] = 0.0;
    /* Iterate over blocks */
    for (ii = 0; ii < l; ii += tile_size)
	    for (jj = 0; jj < n; jj += tile_size)
	  for (kk = 0; kk < m; kk += tile_size)
	    {
	      /* Iterate inside a block */
        #pragma acc loop collapse(2)  private(i, j, k) 
	 	 for (j = jj; j < min (n, jj + tile_size); (j++)) 
	      for (i = ii; i < min (l, ii + tile_size); (i++))
           for (k = kk; k < min (m, kk + tile_size); (k++)) {
                            a[(i * l) + j] += (b[(i * l) + k] * c[(k * m) + j]);
                     }
	    }
}

//  time_stop = omp_get_wtime ( );
/*
  Report.
*/
  ops = l * n * (2 * (double) m);
  CLOCK_End (time_start, time_elapsed);
//  time_elapsed = time_stop - time_begin;
  //printf ("Vamos a ver. Ops %d \t Time %g\n", ops, time_elapsed);
  rate = (T) (ops) / time_elapsed / 1000000.0;

  printf ("## Llc Rate: %g Time: %g [3,10] = %f N=%d\n", rate, time_elapsed,
	  a[3, 10], l);

  //free ( a );
  free (b);
  free (c);

  return;
}



/******************************************************************************/

void
r8_mxm_omp (int l, int m, int n, T ** result, int bsize)
/******************************************************************************/
/*
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
*/
{
  T *a;				// [l][n];
  T *b;				// [l][m];
  T *c;				// [m][n];
  int i;
  int j;
  int k;
  double ops;
  double rate;
  int seed;
  CLOCK_TYPE time_start, time_stop;
  double time_elapsed;
/*
  Allocate the matrices.
*/
  a = (T *) malloc (l * n * sizeof (T));
  b = (T *) malloc (l * m * sizeof (T));
  c = (T *) malloc (m * n * sizeof (T));
/*
  Assign values to the B and C matrices.
*/
  *result = a;
  seed = 123456789;

  for (k = 0; k < l * m; k++)
    {
      b[k] = r8_uniform_01 (&seed);
    }

  for (k = 0; k < m * n; k++)
    {
      c[k] = r8_uniform_01 (&seed);
    }
/*
  Compute A = B * C.
*/
//  time_begin = omp_get_wtime ( );

  CLOCK_Start (time_start);

{
    int ii = 0;
    int jj = 0;
    int kk = 0;
    int tile_size = bsize;

//* #pragma omp parallel for shared(a, b, c, l, m, n) private(i, j, k )
    for (i = ii; i < l; (i++))
      for (j = jj; j < n; (j++))
	{
	  for (k = kk; k < m; (k++))
	    {
	      a[(i * l) + j] = 0.0;
	    }
	}

    /* Iterate over blocks */
 //* #pragma omp parallel for shared(a, b, c, l, m, n) private(i, j, k,ii,jj,kk)
    for (ii = 0; ii < l; ii += tile_size)
      {
	for (jj = 0; jj < n; jj += tile_size)
	  for (kk = 0; kk < m; kk += tile_size)
	    {
	      /* Iterate inside a block */
	      for (i = ii; i < min (l, ii + tile_size); (i++))
		for (j = jj; j < min (n, jj + tile_size); (j++))
		  {
		    for (k = kk; k < min (m, kk + tile_size); (k++))
		      {
			a[(i * l) + j] =
			  a[(i * l) + j] + (b[(i * l) + k] * c[(k * m) + j]);
		      }
		  }
	    }
      }

  }



//  time_stop = omp_get_wtime ( );
/*
  Report.
*/
  ops = l * n * (2 * (double) m);
  CLOCK_End (time_start, time_elapsed);
//  time_elapsed = time_stop - time_begin;
  rate = (T) (ops) / time_elapsed / 1000000.0;

/*  printf ( "\n" );
  printf ( "R8_MXM matrix multiplication timing.\n" );
  printf ( "  A(LxN) = B(LxM) * C(MxN).\n" );
  printf ( "  L = %d\n", l );
  printf ( "  M = %d\n", m );
  printf ( "  N = %d\n", n );
  printf ( "  Ting point OPS roughly %d\n", ops );
  printf ( "  Elapsed time dT = %f\n", time_elapsed );
  printf ( "  Rate = MegaOPS/dT = %f\n", rate );*/

#ifdef _OPENMP
  printf ("Omp Rate: %g Time: %g a[3,10] = %f N=%d \n", rate, time_elapsed,
	  a[3, 10], l);
#else
  printf ("Serial Rate: %g Time: %g a[3,10] = %f N=%d \n", rate, time_elapsed,
	  a[3, 10], l);
#endif

  //free ( a );
  free (b);
  free (c);

  return;
}



/******************************************************************************/

T
r8_uniform_01 (int *seed)
/******************************************************************************/
/*
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

    Output, T R8_UNIFORM_01, a new pseudorandom variate, strictly between
    0 and 1.
*/
{
  int k;
  T r;

  k = *seed / 127773;

  *seed = 16807 * (*seed - k * 127773) - k * 2836;

  if (*seed < 0)
    {
      *seed = *seed + 2147483647;
    }

  r = (T) (*seed) * 4.656612875E-10;

  return r;
}
