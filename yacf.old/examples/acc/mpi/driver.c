#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <mpi.h>
#include <omp.h>
#include <signal.h>
#include <unistd.h>

void mm(  int t,  double *a,  double *b,  double *c,  double *d,  double *r,  int nodo,  int np) ;

void initialize(double *m, int t){
  int i;

  for (i = 0; i < t*t; i++)
  {
    m[i] = (double)((int)(((1.*rand()/RAND_MAX)*2000.-1000.)))/100.;
  }
}

void escribir(double *m, int t){
  int i, j;

  for (i = 0; i < t; i++)
  {
    for (j = 0; j < t; j++)
    {  
      printf("%.8lf ",m[i*t+j]);
    }
    printf("\n");
  }
}

void escribirresult(double *m, int t,int cantidad){
  int i, j,cont=0;

  for (i = 0; i < t; i++)
  {
    for (j = 0; j < t; j++)
    {  
      if((int) (m[i*t+j]*1000.)!=0)
      {
        if((cont%cantidad)==0)
        {
          printf("%.0lf ",m[i*t+j]);
        }
        cont++;
      }
    }
    printf("\n");
  }
  printf("\n");
}

/*
c
c     mseconds - returns elapsed milliseconds since Jan 1st, 1970.
c
*/
long long mseconds(){
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec*1000 + t.tv_usec/1000;
}

static void alarm_handler(int sig) {
  fprintf(stderr, "Time Limit Exceeded\n");
  abort();
}

int main(int argc,char *argv[]) {
  int nodo,np,i, j,N,cuantos,semilla,cantidadescribir;
  int long_name;
  long long ti,tf,tt=0;
  double *a,*b,*c,*d,*r;
  MPI_Status estado;

 // FILE *stats_file = fopen("stats", "w");

  struct sigaction sact;
  sigemptyset(&sact.sa_mask);
  sact.sa_flags = 0;
  sact.sa_handler = alarm_handler;
  sigaction(SIGALRM, &sact, NULL);
  // alarm(500);  /* time limit */

  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&np);
  MPI_Comm_rank(MPI_COMM_WORLD,&nodo);

  if(nodo==0)
  {

    scanf("%d",&cuantos);
    MPI_Bcast(&cuantos,1,MPI_INT,0,MPI_COMM_WORLD);
    fprintf(stdout, "N Time (ms)\n");
  }
  else
  {
    MPI_Bcast(&cuantos,1,MPI_INT,0,MPI_COMM_WORLD);
  }
  for(i=0;i<cuantos;i++)
  {
    if(nodo==0)
    {
      scanf("%d",&N);
      scanf("%d",&semilla);
      scanf("%d",&cantidadescribir);
      a = (double *) malloc(sizeof(double)*N*N);
      b = (double *) malloc(sizeof(double)*N*N);
/*      c = (double *) malloc(sizeof(double)*N*N);
      d = (double *) malloc(sizeof(double)*N*N);*/
      r = (double *) malloc(sizeof(double)*N*N);
      srand(semilla);
      initialize(a,N);
      initialize(b,N);
/*      initialize(c,N);
      initialize(d,N);*/
    }
    MPI_Barrier(MPI_COMM_WORLD);
    ti=mseconds(); 

    mm(N,a,b,c,d,r,nodo,np);
    MPI_Barrier(MPI_COMM_WORLD);
    tf=mseconds(); 
    if(nodo==0)
    {
      tt+=tf-ti;
      fprintf(stdout, "%d %Ld\n", N, (tf-ti));
      // escribirresult(r,N,cantidadescribir);
      free(a);
      free(b);
   /*   free(c);
      free(d);*/
      free(r);
    } 
  }
  
  if(nodo==0)
  { 
    fprintf(stdout, "Total Time %Ld\n", tt);
  }
  MPI_Finalize();
  return 0;
}
