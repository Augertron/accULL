#include "llcomp_llc.h"
 int main(  int argc,  char *argv []) ;
 void r8_mxm(  int l,  int m,  int n) ;
 double r8_uniform_01(  int *seed) ;
 int llc_main(  int argc,  char *argv []) 
{

      int   l = 500;
      int   m = 500;
      int   n = 500;
r8_mxm  (l  ,m  ,n  ) ;
  return 0;
}

;
 double r8_uniform_01(  int *seed) 
{

      int   k;
      double   r;
k= *(seed)  / 127773;
*(seed) = (16807 * (*(seed)  - (k * 127773))) - (k * 2836);
  if   (*(seed)  < 0  ) *(seed) = *(seed)  + 2147483647  ;;
r= (double )(*(seed) ) * 4.656612875E-10;
  return r;
}

;
 void r8_mxm(  int l,  int m,  int n) 
{

      double   *  a;
      double   *  b;
      double   *  c;
      int   i;
      int   j;
      int   k;
      double   R;
      int   seed;
      int   MPI_NAME;
      int   MPI_NUMPROCESSORS;
      struct timeval    start_time;
      struct timeval    end_time;
      double   time;
b= (double *)(malloc((l * m) * sizeof( double ) ) );
a= (double *)(malloc((m * n) * sizeof( double ) ) );
c= (double *)(malloc((l * n) * sizeof( double ) ) );
seed= 123456789;
  for (k= 0; k < (l * m); (k++ )) a[k]= r8_uniform_01((&seed ))  ;
  for (k= 0; k < (m * n); (k++ )) b[k]= r8_uniform_01((&seed ))  ;
gettimeofday  ((&start_time )  ,0  ) ;
  #pragma omp  
;
  
{

loopParallelFor0_support    ((&i )    ,a    ,(&l )    ,(&m )    ,(&n )    ,(&k )    ,c    ,(&R )    ,(&j )    ,b    ) ;
  }

;
gettimeofday  ((&end_time )  ,0  ) ;
time= end_time.tv_sec  - start_time.tv_sec ;
time= time + ((end_time.tv_usec  - start_time.tv_usec ) / 1e6);
MPI_Comm_size  (MPI_COMM_WORLD  ,(&MPI_NUMPROCESSORS )  ) ;
MPI_Comm_rank  (MPI_COMM_WORLD  ,(&MPI_NAME )  ) ;
printf  ("%d:%f\n"  ,MPI_NAME  ,time  ) ;
free  (a  ) ;
free  (b  ) ;
free  (c  ) ;
  return ;
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
