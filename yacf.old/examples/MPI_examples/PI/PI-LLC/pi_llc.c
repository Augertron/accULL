#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_N 1000000000

int
main (int argc, char **argv)
{
  double local, pi_llc = 0.0, w;
  struct timeval start_time, end_time;
  double exe_time_llc;
  long i, N;
  int MPI_NAME, MPI_NUMPROCESSORS;

  N = (argc > 1) ? atol (argv[1]) : DEFAULT_N;
  w = 1.0 / N;

  /* LLC EXECUTION */

  gettimeofday (&start_time, 0);

#pragma omp target device(mpi)
#pragma omp parallel private(i, local)
  {
#pragma omp single
    pi_llc = 0.0;

#pragma omp for reduction (+: pi_llc)
    for (i = 0; i < N; i++)
      {
	local = (i + 0.5) * w;
	pi_llc = pi_llc + 4.0 / (1.0 + local * local);
      }
  }

  gettimeofday (&end_time, 0);
  exe_time_llc = end_time.tv_sec - start_time.tv_sec;
  exe_time_llc = exe_time_llc + (end_time.tv_usec - start_time.tv_usec) / 1e6;
  pi_llc *= w;

  /* END LLC EXECUTION */

  MPI_Comm_size (MPI_COMM_WORLD, &MPI_NUMPROCESSORS);
  MPI_Comm_rank (MPI_COMM_WORLD, &MPI_NAME);

  printf ("%d:%f:%f\n", MPI_NAME, exe_time_llc, pi_llc);

  return 0;
}
