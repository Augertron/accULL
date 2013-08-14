#include "llcomp_llc.h"
double amult;
double tran;
double time1;
double time2;
double second ();
double a[2030000];
double x[14000];
double z[14000];
double r[14000];
double p[14000];
double q[14000];
int colstr[14000 + 1];
int rowidx[2030000];
char *datafile = "data";
double cgsol ();
int
llc_main (int argc, char **argv)
{

  int nn;
  int nnp1;
  int nnz;
  int lenwrk;
  int ilnwrk;
  int niter;
  int nitcg;
  double rcond;
  double shift;
  double resid;
  double zeta;
  double ztz;
  double znorminv;
  int i;
  int it;
  int nnzcom;
  int imax;
  double randlc ();
  int nnzchk;
  double zetchk;
  double zettol;
  double reschk;
  nn = 14000;
  LLC_printMaster
    ("\n************************ NUM PROCS = %d *****************************\n",
     LLC_NUMPROCESSORS);
  tran = 314159265.0;
  amult = 1220703125.0;
  if (LLC_NAME == 0)
    {

      if (argc == 2)
	datafile = argv[1];;
      dataread (nn, a, rowidx, colstr);
    }

  ;
  if (LLC_NUMPROCESSORS > 1)
    {

      MPI_Bcast ((&nn), 1, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Bcast (colstr, nn + 1, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Bcast (rowidx, colstr[nn], MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Bcast (a, colstr[nn], MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

  ;
  for (i = 0; i < nn; (i++))
    x[i] = 1.0;
  time1 = second ();
  for (it = 0; it < 15; (it++))
    {

      resid = cgsol (nn, a, rowidx, colstr, x, (&zeta));
    }

  ;
  time2 = second ();
  printf ("NAME = %d, NP = %d. SOL: resid = %g, zeta = %g\n", LLC_NAME,
	  LLC_NUMPROCESSORS, resid, zeta);
  printf
    ("NAME = %d, NP = %d. TIME => total = %g, iter = %g [(%g - %g)/%d], NUM_ITER_CG=%d\n",
     LLC_NAME, LLC_NUMPROCESSORS, time2 - time1, (time2 - time1) / 15, time2,
     time1, 15, 25);
  LLC_printMaster
    ("%d\t%g\t#llc_plot0 CG:NONZEROS = %d, iter=%d, iterCG=%d (t_seq=%g/t_par=%g)\n",
     LLC_NUMPROCESSORS, 0.0 / (time2 - time1), 1853104, 15, 25, 0.0,
     time2 - time1);
  return 0;
}

;
double
dotpro (int n, double *x, double *y)
{

  double z = 0.0;
  int i;
#pragma omp
  ;

  {

    loopParallelFor0_support (y, x, (&z), (&n), (&i));
  }

  ;
  return z;
}

;
double
cgsol (int n, double *a, int *rowidx, int *colstr, double *x, double *zeta)
{

  int it;
  int cols;
  int i;
  double alpha;
  double beta;
  double rho;
  double rho0;
  double znorminv;
  cols = n;
#pragma omp
  ;

  {

    loopParallelFor1_support (x, r, p, (&cols), (&i), z);
  }

  ;
  rho = dotpro (cols, x, x);
  for (it = 0; it < 25; (it++))
    {

      matvec (a, colstr, rowidx, p, q, cols);
      alpha = rho / dotpro (cols, p, q);
#pragma omp
      ;

      {

	loopParallelFor2_support (p, r, (&cols), q, (&i), (&alpha), z);
      }

      ;
      rho0 = rho;
      rho = dotpro (cols, r, r);
      beta = rho / rho0;
#pragma omp
      ;

      {

	loopParallelFor3_support ((&cols), p, (&beta), r, (&i));
      }

      ;
    }

  ;
  matvec (a, colstr, rowidx, z, r, cols);
#pragma omp
  ;

  {

    loopParallelFor4_support (x, (&cols), r, (&i));
  }

  ;
  *(zeta) = 1.0 / dotpro (cols, x, z);
  znorminv = 1.0 / sqrt (dotpro (cols, z, z));
#pragma omp
  ;

  {

    loopParallelFor5_support ((&cols), x, z, (&i), (&znorminv));
  }

  ;
  return sqrt (dotpro (cols, r, r));
}

;
int
matvec (double *a, int *row_start, int *col_idx, double *x, double *y, int nn)
{

  int i;
  int j;
  int start;
  int end;
  double t;
#pragma omp
  ;

  {

    loopParallelFor6_support ((&t), y, (&i), x, a, (&j), (&end), (&start),
			      (&nn), row_start, col_idx);
  }

  ;
}

;
int
dataread (int nn, double *a, int *rowidx, int *colstr)
{

  FILE *fp;
  int n;
  int ok_flag = 1;
  if ((fp = fopen (datafile, "r")) == (NULL))
    {

      printf ("cannot open 'data' file\n");
      exit (1);
    }

  ;
  ok_flag &= fread ((&n), sizeof (int), 1, fp) == 1;
  if (n != nn)
    {

      fprintf (stderr, "illegal data size.. file %d, prog %d\n", n, nn);
      exit (1);
    }

  ;
  ok_flag &= fread (colstr, sizeof (int), nn + 1, fp) == (nn + 1);
  ok_flag &= fread (rowidx, sizeof (int), colstr[nn], fp) == colstr[nn];
  ok_flag &= fread (a, sizeof (double), colstr[nn], fp) == colstr[nn];
  if ((!ok_flag))
    {

      fprintf (stderr, "fread error\n");
      exit (1);
    }

  ;
  close (fp);
}

;
int
main (int argc, char **argv)
{

  int llc_return_code = 0;
  MPI_Init ((&argc), (&argv));
  MPI_Comm_rank (MPI_COMM_WORLD, (int *) ((&LLC_GLOBAL_NAME)));
  MPI_Comm_size (MPI_COMM_WORLD, (int *) ((&LLC_GLOBAL_NUMPROCESSORS)));
  LLC_NAME = LLC_GLOBAL_NAME;
  LLC_NUMPROCESSORS = LLC_GLOBAL_NUMPROCESSORS;
  llc_GlobalGroup = MPI_COMM_WORLD;
  llc_CurrentGroup = (&llc_GlobalGroup);
  llc_return_code = llc_main (argc, argv);
  MPI_Finalize ();
  return llc_return_code;
}

;
