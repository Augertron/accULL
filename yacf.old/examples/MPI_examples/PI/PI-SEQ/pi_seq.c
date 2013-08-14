#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_N 1000000000

int main(int argc, char **argv) {
    double local, pi_seq = 0.0, w;
    struct timeval start_time, end_time;
    double exe_time_seq;
    long i, N;

    N = (argc > 1) ? atol(argv[1]) : DEFAULT_N;
    w = 1.0 / N;

    /* SEQUENTIAL EXECUTION */

    gettimeofday(&start_time, 0);
    
    {
        pi_seq = 0.0;
        for (i = 0; i < N; i++) {
            local = (i + 0.5)*w;
            pi_seq = pi_seq + 4.0/(1.0 + local*local);
        }
    }

    gettimeofday(&end_time, 0);
    exe_time_seq = end_time.tv_sec - start_time.tv_sec;
    exe_time_seq = exe_time_seq + (end_time.tv_usec - start_time.tv_usec) / 1e6;
    pi_seq *= w;

    /* END SEQUENTIAL EXECUTION */

    printf("%d:%f:%f\n", 0, exe_time_seq, pi_seq);

    return 0;
}

