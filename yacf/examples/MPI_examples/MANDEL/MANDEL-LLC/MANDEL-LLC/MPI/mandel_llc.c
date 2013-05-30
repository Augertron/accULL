#include "llcomp_llc.h"
typedef  struct timeval  CLOCK_TYPE;
 double CLOCK_Avg(  double t) ;
 double CLOCK_Min(  double t) ;
 double CLOCK_Max(  double t) ;
 float uni(void ) ;
 void rinit(int ) ;
typedef  struct {  double creal;  double cimag;}  complex;
 int min(  int a,  int b) 
{

  return ((a < b)?a:b);
}

;
 int llc_main(  int argc,  char **argv) 
{

      int   i;
      int   j;
      int   numinside;
      double   area_llc;
      double   error_llc;
      complex   z;
    complex c [4092];
      double   ztemp;
      int   numoutside;
      CLOCK_TYPE   chrono;
      double   t_llc;
rinit  (54321  ) ;
  for (i= 0; i < 4092; (i++ ))  
{

c[i].creal = (-2.0 ) + (2.5 * uni( ) );
c[i].cimag = 1.125 * uni( ) ;
  }

 ;
gettimeofday  ((&chrono )  ,NULL  ) ;
numoutside= 0;
  #pragma omp  
;
  
{

loopParallelFor0_support    (c    ,(&z )    ,(&i )    ,(&ztemp )    ,(&numoutside )    ,(&j )    ) ;
  }

;
numinside= 4092 - numoutside;
area_llc= (((2.0 * 2.5) * 1.125) * numinside) / 4092;
error_llc= area_llc / sqrt(4092) ;
  
{

          CLOCK_TYPE     ch2;
gettimeofday    ((&ch2 )    ,NULL    ) ;
t_llc= (double )(ch2.tv_sec  - chrono.tv_sec ) + ((double )(ch2.tv_usec  - chrono.tv_usec ) / 1.0e6);
  }

;
printf  ("%d:%g:%16.12f +/- %16.12f\n"  ,LLC_NAME  ,t_llc  ,area_llc  ,error_llc  ) ;
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
