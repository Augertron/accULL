#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include "mytime.h"

#ifndef ASSERT_DEBUG
# define NDEBUG
#endif
#include <assert.h>


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef min
# define min(x,y)	( ((x) < (y)) ? (x) : (y) )
#endif
#ifndef max
# define max(x,y)	( ((x) > (y)) ? (x) : (y) )
#endif
#ifndef abs
# define abs(x)		( ((x) > 0) ? (x) : -(x) )
#endif

#define omp_get_wtime()            0
#define omp_get_max_threads()            4



/* Algorithm parameters: */

#ifndef TASKQ_DEPTH
# define TASKQ_DEPTH 3
#endif

#ifndef ARRAY_SIZE
# define ARRAY_SIZE 1024
#endif

#ifndef REPETITIONS
# define REPETITIONS 1
#endif

#ifndef STRASSEN_CUTOFF
# define STRASSEN_CUTOFF 32
#endif

#ifndef MATMUL_BLOCKSIZE
# define MATMUL_BLOCKSIZE 16
#endif

/* global variables */
int rep = REPETITIONS;
int size = ARRAY_SIZE;
int taskq = TASKQ_DEPTH;
int strass = STRASSEN_CUTOFF;
int block = MATMUL_BLOCKSIZE;
int parallel = TRUE;
int threadreq = 0;

char *check_malloc (int size) {
  char *ptr;

  ptr = (char *) malloc (size); 
  if (ptr == NULL) {
    fprintf (stderr,"Not enough memory. Exit!!!\n");
    exit (-1);
  }
  return ptr;
}

   

void help_message(char *prog_name, char arg_type)
{
    fprintf(stderr, "\n");
    fprintf(stderr, "%s usage:\n", prog_name);
    if (arg_type == 'p' || arg_type == '\0') {
        fprintf(stderr, "\n");
        fprintf(stderr, "  Parameters:\n");
        fprintf(stderr, "    -n<num> : input matrix dimensions (num x num)\n");
        fprintf(stderr, "    -r<num> : number of times to repeat calculations (num >= 1)\n");
        fprintf(stderr, "    -d<num> : recursion depth to employ taskq (num >= 0)\n");
        fprintf(stderr, "    -S<num> : minimum array dimension to use with Strassen's alg. (num >= 2)\n");
	fprintf(stderr, "    -B<num> : array dimension used for blocking matrix multiply (num > 2)\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "  Testing Methods (override above parameters if later in option list):\n");
        fprintf(stderr, "    -c : use correctness testing parameter defaults\n");
        fprintf(stderr, "    -b : use benchmarking parameter defaults\n");
    }
    if (arg_type == 'e' || arg_type == '\0') {
        fprintf(stderr, "\n");
        fprintf(stderr, "  Execution Environment:\n");
        fprintf(stderr, "    -p[<num>] : run in parallel [with num >= 1 threads]\n");
        fprintf(stderr, "    -s        : run sequentially [without parallel directives]\n");
    }
    if (arg_type == '\0') {
        fprintf(stderr, "\n");
        fprintf(stderr, "  Help:\n");
        fprintf(stderr, "    -h : print this help message\n");
    }
    fprintf(stderr, "\n");
}

void process_args(int argc, char *argv[], int *size, int *rep, int *taskq, int *strass, int *block,
                  int *threadreq, int *parallel)
{
    int values, i;

    /* process command line arguments */
    for (i=1; i<argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
              case 'n': /* set array size parameter */
                if (sscanf(&argv[i][2], "%d", size) != 1 || *size < 1) {
                    fprintf(stderr, "%s Warning: argument of -n option unacceptable: %s\n",
                            argv[0], &argv[i][2]);
                    help_message(argv[0], 'p');
                }
                break;
              case 'r': /* set number of repetitions parameter */
                if (sscanf(&argv[i][2], "%d", rep) != 1 || *rep < 1) {
                    fprintf(stderr, "%s Warning: argument of -r option unacceptable: %s\n",
                            argv[0], &argv[i][2]);
                    help_message(argv[0], 'p');
                }
                break;
              case 'd': /* set number of leaf tasks per thread parameter */
                if (sscanf(&argv[i][2], "%d", taskq) != 1 || *taskq < 1) {
                    fprintf(stderr, "%s Warning: argument of -d option unacceptable: %s\n",
                            argv[0], &argv[i][2]);
                    help_message(argv[0], 'p');
                }
                break;
              case 'S': /* set minimum size array dimension to use with Strassen's algorithm */
                if (sscanf(&argv[i][2], "%d", strass) != 1 || *strass < 2) {
                    fprintf(stderr, "%s Warning: argument of -S option unacceptable: %s\n",
                            argv[0], &argv[i][2]);
                    help_message(argv[0], 'p');
                }
                break;
              case 'B': /* set minimum size array dimension to block for regular matrix multiply algorithm */
                if (sscanf(&argv[i][2], "%d", block) != 1 || *block < 2) {
                    fprintf(stderr, "%s Warning: argument of -B option unacceptable: %s\n",
                            argv[0], &argv[i][2]);
                    help_message(argv[0], 'p');
                }
                break;
              case 'c': /* enable correctness parameter defaults */
                *size = 397;
                *rep = 1;
                *taskq = 100;
		*strass = 2; 
		*block = 2;
                break;
              case 'b': /* enable benchmarking parameter defaults */
                *size = 1024;
                *rep = 1;
                *taskq = 3;
		*strass = 32;
		*block = 16;
                break;
              case 's': /* execute serial run */
                *parallel = FALSE;
                *threadreq = 1;
                break;
              case 'p': /* execute parallel run */
                *parallel = TRUE;
                if ((values = sscanf(&argv[i][2], "%d", threadreq)) != 1) {
                    if (values != EOF) {
                        fprintf(stderr, "%s Warning: argument of -p option unacceptable: %s\n",
                                argv[0], &argv[i][2]);
                        help_message(argv[0], 'e');
                        *threadreq = 0;
                    }
                } else if (*threadreq < 1) {
                    fprintf(stderr, "%s Warning: argument of -p option unacceptable: %s\n",
                            argv[0], &argv[i][2]);
                    help_message(argv[0], 'e');
                    *threadreq = 0;
                } else {
                }
                break;
              case 'h': /* print help message */
                help_message(argv[0], '\0');
		exit(0);
                break;
              default:
                    fprintf(stderr, "%s: Warning: command-line option ignored: %s\n",
                            argv[0], argv[i]);
                help_message(argv[0], '\0');
                break;
            }
        } else {
            fprintf(stderr, "%s: Warning: command-line option ignored: %s\n",
                    argv[0], argv[i]);
            help_message(argv[0], '\0');
        }
    }
}

void
matrix_add(
       int n, int m,		     	   /* dimensions of A, B, and C submatrices */
       double *A, int ax, int ay, int as,  /* (ax,ay) = origin of A submatrix for multiplicand */
       double *B, int bx, int by, int bs,  /* (bx,by) = origin of B submatrix for multiplicand */
       double *C, int cx, int cy, int cs   /* (cx,cy) = origin of C submatrix for result */
)
{
    int i, j;

    for (i=0; i<n; i+=1) {
        for (j=0; j<m; j+=1) {
	    C[(i+cx)*cs + j+cy] = A[(i+ax)*as + j+ay] + B[(i+bx)*bs + j+by];
	}
    }
}

void
matrix_sub(
       int n, int m,		            /* dimensions of A, B, and C submatrices */
       double *A, int ax, int ay, int as,   /* (ax,ay) = origin of A submatrix for multiplicand */
       double *B, int bx, int by, int bs,   /* (bx,by) = origin of B submatrix for multiplicand */
       double *C, int cx, int cy, int cs    /* (cx,cy) = origin of C submatrix for result */
)
{
    int i, j;

    for (i=0; i<n; i+=1) {
        for (j=0; j<m; j+=1) {
	    C[(i+cx)*cs + j+cy] = A[(i+ax)*as + j+ay] - B[(i+bx)*bs + j+by];
	}
    }
}

void
matrix_mult(
       int l, int m, int n,    	            /* dimensions of A (lxm), B(mxn), and C(lxn) submatrices */
       double *A, int ax, int ay, int as,   /* (ax,ay) = origin of A submatrix for multiplicand */
       double *B, int bx, int by, int bs,   /* (bx,by) = origin of B submatrix for multiplicand */
       double *C, int cx, int cy, int cs,   /* (cx,cy) = origin of C submatrix for result */
       int d				    /* depth of recursion (for debug only) */
)
{
    int i, j, k;

#if DEBUG 
    #pragma omp critical(debug)
    {
	 for (i = 1; i <= d; i++) LLC_printMaster(" ");
	 LLC_printMaster("matrix_mult: (t%d) (%dx%d)*(%dx%d); depth %d; A[%d..%d][%d..%d](%dx%d) *"
		" B[%d..%d][%d..%d](%dx%d) => C[%d..%d][%d..%d](%dx%d)\n", 
		omp_get_thread_num(), l, m, m, n, d, 
		ax, ax+l-1, ay, ay+m-1, as, as,  
		bx, bx+m-1, by, by+n-1, bs, bs,
		cx, cx+l-1, cy, cy+n-1, cs, cs
	 );
    }
#endif

#ifndef NO_BLOCK_MATMUL
    if (l > block && n > block && m > block) {
	int ib, jb, kb;
	int il, jl, kl; 
	int iu, ju, ku;
	int is, js, ks;
	int ir, jr, kr;

	ir = 1 + (l-1) % block;
	jr = 1 + (n-1) % block;
	kr = 1 + (m-1) % block;

	il = 0;
	is = ir;
	for (ib=0; ib<l; ib+=block) {
	    iu = il + is;
     
	    jl = 0;
	    js = jr;
	    for (jb=0; jb<n; jb+=block) {
		ju = jl + js;

		for (i=il; i<iu; i++) {
		    for (j=jl; j<ju; j++) {
			assert(0 <= i+cx && i+cx < cs);
			assert(0 <= j+cy && j+cy < cs);
			C[(i+cx)*cs + j+cy] = 0.0;
		    }
		}

		kl = 0;
		ks = kr;
		for (kb=0; kb<m; kb+=block) {
		    ku = kl + ks;

		    for (i=il; i<iu; i++) {
			for (j=jl; j<ju; j++) {
			    for (k=kl; k<ku; k++) {
				assert(0 <= i+ax && i+ax < as);
				assert(0 <= k+ay && k+ay < as);
				assert(0 <= k+bx && k+bx < bs);
				assert(0 <= j+by && j+by < bs);
				C[(i+cx)*cs + j+cy] += A[(i+ax)*as + k+ay] * B[(k+bx)*bs + j+by];
			    }
			}
		    }
		    kl = ku;
		    ks = block;
		}
		jl = ju;
		js = block;
	    }
	    il = iu;
	    is = block;
	}
    }
    else {
#endif
	for (i=0; i<l; i++) {
	    for (j=0; j<n; j++) {
		assert(0 <= i+cx && i+cx < cs);
		assert(0 <= j+cy && j+cy < cs);
		C[(i+cx)*cs + j+cy] = 0.0;
		for (k=0; k<m; k++) {
		    assert(0 <= i+ax && i+ax < as);
		    assert(0 <= k+ay && k+ay < as);
		    assert(0 <= k+bx && k+bx < bs);
		    assert(0 <= j+by && j+by < bs);
		    C[(i+cx)*cs + j+cy] += A[(i+ax)*as + k+ay] * B[(k+bx)*bs + j+by];
		}
	    }
	}
#ifndef NO_BLOCK_MATMUL
    }
#endif
}

/* forward decl for mutually-recursive routines */
void 
strassen_mult(
       int n,
       double *A, int ax, int ay, int as,
       double *B, int bx, int by, int bs,
       double *C, int cx, int cy, int cs,
       int d, 
       int qd,
       int s
);			  

/******************************************************************************************\
 * Parallel version of the main computation of strassen_mult().
\******************************************************************************************/
void 
strassen_mult_par(
       int n, 			            /* dimensions of A, B, and C submatrices */
       double *A, int ax, int ay, int as,   /* (ax,ay) = origin of A submatrix for multiplicand */
       double *B, int bx, int by, int bs,   /* (bx,by) = origin of B submatrix for multiplicand */
       double *C, int cx, int cy, int cs,   /* (cx,cy) = origin of C submatrix for result */
       int d, 			            /* current depth of strassen's recursion */
       int qd,		 	            /* target depth for taskq recursion */
       int s			            /* Strassen's recursion limit for array dimensions */
)
{
    int n_2 = n >> 1;
    double *p1, *p2, *p3, *p4, *p5, *p6, *p7;

    p1    = (double *) check_malloc (sizeof(double) * n_2 * n_2 * 7);
    p2    = p1 + n_2 * n_2;
    p3    = p2 + n_2 * n_2;
    p4    = p3 + n_2 * n_2;
    p5    = p4 + n_2 * n_2;
    p6    = p5 + n_2 * n_2;
    p7    = p6 + n_2 * n_2;

    #pragma intel omp taskq 
    {
	#pragma intel omp task
	#pragma llc task_slave_data (p1, (n_2 * n_2))
	{
	    double *a_cum, *b_cum;

	    /* p1 = (a11 + a22) x (b11 + b22) */
	    a_cum = (double *) check_malloc (sizeof(double) * n_2 * n_2 * 2);
	    b_cum = a_cum + n_2 * n_2;
	    matrix_add(n_2, n_2, A, ax, ay, as, A, ax+n_2, ay+n_2, as, a_cum, 0, 0, n_2);
	    matrix_add(n_2, n_2, B, bx, by, bs, B, bx+n_2, by+n_2, bs, b_cum, 0, 0, n_2);
	    strassen_mult(n_2, a_cum, 0, 0, n_2, b_cum, 0, 0, n_2, p1, 0, 0, n_2, d+1, qd, s);
            free (a_cum);
	}

	#pragma intel omp task
	#pragma llc task_slave_data (p2, (n_2 * n_2))
	{
	    double *a_cum;

	    /* p2 = (a21 + a22) x b11 */
	    a_cum = (double *) check_malloc (sizeof(double) * n_2 * n_2);
	    matrix_add(n_2, n_2, A, ax+n_2, ay, as, A, ax+n_2, ay+n_2, as, a_cum, 0, 0, n_2);
	    strassen_mult(n_2, a_cum, 0, 0, n_2, B, bx, by, bs, p2, 0, 0, n_2, d+1, qd, s);
            free (a_cum);
	}
	
	#pragma intel omp task
	#pragma llc task_slave_data (p3, (n_2 * n_2))
	{
	    double *b_cum;

	    /* p3 = a11 x (b12 - b22) */
	    b_cum = (double *) check_malloc (sizeof(double) * n_2 * n_2);
	    matrix_sub(n_2, n_2, B, bx, by+n_2, bs, B, bx+n_2, by+n_2, bs, b_cum, 0, 0, n_2);
	    strassen_mult(n_2, A, ax, ay, as, b_cum, 0, 0, n_2, p3, 0, 0, n_2, d+1, qd, s);
            free (b_cum);
	}

	#pragma intel omp task
	#pragma llc task_slave_data (p4, (n_2 * n_2))
	{
	    double *b_cum;

	    /* p4 = a22 x (b21 - b11) */
	    b_cum = (double *) check_malloc (sizeof(double) * n_2 * n_2);
	    matrix_sub(n_2, n_2, B, bx+n_2, by, bs, B, bx, by, bs, b_cum, 0, 0, n_2);
	    strassen_mult(n_2, A, ax+n_2, ay+n_2, as, b_cum, 0, 0, n_2, p4, 0, 0, n_2, d+1, qd, s);
            free (b_cum);
	}	

	#pragma intel omp task
	#pragma llc task_slave_data (p5, (n_2 * n_2))
	{
	    double *a_cum;

	    /* p5 = (a11 + a12) x b22 */
	    a_cum = (double *) check_malloc (sizeof(double) * n_2 * n_2);
	    matrix_add(n_2, n_2, A, ax, ay, as, A, ax, ay+n_2, as, a_cum, 0, 0, n_2);
	    strassen_mult(n_2, a_cum, 0, 0, n_2, B, bx+n_2, by+n_2, bs, p5, 0, 0, n_2, d+1, qd, s);
            free (a_cum);
	}
	    
	#pragma intel omp task
	#pragma llc task_slave_data (p6, (n_2 * n_2))
	{
	    double *a_cum, *b_cum;

	    /* p6 = (a21 - a11) x (b11 + b12) */
	    a_cum = (double *) check_malloc (sizeof(double) * n_2 * n_2 * 2);
	    b_cum = a_cum + n_2 * n_2;
	    matrix_sub(n_2, n_2, A, ax+n_2, ay, as, A, ax, ay, as, a_cum, 0, 0, n_2);
	    matrix_add(n_2, n_2, B, bx, by, bs, B, bx, by+n_2, bs, b_cum, 0, 0, n_2);
	    strassen_mult(n_2, a_cum, 0, 0, n_2, b_cum, 0, 0, n_2, p6, 0, 0, n_2, d+1, qd, s);
            free (a_cum);
	}

	#pragma intel omp task
	#pragma llc task_slave_data (p7, (n_2 * n_2))
	{
	    double *a_cum, *b_cum;

	    /* p7 = (a12 - a22) x (b21 + b22) */
	    a_cum = (double *) check_malloc (sizeof(double) * n_2 * n_2 * 2);
	    b_cum = a_cum + n_2 * n_2;
	    matrix_sub(n_2, n_2, A, ax, ay+n_2, as, A, ax+n_2, ay+n_2, as, a_cum, 0, 0, n_2);
	    matrix_add(n_2, n_2, B, bx+n_2, by, bs, B, bx+n_2, by+n_2, bs, b_cum, 0, 0, n_2);
	    strassen_mult(n_2, a_cum, 0, 0, n_2, b_cum, 0, 0, n_2, p7, 0, 0, n_2, d+1, qd, s);
            free (a_cum);
	}
    }

    /* c11 = p1 + p4 - p5 + p7 */
    matrix_add(n_2, n_2, p1, 0, 0, n_2, p4, 0, 0, n_2, C, cx, cy, cs);
    matrix_sub(n_2, n_2, C, cx, cy, cs, p5, 0, 0, n_2, C, cx, cy, cs);
    matrix_add(n_2, n_2, C, cx, cy, cs, p7, 0, 0, n_2, C, cx, cy, cs);

    /* c12 = p3 + p5 */
    matrix_add(n_2, n_2, p3, 0, 0, n_2, p5, 0, 0, n_2, C, cx, cy+n_2, cs);

    /* c21 = p2 + p4 */
    matrix_add(n_2, n_2, p2, 0, 0, n_2, p4, 0, 0, n_2, C, cx+n_2, cy, cs);

    /* c22 = p1 + p3 - p2 + p6 */
    matrix_add(n_2, n_2, p1, 0, 0, n_2, p3, 0, 0, n_2, C, cx+n_2, cy+n_2, cs);
    matrix_sub(n_2, n_2, C, cx+n_2, cy+n_2, cs, p2, 0, 0, n_2, C, cx+n_2, cy+n_2, cs);
    matrix_add(n_2, n_2, C, cx+n_2, cy+n_2, cs, p6, 0, 0, n_2, C, cx+n_2, cy+n_2, cs);
   
    free (p1);
}

/******************************************************************************************\
 * Perform A x B => C  for square submatrices of A, B, and C assuming the submatrix 
 * dimension is divisible by two.
 *
 *      n_2 = n/2 = order of partitioned matrices
 *
 *          ------------- -------------   -------------
 *      n/2 ! a11 ! a12 ! ! b11 ! b12 !   ! c11 ! c12 ! n/2
 *          !-----!-----!*!-----!-----! = !-----!-----!
 *      n/2 ! a21 ! a22 ! ! b21 ! b22 !   ! c21 ! c22 ! n/2
 *          ------------- -------------   -------------
 *            n/2   n/2     n/2   n/2       n/2   n/2
 *
 *      algorithm:
 *
 *      p1 = (a11+a22)*(b11+b22)
 *      p2 = (a21+a22)*b11
 *      p3 = a11*(b12-b22)
 *      p4 = a22*(b21-b11)
 *      p5 = (a11+a12)*b22
 *      p6 = (a21-a11)*(b11+b12)
 *      p7 = (a12-a22)*(b21+b22)
 *
 *      c11 = p1+p4-p5+p7
 *      c12 = p3+p5
 *      c21 = p2+p4
 *      c22 = p1+p3-p2+p6
 *
 *      where each matrix multiplication is implemented as
 *      a recursive call to strassen_mult.
 *
 * 
\******************************************************************************************/

void 
strassen_mult(
       int n, 			     	    /* dimensions of A, B, and C submatrices */
       double *A, int ax, int ay, int as,   /* (ax,ay) = origin of A submatrix for multiplicand */
       double *B, int bx, int by, int bs,   /* (bx,by) = origin of B submatrix for multiplicand */
       double *C, int cx, int cy, int cs,   /* (cx,cy) = origin of C submatrix for result */
       int d, 			            /* current depth of strassen's recursion */
       int qd,		 	            /* target depth for taskq recursion */
       int s			            /* Strassen's recursion limit for array dimensions */
)
{
    if (n < s) {

	matrix_mult(n, n, n, A, ax, ay, as, B, bx, by, bs, C, cx, cy, cs, d);

    }
    else {

	    int n_2 = n >> 1;
	    double *work;
	    double *a_cum, *b_cum;
	    double *p1, *p2, *p3, *p4, *p5, *p6, *p7;

	    work  = (double *) check_malloc (sizeof(double) * n_2 * n_2 * 9);
	    a_cum = work; 
	    b_cum = a_cum + n_2 * n_2;
	    p1    = b_cum + n_2 * n_2;
	    p2    = p1 + n_2 * n_2;
	    p3    = p2 + n_2 * n_2;
	    p4    = p3 + n_2 * n_2;
	    p5    = p4 + n_2 * n_2;
	    p6    = p5 + n_2 * n_2;
	    p7    = p6 + n_2 * n_2;

  	    /* p1 = (a11 + a22) x (b11 + b22) */
	    matrix_add(n_2, n_2, A, ax, ay, as, A, ax+n_2, ay+n_2, as, a_cum, 0, 0, n_2);
	    matrix_add(n_2, n_2, B, bx, by, bs, B, bx+n_2, by+n_2, bs, b_cum, 0, 0, n_2);
	    strassen_mult(n_2, a_cum, 0, 0, n_2, b_cum, 0, 0, n_2, p1, 0, 0, n_2, d+1, qd, s);

	    /* p2 = (a21 + a22) x b11 */
	    matrix_add(n_2, n_2, A, ax+n_2, ay, as, A, ax+n_2, ay+n_2, as, a_cum, 0, 0, n_2);
	    strassen_mult(n_2, a_cum, 0, 0, n_2, B, bx, by, bs, p2, 0, 0, n_2, d+1, qd, s);
	    
	    /* p3 = a11 x (b12 - b22) */
	    matrix_sub(n_2, n_2, B, bx, by+n_2, bs, B, bx+n_2, by+n_2, bs, b_cum, 0, 0, n_2);
	    strassen_mult(n_2, A, ax, ay, as, b_cum, 0, 0, n_2, p3, 0, 0, n_2, d+1, qd, s);

	    /* p4 = a22 x (b21 - b11) */
	    matrix_sub(n_2, n_2, B, bx+n_2, by, bs, B, bx, by, bs, b_cum, 0, 0, n_2);
	    strassen_mult(n_2, A, ax+n_2, ay+n_2, as, b_cum, 0, 0, n_2, p4, 0, 0, n_2, d+1, qd, s);

	    /* p5 = (a11 + a12) x b22 */
	    matrix_add(n_2, n_2, A, ax, ay, as, A, ax, ay+n_2, as, a_cum, 0, 0, n_2);
	    strassen_mult(n_2, a_cum, 0, 0, n_2, B, bx+n_2, by+n_2, bs, p5, 0, 0, n_2, d+1, qd, s);
	    
	    /* p6 = (a21 - a11) x (b11 + b12) */
	    matrix_sub(n_2, n_2, A, ax+n_2, ay, as, A, ax, ay, as, a_cum, 0, 0, n_2);
	    matrix_add(n_2, n_2, B, bx, by, bs, B, bx, by+n_2, bs, b_cum, 0, 0, n_2);
	    strassen_mult(n_2, a_cum, 0, 0, n_2, b_cum, 0, 0, n_2, p6, 0, 0, n_2, d+1, qd, s);

	    /* p7 = (a12 - a22) x (b21 + b22) */
	    matrix_sub(n_2, n_2, A, ax, ay+n_2, as, A, ax+n_2, ay+n_2, as, a_cum, 0, 0, n_2);
	    matrix_add(n_2, n_2, B, bx+n_2, by, bs, B, bx+n_2, by+n_2, bs, b_cum, 0, 0, n_2);
	    strassen_mult(n_2, a_cum, 0, 0, n_2, b_cum, 0, 0, n_2, p7, 0, 0, n_2, d+1, qd, s);

	    /* c11 = p1 + p4 - p5 + p7 */
	    matrix_add(n_2, n_2, p1, 0, 0, n_2, p4, 0, 0, n_2, C, cx, cy, cs);
	    matrix_sub(n_2, n_2, C, cx, cy, cs, p5, 0, 0, n_2, C, cx, cy, cs);
	    matrix_add(n_2, n_2, C, cx, cy, cs, p7, 0, 0, n_2, C, cx, cy, cs);

	    /* c12 = p3 + p5 */
	    matrix_add(n_2, n_2, p3, 0, 0, n_2, p5, 0, 0, n_2, C, cx, cy+n_2, cs);

	    /* c21 = p2 + p4 */
	    matrix_add(n_2, n_2, p2, 0, 0, n_2, p4, 0, 0, n_2, C, cx+n_2, cy, cs);

	    /* c22 = p1 + p3 - p2 + p6 */
	    matrix_add(n_2, n_2, p1, 0, 0, n_2, p3, 0, 0, n_2, C, cx+n_2, cy+n_2, cs);
	    matrix_sub(n_2, n_2, C, cx+n_2, cy+n_2, cs, p2, 0, 0, n_2, C, cx+n_2, cy+n_2, cs);
	    matrix_add(n_2, n_2, C, cx+n_2, cy+n_2, cs, p6, 0, 0, n_2, C, cx+n_2, cy+n_2, cs);
	 
            free (work);
    }
    return;
}


/******************************************************************************************\
 * Perform A x B => C  for square A, B, and C matrices.
 *
 * First, decompose as follows:
 * 
 *             ss    bs      ss    bs        ss    bs
 *          ------------- -------------   -------------
 *       ss ! a11 ! a12 ! ! b11 ! b12 !   ! c11 ! c12 ! ss
 *          !-----!-----!*!-----!-----! = !-----!-----!
 *       bs ! a21 ! a22 ! ! b21 ! b22 !   ! c21 ! c22 ! bs
 *          ------------- -------------   -------------
 *             ss    bs      ss    bs        ss    bs
 *
 *
 * Where ss = the maximum value <= n such that ss is divisible by 2 ^ fd
 *       bs = the remaining border dimension for a22, b22, and c22
 * 
 *       we are assuming that bs << ss to maximize parallelism
 *
 * Then:
 *       c11 = strassen(a11 x b11) + (a12 x b21) 
 *
 *       c12 = (a11,a12) x (b12)
 *                         (b22)
 *
 *       c21 = (a21,a22) x (b11)
 *                         (b21)
 *
 *       c22 = (a21,a22) x (b12)
 *                         (b22)
 *
\******************************************************************************************/

void 
strassen_main_par(
       int n, 	     	/* dimensions of A, B, and C submatrices */
       double *A,	/* A submatrix for multiplicand */
       double *B,	/* B submatrix for multiplicand */
       double *C,	/* C submatrix for result */
       int qd,		/* target depth for taskq recursion */
       int s,		/* Strassen's recursion limit for array dimensions */
       int fd 		/* depth of recursion for strassen's algorithm */
)
{
  int ss; 	/* strassen's piece size */
  int bs; 	/* remaining border size */    
  double *tmp; 

  ss = (n>>fd)<<fd;
  bs = n - ss;

#if DEBUG
     LLC_printMaster("strassen_main: (t%d) (%dx%d)*(%dx%d); depth %d; A[%d..%d][%d..%d](%dx%d) *"
	    " B[%d..%d][%d..%d](%dx%d) => C[%d..%d][%d..%d](%dx%d)\n", 
	    omp_get_thread_num(), n, n, n, n, 0, 
	    0, size-1, 0, size-1, size, size,  
	    0, size-1, 0, size-1, size, size,
	    0, size-1, 0, size-1, size, size
     );
     LLC_printMaster("               strassen size = (%dx%d); strassen ops ~= %d;"
            " matmul ops ~= %d\n", 
            ss, ss, ss*ss*ss, ss*bs*ss + 2*ss*n*bs + bs*n*bs);
#endif

  if (bs == 0) {
		/* compute C <= A x B (in parallel) */
    strassen_mult_par(ss, A, 0, 0, size, B, 0, 0, size, C, 0, 0, size, 1, qd, s);
  } 
  else {
    tmp = (double *) check_malloc (ss * ss * sizeof(double));

		    /* compute c11 <= a11 x b11 (in parallel) */
		strassen_mult_par(ss, A, 0, 0, size, B, 0, 0, size, C, 0, 0, size, 1, qd, s);

    #pragma intel omp taskq 
    {
	      /* compute tmp <= a12 x b21 */
      #pragma intel omp task 
      #pragma llc task_slave_data (&(tmp[0]), (ss * ss)) 
		  {
        matrix_mult(ss, bs, ss, A, 0, ss, size, B, ss, 0, size, tmp, 0, 0, ss, 1); 
      }

		    /* compute c12 <= (a11,a12) x (b12,b22)T */
      #pragma intel omp task 
      #pragma llc task_slave_rnc_data (&(C[ss]), bs, (size - bs), ss) 
      {
        matrix_mult(ss, n, bs, A, 0, 0, size, B, 0, ss, size, C, 0, ss, size, 1);
      }

        /* compute c21 <= (a21,a22) x (b11,b21)T */
      #pragma intel omp task 
      #pragma llc task_slave_rnc_data (&(C[ss*size]), ss, (size - ss), bs) 
      {
        matrix_mult(bs, n, ss, A, ss, 0, size, B, 0, 0, size, C, ss, 0, size, 1);
      }

		    /* compute c22 <= (a21,a22) x (b12,b22)T */
      #pragma intel omp task 
      #pragma llc task_slave_rnc_data (&(C[(ss*size)+ss]), bs, (size - bs), bs) 
      {
        matrix_mult(bs, n, bs, A, ss, 0, size, B, 0, ss, size, C, ss, ss, size, 1);
      }
		}

		/* compute c11 <= c11 + tmp */
		matrix_add(ss, ss, C, 0, 0, size, tmp, 0, 0, ss, C, 0, 0, size);
               
    free (tmp);
	}
}

void strassen_main_serial(
       int n, 	     	/* dimensions of A, B, and C submatrices */
       double *A,	/* A submatrix for multiplicand */
       double *B,	/* B submatrix for multiplicand */
       double *C,	/* C submatrix for result */
       int qd,		/* target depth for taskq recursion */
       int s,		/* Strassen's recursion limit for array dimensions */
       int fd 		/* depth of recursion for strassen's algorithm */
)
{
    int ss; 	/* strassen's piece size */
    int bs; 	/* remaining border size */    
    double *tmp; 

    ss = (n>>fd)<<fd;
    bs = n - ss;

#if DEBUG
     LLC_printMaster("strassen_main: (t%d) (%dx%d)*(%dx%d); depth %d; A[%d..%d][%d..%d](%dx%d) *"
	    " B[%d..%d][%d..%d](%dx%d) => C[%d..%d][%d..%d](%dx%d)\n", 
	    omp_get_thread_num(), n, n, n, n, 0, 
	    0, size-1, 0, size-1, size, size,  
	    0, size-1, 0, size-1, size, size,
	    0, size-1, 0, size-1, size, size
     );
     LLC_printMaster("               strassen size = (%dx%d); strassen ops ~= %d;"
            " matmul ops ~= %d\n", 
            ss, ss, ss*ss*ss, ss*bs*ss + 2*ss*n*bs + bs*n*bs);
#endif

	    if (bs == 0) {
		/* compute C <= A x B (in parallel) */
		strassen_mult(ss, A, 0, 0, size, B, 0, 0, size, C, 0, 0, size, 1, qd, s);

	    } else {
		tmp = (double *) check_malloc (ss * ss * sizeof(double));

		{
		    /* compute c11 <= a11 x b11 (in parallel) */
		    {
			strassen_mult(ss, A, 0, 0, size, B, 0, 0, size, C, 0, 0, size, 1, qd, s);
		    }

		    /* compute tmp <= a12 x b21 */
		    {
			matrix_mult(ss, bs, ss, A, 0, ss, size, B, ss, 0, size, tmp, 0, 0, ss, 1); 
		    }

		    /* compute c12 <= (a11,a12) x (b12,b22)T */
		    {
			matrix_mult(ss, n, bs, A, 0, 0, size, B, 0, ss, size, C, 0, ss, size, 1);
		    }

		    /* compute c21 <= (a21,a22) x (b11,b21)T */
		    {
			matrix_mult(bs, n, ss, A, ss, 0, size, B, 0, 0, size, C, ss, 0, size, 1);
		    }

		    /* compute c22 <= (a21,a22) x (b12,b22)T */
		    {
			matrix_mult(bs, n, bs, A, ss, 0, size, B, 0, ss, size, C, ss, ss, size, 1);
		    }
		}

		/* compute c11 <= c11 + tmp */
		matrix_add(ss, ss, C, 0, 0, size, tmp, 0, 0, ss, C, 0, 0, size);
               
                free (tmp);
	    }
}



int main(int argc, char *argv[]) { 
  int i, j, r, qdepth, numthreads, fulldepth;
  double rel_error_serial, max_error_serial;
  double rel_error_taskq, max_error_taskq;
  double *A, *B, *C, *D, *E;
  CLOCK_TYPE chrono;
  double mult_time, serial_time, taskq_time;
  double mult_time_part, serial_time_part, taskq_time_part;


  process_args(argc, argv, &size, &rep, &taskq, &strass, &block, &threadreq, &parallel);

    /* for allocation of thread stacks */
#ifdef _OPENMP
    if (parallel) kmp_set_stacksize( max( (3*size*size + 128*size) * sizeof(double), 1024*1024 ) );
#endif

    /* allocate array storage */
  A = (double *) check_malloc (size * size * sizeof(double));
  B = (double *) check_malloc (size * size * sizeof(double));
  C = (double *) check_malloc (size * size * sizeof(double));
  D = (double *) check_malloc (size * size * sizeof(double));
  E = (double *) check_malloc (size * size * sizeof(double));

  /* set up input matrices with random values */
  srand((unsigned int)time(NULL));

  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++) {
      A[i*size + j] = ((float) rand()) / ((float) RAND_MAX);
      B[i*size + j] = ((float) rand()) / ((float) RAND_MAX);
    }
  }
  numthreads = LLC_NUMPROCESSORS;
  fulldepth = 0;
  for(i=size; i>=strass; i>>=1) fulldepth++;

  if (parallel) {
      if (threadreq == 0) threadreq = numthreads;
      qdepth = min(taskq, fulldepth);
  } else {
      qdepth = 0;
  }

    /* calculate runtime parameters */

  LLC_printMaster("\n");
  LLC_printMaster("Parameters:\n");
  LLC_printMaster("  array size: %d x %d\n", size, size);
  LLC_printMaster("  threads: %d\n", threadreq);
  LLC_printMaster("  taskq depth: %d\n", qdepth);
  LLC_printMaster("  repetitions: %d\n", rep);
  LLC_printMaster("  strassen minimum: %d\n", strass);
  LLC_printMaster("  matmul blocksize: %d\n", block);
  LLC_printMaster("\n");
  fflush(stdout);

  mult_time = 0.0;
  serial_time = 0.0;
  taskq_time = 0.0;


  for (r = 1; r <= rep; r++) {

	/* LLC_printMaster("\nRepetition: %d\n", r); */


    /* do regular matrix multiply A * B => C for result comparision */
    CLOCK_Start(chrono);
    matrix_mult(size, size, size, A, 0, 0, size, B, 0, 0, size, C, 0, 0, size, 0);
    CLOCK_End(chrono, mult_time_part);
    mult_time += mult_time_part;

    CLOCK_Start(chrono);
    /* do strassen serial matrix multiply A * B => D */
    strassen_main_serial(size, A, B, D, qdepth, strass, fulldepth);
    CLOCK_End(chrono, serial_time_part);
    serial_time += serial_time_part;

    CLOCK_Start(chrono);
	  /* do strassen parallel matrix multiply A * B => E */
    strassen_main_par(size, A, B, E, qdepth, strass, fulldepth);
    CLOCK_End(chrono, taskq_time_part);
    taskq_time += taskq_time_part;


	/* calculate largest relative error */
    max_error_serial = 0.0;
    max_error_taskq = 0.0;
    for (i = 0; i < size; i++) {
      for (j = 0; j < size; j++) {
        rel_error_serial = abs(C[i*size + j] - D[i*size + j]) / 
                     ((abs(C[i*size + j]) + abs(D[i*size + j])) / 2.0);
        rel_error_taskq = abs(C[i*size + j] - E[i*size + j]) / 
                     ((abs(C[i*size + j]) + abs(E[i*size + j])) / 2.0);
#ifdef PRINT_ERROR
        if (i == j) {
		      LLC_printMaster("C[%d][%d] = %18.16f, E[%d][%d] = %18.16f, rel_error = %20.18f\n", 
                  i, j, C[i*size+j], i, j, E[i*size+j], rel_error );
        }
#endif
        max_error_serial = max(max_error_serial, rel_error_serial);
        max_error_taskq = max(max_error_taskq, rel_error_taskq);
      }
    }

    LLC_printMaster("\n");
    LLC_printMaster("Maximum relative error for Strassen's multiply (Serial): %6.2e\n", max_error_serial);
    LLC_printMaster("Maximum relative error for Strassen's multiply (Taskq):  %6.2e\n", max_error_taskq);
    fflush(stdout);
    if (max_error_taskq >= 1e-9) {
      LLC_printMaster("\n*** ERROR!!!: Run Failed! ***:\n");
    }
    else {
      LLC_printMaster("Run Successful:\n");
    }
  }

  free (A);
  free (B);
  free (C);
  free (D);
  free (E);

  LLC_printMaster("\n");
  LLC_printMaster("Run Successful:\n");
  LLC_printMaster("\n");
  LLC_printMaster("Maximum relative error for Strassen's multiply (Serial): %6.2e\n", max_error_serial);
  LLC_printMaster("Maximum relative error for Strassen's multiply (Taskq):  %6.2e\n", max_error_taskq);

	LLC_printMaster("  Serial Matmul Compute time:     %f seconds\n", mult_time);
 	LLC_printMaster("  Serial Strassen's Compute time: %f seconds\n", serial_time);
	LLC_printMaster("  Taskq  Strassen's Compute time: %f seconds\n", taskq_time);
  LLC_printMaster("\n");

  LLC_printMaster ("%d\t%g\t#llc_plot0; n = %d, rep = %d  (t_mult=%g/t_serial=%g)\n",
            LLC_NUMPROCESSORS, (mult_time / serial_time), size, rep, mult_time, serial_time);
  LLC_printMaster ("%d\t%g\t#llc_plot1; n = %d, rep = %d  (t_mult=%g/t_taskq=%g)\n",
            LLC_NUMPROCESSORS, (mult_time / taskq_time), size, rep, mult_time, taskq_time);
  LLC_printMaster ("%d\t%g\t#llc_plot2; n = %d, rep = %d  (t_serial=%g/t_taskq=%g)\n",
            LLC_NUMPROCESSORS, (serial_time / taskq_time), size, rep, serial_time, taskq_time);

  return 0;
}

