#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define DEFAULT_N 1000000000

int main(int argc, char **argv) {
    double local, pi_mpi = 0.0, gpi_mpi, w;
    struct timeval start_time, end_time;
    double exe_time_mpi;
    long i, N;
    int MPI_NAME, MPI_NUMPROCESSORS;
    int namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    N = (argc > 1) ? atol(argv[1]) : DEFAULT_N;

    /* MPI EXECUTION */
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &MPI_NUMPROCESSORS);
    MPI_Comm_rank(MPI_COMM_WORLD, &MPI_NAME);
    MPI_Get_processor_name(processor_name, &namelen);

    w = 1.0 / N;

    gettimeofday(&start_time, 0);

    for (i = MPI_NAME; i < N; i += MPI_NUMPROCESSORS) {
        local = (i + 0.5) * w;
        pi_mpi = pi_mpi + 4.0 / (1.0 + local * local);
    }
    MPI_Allreduce(&pi_mpi, &gpi_mpi, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    gettimeofday(&end_time, 0);
    exe_time_mpi = end_time.tv_sec - start_time.tv_sec;
    exe_time_mpi = exe_time_mpi + (end_time.tv_usec - start_time.tv_usec) / 1e6;
    pi_mpi = gpi_mpi * w;

    /* END MPI EXECUTION */

    printf("%d:%f:%f\n", MPI_NAME, exe_time_mpi, pi_mpi);

    MPI_Finalize();

    return 0;
}

