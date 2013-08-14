#include "llcomp_llc.h"
 int llc_main(  int argc,  char **argv) 
{

      double   local;
      double   pi_llc = 0.0;
      double   w;
      struct timeval    start_time;
      struct timeval    end_time;
      double   exe_time_llc;
      long   i;
      long   N;
      int   MPI_NAME;
      int   MPI_NUMPROCESSORS;
N= ((argc > 1)?atol(argv[1]) :1000000000);
w= 1.0 / N;
gettimeofday  ((&start_time )  ,0  ) ;
  #pragma omp  
;
  
{

    #pragma omp    
;
pi_llc= 0.0;
    
{

loopParallel00_support      ((&local )      ,(&N )      ,(&pi_llc )      ,(&i )      ,(&w )      ) ;
    }

;
  }

;
gettimeofday  ((&end_time )  ,0  ) ;
exe_time_llc= end_time.tv_sec  - start_time.tv_sec ;
exe_time_llc= exe_time_llc + ((end_time.tv_usec  - start_time.tv_usec ) / 1e6);
pi_llc*= w;
MPI_Comm_size  (MPI_COMM_WORLD  ,(&MPI_NUMPROCESSORS )  ) ;
MPI_Comm_rank  (MPI_COMM_WORLD  ,(&MPI_NAME )  ) ;
printf  ("%d:%f:%f\n"  ,MPI_NAME  ,exe_time_llc  ,pi_llc  ) ;
  return 0;
}

;
 int main(  int argc,  char **argv) 
{

      int   llc_return_code = 0;
MPI_Init  ((&argc )  ,(&argv )  ) ;
MPI_Comm_rank  (MPI_COMM_WORLD  ,(int *)((&LLC_GLOBAL_NAME ))  ) ;
MPI_Comm_size  (MPI_COMM_WORLD  ,(int *)((&LLC_GLOBAL_NUMPROCESSORS ))  ) ;
LLC_NAME= LLC_GLOBAL_NAME;
LLC_NUMPROCESSORS= LLC_GLOBAL_NUMPROCESSORS;
llc_GlobalGroup= MPI_COMM_WORLD;
llc_CurrentGroup= (&llc_GlobalGroup );
llc_return_code= llc_main(argc,argv) ;
MPI_Finalize  (   ) ;
  return llc_return_code;
}

;
