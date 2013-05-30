/***********************************************************************
 * This program implements a simple molecular dynamics simulation,
 *   using the velocity Verlet time integration scheme. The particles
 *   interact with a central pair potential.
 *
 * Author:   Bill Magro, Kuck and Associates, Inc. (KAI), 1998
 *
 * Parallelism is implemented via OpenMP directives.
 ***********************************************************************/
/* History: 22.05.2002 F. de Sande Adapted for use with llc,
 *           adding only #pragma llc directives
 *            - NDIM, NSTEPS & NPARTS in uppercase
 *            - Use of R8F string is elliminated
 *            - Only the master processor prints the result
 *            - real8 type substituted with double
 ***********************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <mpi.h> 

#ifndef RAND_MAX
#define RAND_MAX 0x7fff
#endif


#ifndef M_PI_2
/* pi/2 */
#define M_PI_2   1.57079632679489661923
#endif


#define ndim 3
#define nparts 8192
#define nsteps 10

#define NDIM ndim
#define NPARTS nparts
#define NSTEPS nsteps

int MPI_NAME, NUMPROCESSORS; 
int sendcounts[64], sdispls[64], recvcounts[64], rdispls[64];



typedef double vnd_t[ndim] ;

double t() {
    struct timeval tv;
    gettimeofday(&tv, ((void *)0));
    return (double)tv.tv_sec + (double)tv.tv_usec/1000000.0;
}

double mpi_time;

/* statement function for the pair potential and its derivative
   This potential is a harmonic well which smoothly saturates to a
   maximum value at PI/2.  */

double v(double x) {
  if (x < M_PI_2) 
    return pow(sin(x), 2.0);
  else
    return 1.0;
}

double dv(double x) {
  if (x < M_PI_2) 
    return 2.0 * sin(x) * cos(x);
  else
    return 0.0;
}


/***********************************************************************
 * Initialize the positions, velocities, and accelerations.
 ***********************************************************************/
void initialize(int np, int nd, vnd_t box, vnd_t *pos, vnd_t *vel, vnd_t *acc)
{
  int i, j;
  double x;
  
  srand(4711L);
  for (i = 0; i < np; i++) {
    for (j = 0; j < nd; j++) {
      x = rand() % 10000/(double)10000.0;
      pos[i][j] = box[j]*x;
      vel[i][j] = 0.0;
      acc[i][j] = 0.0;
    }
  }
}

/* Compute the displacement vector (and its norm) between two particles. */
double dist(int nd, vnd_t box, vnd_t r1, vnd_t r2, vnd_t dr)
{
  int i;
  double d;
  
  d = 0.0;
  for (i = 0; i < nd; i++) {
    dr[i] = r1[i] - r2[i];
    d += dr[i] * dr[i];
  }
  return sqrt(d);
}

/* Return the dot product between two vectors of type double and length n */
double dotr8(int n, vnd_t x,vnd_t y)
{
  int i;
  double t = 0.0;
  
  for (i = 0; i < n; i++) {
    t += x[i]*y[i];
  }

  return t;
}

/***********************************************************************
 * Perform the time integration, using a velocity Verlet algorithm
 ***********************************************************************/
void update(int np, int nd, vnd_t *pos, vnd_t *vel, vnd_t *f, vnd_t *a, double mass, double dt)
{
  int i, j;
  double rmass;
	int start, end, chunk_size;
  
  rmass = 1.0/mass;
  
/* The time integration is fully parallel */
	chunk_size = np / NUMPROCESSORS;
	start = MPI_NAME * chunk_size;
	end = (MPI_NAME == (NUMPROCESSORS - 1)) ? np : (MPI_NAME + 1) * chunk_size;
	if (MPI_NAME == (NUMPROCESSORS - 1)) 
		chunk_size += (np % NUMPROCESSORS);
	for (i = 0; i < NUMPROCESSORS; i++) {
		sdispls[i] = start * nd;
		sendcounts[i] = chunk_size * nd;	
		rdispls[i] = i * (np / NUMPROCESSORS) * nd;
		recvcounts[i] = (np / NUMPROCESSORS) * nd; 
	}
	recvcounts[NUMPROCESSORS - 1] += ((np % NUMPROCESSORS) * nd); 
  for (i = start; i < end; i++) {
    for (j = 0; j < nd; j++) {
      pos[i][j] = pos[i][j] + vel[i][j]*dt + 0.5*dt*dt*a[i][j];
      vel[i][j] = vel[i][j] + 0.5*dt*(f[i][j]*rmass + a[i][j]);
      a[i][j] = f[i][j]*rmass;
    }
  }
  MPI_Alltoallv(pos, sendcounts, sdispls, MPI_DOUBLE, pos, recvcounts, rdispls, MPI_DOUBLE, MPI_COMM_WORLD); 
  MPI_Alltoallv(vel, sendcounts, sdispls, MPI_DOUBLE, vel, recvcounts, rdispls, MPI_DOUBLE, MPI_COMM_WORLD); 
  MPI_Alltoallv(a,   sendcounts, sdispls, MPI_DOUBLE, a,   recvcounts, rdispls, MPI_DOUBLE, MPI_COMM_WORLD); 
}

/***********************************************************************
 * Compute the forces and energies, given positions, masses, and velocities
 ***********************************************************************/
void compute(int np, int nd, double *box, vnd_t *pos, vnd_t *vel, double mass, vnd_t *f, double *pot_p, double *kin_p)
{
  int i, j, k;
  vnd_t rij;
  double  d;
  double pot, kin, lpot, lkin;
	int start, end, chunk_size;

  lpot = pot = 0.0;
  lkin = kin = 0.0;
  
/* The computation of forces and energies is fully parallel. */
/*
#pragma omp parallel for default(shared) private(i,j,k,rij,d) reduction(+ : pot, kin)
#pragma llc reduction_type (double, double) 
#pragma llc result(f[i], nd)
*/
	chunk_size = np / NUMPROCESSORS;
	start = MPI_NAME * chunk_size;
	end = (MPI_NAME == (NUMPROCESSORS - 1)) ? np : (MPI_NAME + 1) * chunk_size;
	if (MPI_NAME == (NUMPROCESSORS - 1)) 
		chunk_size += (np % NUMPROCESSORS);
	for (i = 0; i < NUMPROCESSORS; i++) {
		sdispls[i] = start * nd;
		sendcounts[i] = chunk_size * nd;
		
		rdispls[i] = i * (np / NUMPROCESSORS) * nd;
		recvcounts[i] = (np / NUMPROCESSORS) * nd; 
	}
	recvcounts[NUMPROCESSORS - 1] += ((np % NUMPROCESSORS) * nd); 
  for (i = start; i < end; i++) {
    /* compute potential energy and forces */
    for (j = 0; j < nd; j++)
      f[i][j] = 0.0;

    for (j = 0; j < np; j++) {
      if (i != j) {
        d = dist(nd,box,pos[i],pos[j],rij);
        /* attribute half of the potential energy to particle 'j' */
        lpot = lpot + 0.5 * v(d);
        for (k = 0; k < nd; k++) {
          f[i][k] = f[i][k] - rij[k] * dv(d) / d;
        }
      }
    }
    /* compute kinetic energy */
    lkin = lkin + dotr8(nd,vel[i],vel[j]);
  }
  MPI_Alltoallv(f, sendcounts, sdispls, MPI_DOUBLE, f, recvcounts, rdispls, MPI_DOUBLE, MPI_COMM_WORLD); 
  MPI_Allreduce(&lpot, &pot, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD); 
  MPI_Allreduce(&lkin, &kin, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD); 

  kin = kin * 0.5 * mass;
  *pot_p = pot;
  *kin_p = kin;
}

/******************
 * main program
 ******************/

int main (int argc, char **argv) {
  /* simulation parameters */
  double mass = 1.0;
  double dt = 1.0e-4;
  vnd_t box;
  vnd_t position[nparts];
  vnd_t velocity[nparts];
  vnd_t force[nparts];
  vnd_t accel[nparts];
  double potential, kinetic, E0;
  int i;
  double t0,t1;

	/*
	if (argc == 3) {
		NPARTS = atoi(argv[1]);
		NSTEPS = atoi(argv[2]);
	}
	else {
		NPARTS = 2048;
		NSTEPS = 10;
	}
	printf("NPARTS: %d NSTEPS: %d\n", NPARTS, NSTEPS);
	position = (vnd_t *)malloc(NPARTS * sizeof(vnd_t));
	velocity = (vnd_t *)malloc(NPARTS * sizeof(vnd_t));
	accel    = (vnd_t *)malloc(NPARTS * sizeof(vnd_t));
	force    = (vnd_t *)malloc(NPARTS * sizeof(vnd_t));
	if ((position == NULL) || (velocity == NULL) || (accel == NULL) || (force == NULL)) {
	  printf("Not enough memory for the vectors. NPARTS: %d\n", NPARTS);
    exit(1);
	}
  */
  MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &NUMPROCESSORS);
	MPI_Comm_rank(MPI_COMM_WORLD, &MPI_NAME);

  for (i = 0; i < ndim; i++)
    box[i] = 10.0;

  /* set initial positions, velocities, and accelerations */
  initialize(nparts,ndim,box,position,velocity,accel);

  t0 = t();

  /* compute the forces and energies */
  compute(nparts,ndim,box,position,velocity,mass,force,&potential,&kinetic);
  E0 = potential + kinetic;

  /* This is the main time stepping loop */
  for (i = 0; i < nsteps; i++) {
    compute(nparts,ndim,box,position,velocity,mass,force,&potential,&kinetic);
    /* printf("%15.7e\t%15.7e\t%15.7e\n", potential, kinetic, (potential + kinetic - E0)/E0); */
    update(nparts,ndim,position,velocity,force,accel,mass,dt);
  }

  t1 = t();
  mpi_time = t1-t0;

  printf("%d:%f:[parts = %d dim = %d steps = %d]\n", MPI_NAME, mpi_time, nparts, ndim, nsteps);
	MPI_Finalize();  
  return (0);
}


/*
 * vim:ts=2:sw=2:
 */
