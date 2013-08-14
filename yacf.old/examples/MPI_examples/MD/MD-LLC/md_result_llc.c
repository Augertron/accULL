/***********************************************************************
 * This program implements a simple molecular dynamics simulation,
 *   using the velocity Verlet time integration scheme. The particles
 *   interact with a central pair potential.
 *
 * Author:   Bill Magro, Kuck and Associates, Inc. (KAI), 1998
 *
 * Parallelism is implemented via OpenMP directives.
 ***********************************************************************/
#define SEQ_TIME 0.0

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

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

int NAME, NUMPROCESSORS; 
int sendcounts[64], sdispls[64], recvcounts[64], rdispls[64];


typedef double real8;

typedef double vnd_t[ndim] ;

double t() {
    struct timeval tv;
    gettimeofday(&tv, ((void *)0));
    return (double)tv.tv_sec + (double)tv.tv_usec/1000000.0;
}

double llc_time;

/* statement function for the pair potential and its derivative
   This potential is a harmonic well which smoothly saturates to a
   maximum value at PI/2.  */

real8 v(real8 x) {
  if (x < M_PI_2) 
    return pow(sin(x), 2.0);
  else
    return 1.0;
}

real8 dv(real8 x) {
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
  real8 x;
  
  srand(4711L);
  for (i = 0; i < np; i++) {
    for (j = 0; j < nd; j++) {
      x = rand() % 10000/(real8)10000.0;
      pos[i][j] = box[j]*x;
      vel[i][j] = 0.0;
      acc[i][j] = 0.0;
    }
  }
}

/* Compute the displacement vector (and its norm) between two particles. */
real8 dist(int nd, vnd_t box, vnd_t r1, vnd_t r2, vnd_t dr)
{
  int i;
  real8 d;
  
  d = 0.0;
  for (i = 0; i < nd; i++) {
    dr[i] = r1[i] - r2[i];
    d += dr[i] * dr[i];
  }
  return sqrt(d);
}

/* Return the dot product between two vectors of type real*8 and length n */
real8 dotr8(int n, vnd_t x,vnd_t y)
{
  int i;
  real8 t = 0.0;
  
  for (i = 0; i < n; i++) {
    t += x[i]*y[i];
  }

  return t;
}

/***********************************************************************
 * Perform the time integration, using a velocity Verlet algorithm
 ***********************************************************************/
void update(int np, int nd, vnd_t *pos, vnd_t *vel, vnd_t *f, vnd_t *a, real8 mass, real8 dt)
{
  int i, j;
  real8 rmass;
  
  rmass = 1.0/mass;
  
  /* The time integration is fully parallel */
#pragma omp target device(mpi)
#pragma omp parallel for default(shared) private(i,j) firstprivate(rmass, dt)
#pragma llc result (&pos[i], 1, &vel[i], 1, &a[i], 1)
  for (i = 0; i < np; i++) {
    for (j = 0; j < nd; j++) {
      pos[i][j] = pos[i][j] + vel[i][j]*dt + 0.5*dt*dt*a[i][j];
      vel[i][j] = vel[i][j] + 0.5*dt*(f[i][j]*rmass + a[i][j]);
      a[i][j] = f[i][j]*rmass;
    }
  }
}

/***********************************************************************
 * Compute the forces and energies, given positions, masses, and velocities
 ***********************************************************************/
void compute(int np, int nd, real8 *box, vnd_t *pos, vnd_t *vel, real8 mass, vnd_t *f, real8 *pot_p, real8 *kin_p) 
{
  int i, j, k;
  vnd_t rij;
  real8  d;
  real8 pot, kin;
  
  pot = 0.0;
  kin = 0.0;
  
  /* The computation of forces and energies is fully parallel. */
#pragma omp target device(mpi)
#pragma omp parallel for default(shared) private(i,j,k,rij,d) reduction(+ : pot, kin)
#pragma llc result (&f[i], 1)
  for (i = 0; i < np; i++) {
    /* compute potential energy and forces */
    for (j = 0; j < nd; j++)
      f[i][j] = 0.0;

    for (j = 0; j < np; j++) {
      if (i != j) {
        d = dist(nd,box,pos[i],pos[j],rij);
        /* attribute half of the potential energy to particle 'j' */
        pot = pot + 0.5 * v(d);
        for (k = 0; k < nd; k++) {
          f[i][k] = f[i][k] - rij[k]* dv(d) /d;
        }
      }
    }
    /* compute kinetic energy */
    kin = kin + dotr8(nd,vel[i],vel[j]);
  }
  
  kin = kin*0.5*mass;

  *pot_p = pot;
  *kin_p = kin;
}

/******************
 * main program
 ******************/

int main (int argc, char **argv) {
  /* simulation parameters */
  real8 mass = 1.0;
  real8 dt = 1.0e-4;
  vnd_t box;
  vnd_t position[nparts];
  vnd_t velocity[nparts];
  vnd_t force[nparts];
  vnd_t accel[nparts];
  real8 potential, kinetic, E0;
  int i;
  double t0,t1;

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
    /* LLC_printMaster("%15.7e\t%15.7e\t%15.7e\n", potential, kinetic, (potential + kinetic - E0)/E0); */
    update(nparts,ndim,position,velocity,force,accel,mass,dt);
  }

  t1 = t();
  llc_time = t1-t0;

  printf("%d:%f:[parts = %d dim = %d steps = %d]\n", LLC_NAME, llc_time, nparts, ndim, nsteps);
  
  return (0);
}

