#include "llcomp_llc.h"
  int NAME;
  int NUMPROCESSORS;
  int sendcounts [64];
  int sdispls [64];
  int recvcounts [64];
  int rdispls [64];
typedef  double real8;
typedef  double vnd_t [3];
 double t()
{

      struct timeval    tv;
gettimeofday  ((&tv )  ,(void *)(0)  ) ;
  return (double )(tv.tv_sec ) + ((double )(tv.tv_usec ) / 1000000.0);
}

;
  double llc_time;
 real8 v(  real8 x) 
{

  if   (x < 1.57079632679489661923  )   return pow(sin(x) ,2.0)   ;  else return 1.0;
}

;
 real8 dv(  real8 x) 
{

  if   (x < 1.57079632679489661923  )   return (2.0 * sin(x) ) * cos(x)   ;  else return 0.0;
}

;
 void initialize(  int np,  int nd,  vnd_t box,  vnd_t *pos,  vnd_t *vel,  vnd_t *acc) 
{

      int   i;
      int   j;
      real8   x;
srand  (4711L  ) ;
  for (i= 0; i < np; (i++ ))  
{

    for (j= 0; j < nd; (j++ ))    
{

x= (rand( )  % 10000) / (real8 )(10000.0);
pos[i][j]= box[j] * x;
vel[i][j]= 0.0;
acc[i][j]= 0.0;
    }

 ;
  }

 ;
}

;
 real8 dist(  int nd,  vnd_t box,  vnd_t r1,  vnd_t r2,  vnd_t dr) 
{

      int   i;
      real8   d;
d= 0.0;
  for (i= 0; i < nd; (i++ ))  
{

dr[i]= r1[i] - r2[i];
d+= dr[i] * dr[i];
  }

 ;
  return sqrt(d) ;
}

;
 real8 dotr8(  int n,  vnd_t x,  vnd_t y) 
{

      int   i;
      real8   t = 0.0;
  for (i= 0; i < n; (i++ ))  
{

t+= x[i] * y[i];
  }

 ;
  return t;
}

;
 void update(  int np,  int nd,  vnd_t *pos,  vnd_t *vel,  vnd_t *f,  vnd_t *a,  real8 mass,  real8 dt) 
{

      int   i;
      int   j;
      real8   rmass;
rmass= 1.0 / mass;
  #pragma omp  
;
  
{

loopParallelFor0_support    (f    ,(&np )    ,pos    ,vel    ,a    ,(&nd )    ,(&i )    ,(&dt )    ,(&rmass )    ,(&j )    ) ;
  }

;
}

;
 void compute(  int np,  int nd,  real8 *box,  vnd_t *pos,  vnd_t *vel,  real8 mass,  vnd_t *f,  real8 *pot_p,  real8 *kin_p) 
{

      int   i;
      int   j;
      int   k;
      vnd_t   rij;
      real8   d;
      real8   pot;
      real8   kin;
pot= 0.0;
kin= 0.0;
  #pragma omp  
;
  
{

loopParallelFor1_support    ((&np )    ,(&pot )    ,(&k )    ,rij    ,box    ,f    ,(&kin )    ,(&d )    ,(&i )    ,(&nd )    ,pos    ,vel    ,(&j )    ) ;
  }

;
kin= (kin * 0.5) * mass;
*(pot_p) = pot;
*(kin_p) = kin;
}

;
 int llc_main(  int argc,  char **argv) 
{

      real8   mass = 1.0;
      real8   dt = 1.0e-4;
      vnd_t   box;
    vnd_t position [8192];
    vnd_t velocity [8192];
    vnd_t force [8192];
    vnd_t accel [8192];
      real8   potential;
      real8   kinetic;
      real8   E0;
      int   i;
      double   t0;
      double   t1;
  for (i= 0; i < 3; (i++ )) box[i]= 10.0 ;
initialize  (8192  ,3  ,box  ,position  ,velocity  ,accel  ) ;
t0= t( ) ;
compute  (8192  ,3  ,box  ,position  ,velocity  ,mass  ,force  ,(&potential )  ,(&kinetic )  ) ;
E0= potential + kinetic;
  for (i= 0; i < 10; (i++ ))  
{

compute    (8192    ,3    ,box    ,position    ,velocity    ,mass    ,force    ,(&potential )    ,(&kinetic )    ) ;
update    (8192    ,3    ,position    ,velocity    ,force    ,accel    ,mass    ,dt    ) ;
  }

 ;
t1= t( ) ;
llc_time= t1 - t0;
printf  ("%d:%f:[parts = %d dim = %d steps = %d]\n"  ,LLC_NAME  ,llc_time  ,8192  ,3  ,10  ) ;
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
