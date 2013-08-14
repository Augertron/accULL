/* ***********************************************************************
  This program is part of the
	OpenMP Source Code Repository

	http://www.pcg.ull.es/ompscr/
	e-mail: ompscr@etsii.ull.es

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License 
  (LICENSE file) along with this program; if not, write to
  the Free Software Foundation, Inc., 59 Temple Place, Suite 330, 
  Boston, MA  02111-1307  USA
	
	FILE:              c_md.c
  VERSION:           1.0
  DATE:              May 2004
  AUTHOR:            Bill Magro, Kuck and Associates, Inc. (KAI), 1998
  COMMENTS TO:       sande@csi.ull.es
  DESCRIPTION:       This program implements a simple molecular dynamics simulation,
                     using the velocity Verlet time integration scheme. 
										 The particles interact with a central pair potential.
  COMMENTS:	
  REFERENCES:        W. C. Swope and H. C. Andersen and P. H. Berens and K. R.  Wilson
                     A Computer Simulation Method for the Calculation of
                     Equilibrium Constants for the Formation of Physical
                     Clusters of Molecules: Application to Small Water Clusters
                     Journal of Chemical Physics, 1982 vol. 76 pg 637-649
  BASIC PRAGMAS:     parallel for
	USAGE:             ./c_md.par 8192 10
  INPUT:             Number of particles
	                   Number of simulation steps
  OUTPUT:            - 
	FILE FORMATS:      -
	RESTRICTIONS:      -
	REVISION HISTORY:
**************************************************************************/
#include <sys/time.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

#ifndef RAND_MAX
#define RAND_MAX 0x7fff
#endif

#ifndef M_PI_2
#define M_PI_2   1.57079632679489661923  /* pi/2 */
#endif

#define NUM_ARGS				2
#define NUM_TIMERS			1
#define DEFAULT_NPARTS 	8192/4
#define DEFAULT_NSTEPS 	10
#define USAGE_STR "NPARTS NSTEPS"
#define NDIM 3

int NPARTS;       /* No. of particles */
int NSTEPS;       /* No. of simulation steps */

typedef double vnd_t[NDIM];

/* -----------------------------------------------------------------------
                          PROTOTYPES
 * ----------------------------------------------------------------------- */

double v(double x);
double dv(double x);
void initialize(int np, int nd, vnd_t box, vnd_t *pos, vnd_t *vel, vnd_t *acc);
double dist(int nd, vnd_t r1, vnd_t r2, vnd_t dr);
double dot_prod(int n, vnd_t x,vnd_t y);
void compute(int np, int nd, vnd_t *pos, vnd_t *vel, double mass, vnd_t *f, double *pot_p, double *kin_p);
void update(int np, int nd, vnd_t *pos, vnd_t *vel, vnd_t *f, vnd_t *a, double mass, double dt);
int main (int argc, char **argv);

// Time measurement
typedef  struct timespec CLOCK_TYPE;

void chrono_gettime(CLOCK_TYPE *tv) {
            clock_gettime(CLOCK_MONOTONIC, tv);
}

double diftime(CLOCK_TYPE ch, CLOCK_TYPE ch2) {
    CLOCK_TYPE temp;
    if ((ch2.tv_nsec-ch.tv_nsec)<0) {
        temp.tv_sec = ch2.tv_sec-ch.tv_sec-1;
        temp.tv_nsec = 1000000000+ch2.tv_nsec-ch.tv_nsec;
    } else {
        temp.tv_sec = ch2.tv_sec-ch.tv_sec;
        temp.tv_nsec = ch2.tv_nsec-ch.tv_nsec;
    }
    return (double) temp.tv_sec + (double) (temp.tv_nsec*1e-9f);
}
CLOCK_TYPE start_clock, end_clock;

/* -----------------------------------------------------------------------
                          IMPLEMENTATION
 * ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
   statement function for the pair potential.
   This potential is a harmonic well which smoothly saturates to a
   maximum value at PI/2.  
 * ----------------------------------------------------------------------- */
double v(double x) {
  if (x < M_PI_2) 
    return pow(sin(x), 2.0);
  else
    return 1.0;
}
/* -----------------------------------------------------------------------
   statement function for the derivative of the pair potential
 * ----------------------------------------------------------------------- */
double dv(double x) {
  if (x < M_PI_2) 
    return 2.0 * sin(x) * cos(x);
  else
    return 0.0;
}
/* -----------------------------------------------------------------------
   Initialize the positions, velocities, and accelerations.
 * ----------------------------------------------------------------------- */
void initialize(int np, int nd, 
vnd_t box, vnd_t *pos, vnd_t *vel, vnd_t *acc) {
  int i, j;
  double x;
  
  srand(4711L);
  for (i = 0; i < np; i++) {
    for (j = 0; j < nd; j++) {
      x = rand() % 10000 / (double)10000.0;
      pos[i][j] = box[j] * x;
      vel[i][j] = 0.0;
      acc[i][j] = 0.0;
    }
  }
}
/* -----------------------------------------------------------------------
   Compute the displacement vector (and its norm) between two particles. 
 * ----------------------------------------------------------------------- */
double dist(int nd, vnd_t r1, vnd_t r2, vnd_t dr) {
  int i;
  double d;
  
  d = 0.0;
  for (i = 0; i < nd; i++) {
    dr[i] = r1[i] - r2[i];
    d += dr[i] * dr[i];
  }
  return sqrt(d);
}
/* -----------------------------------------------------------------------
   Return the dot product between two vectors of type double and length n 
 * ----------------------------------------------------------------------- */
double dot_prod(int n, vnd_t x, vnd_t y) {
  int i;
  double t = 0.0;
  
  for (i = 0; i < n; i++) {
    t += x[i] * y[i];
  }
  return t;
}
/* -----------------------------------------------------------------------
   Compute the forces and energies, given positions, masses,
   and velocities
 * ----------------------------------------------------------------------- */
void compute(int np, int nd, vnd_t *pos, vnd_t *vel, 
	     double mass, vnd_t *f, double *pot_p, double *kin_p) {
  int i, j, k;
  double pot, kin;
  
  pot = 0.0;
  kin = 0.0;
  /* The computation of forces and energies is fully parallel. */
#pragma acc kernels name("compute") create(i, j, k) pcopy(f[NPARTS]) copy(pot,kin) pcopyin(vel[NPARTS],pos[NPARTS]) 
{
  i = 0;
  #pragma acc loop  private (i) 
  for (i = 0; i < np; i++) {
      f[i][0] = 0.0;
      f[i][1] = 0.0;
      f[i][2] = 0.0;
  }

  #pragma acc loop reduction(+ : pot) private (i,j)
  for (i = 0; i < np; i++) 
    for (j = 0; j < np; j++) {
      vnd_t rij;
      double  d, tmp_dv;
      int ii;
      if (i != j) {
// d = dist(nd, pos[i], pos[j], rij);
        d = 0.0;
        /* Distance */
       /* for (ii = 0; ii < nd; ii++) {
            rij[ii] = pos[i][ii] - pos[j][ii];
            d += rij[ii] * rij[ii];
        }*/
            rij[0] = pos[i][0] - pos[j][0];
            rij[1] = pos[i][1] - pos[j][1];
            rij[2] = pos[i][2] - pos[j][2];

            d += rij[0] * rij[0];
            d += rij[1] * rij[1];
            d += rij[2] * rij[2];


         d = sqrt(d);
        /* attribute half of the potential energy to particle 'j' */
        pot = pot + 0.5 * v(d);
       /* for (k = 0; k < nd; k++) {
          f[i][k] = f[i][k] - rij[k]* dv(d) /d;
        }*/
          tmp_dv = dv(d) / d;
          f[i][0] = f[i][0] - rij[0]* tmp_dv;
          f[i][1] = f[i][1] - rij[1]* tmp_dv;
          f[i][2] = f[i][2] - rij[2]* tmp_dv;
     } 
   }
  
   /* compute kinetic energy */
/*   int ii = 0;
   for (ii = 0; ii < nd; ii++) {
    kin += vel[i][ii] * vel[i][ii];
   }*/

  #pragma acc loop reduction(+ : kin) private (i)
   for(i = 0; i < np; i++) {
    kin += vel[i][0] * vel[i][0];
    kin += vel[i][1] * vel[i][1];
    kin += vel[i][2] * vel[i][2];
   }
}
  kin = kin * 0.5 * mass;
  *pot_p = pot;
  *kin_p = kin;
}
/* -----------------------------------------------------------------------
   Perform the time integration, using a velocity Verlet algorithm
 * ----------------------------------------------------------------------- */
void update(int np, int nd, vnd_t *pos, vnd_t *vel, vnd_t *f, vnd_t *a, double mass, double dt) {
  int i, j;
  double rmass;
  /* Seems to solve rare bug of Frangollo */
  int _np = np;
  int _nd = nd;
  
  rmass = 1.0/mass;
  /* The time integration is fully parallel */
  #pragma acc kernels loop create(i, j) copyin(rmass,dt)  pcopyin(f[NPARTS]) pcopy(vel[NPARTS],pos[NPARTS],a[NPARTS]) private(i, j) name("update") 
  for (i = 0; i < _np; i++) 
    for (j = 0; j < _nd; j++) {
      pos[i][j] = pos[i][j] + vel[i][j]*dt + 0.5*dt*dt*a[i][j];
      vel[i][j] = vel[i][j] + 0.5*dt*(f[i][j]*rmass + a[i][j]);
      a[i][j] = f[i][j]*rmass;
    }
}
/* ----------------------------------------------------------------------- */
int main (int argc, char **argv) {
  /* simulation parameters */
  double mass = 1.0;
  double dt = 1.0e-4;
  vnd_t box;
  vnd_t *position;
  vnd_t *velocity;
  vnd_t *force;
  vnd_t *accel;
  double potential, kinetic, E0;
  int i;
	int NUMTHREADS;
	double total_time;
	char *PARAM_NAMES[NUM_ARGS] = {"Nparts", "Nsteps"};
	char *TIMERS_NAMES[NUM_TIMERS] = {"Total_time" };
  char *DEFAULT_VALUES[NUM_ARGS] = {"8193", "10"};


  NUMTHREADS = 1; // omp_get_max_threads();
/*  OSCR_init (NUMTHREADS, "Molecular dynamic simulation", "Use md <Nparts> <Nsteps>", NUM_ARGS, 
    PARAM_NAMES, DEFAULT_VALUES , NUM_TIMERS, NUM_TIMERS, TIMERS_NAMES, 
    argc, argv);*/
   if (argc != 3){
      printf(" Incorrect number of parameters \n");
      exit(-1);
   }
 
  /* Default: DEFAULT_NPARTS, DEFAULT_NSTEPS */
    NPARTS = atoi(argv[1]);
    NSTEPS = atoi(argv[2]);

	/* Memory allocation */
	position = malloc(NPARTS * sizeof(vnd_t));
	velocity = malloc(NPARTS * sizeof(vnd_t));
	force    = malloc(NPARTS * sizeof(vnd_t));
	accel    = malloc(NPARTS* sizeof(vnd_t));

  NUMTHREADS = 1; // omp_get_max_threads();
  for (i = 0; i < NDIM; i++)
    box[i] = 10.0;
  /* set initial positions, velocities, and accelerations */
  initialize(NPARTS, NDIM, box, position, velocity, accel);
	// OSCR_timer_start(0);
  printf("POTENTIAL KINETIC ACTUAL\n", potential, kinetic, (potential + kinetic - E0) / E0);
  chrono_gettime(&start_clock);
  /* compute the forces and energies */
  compute(NPARTS, NDIM, position, velocity, mass, force, &potential, &kinetic);
  E0 = potential + kinetic;
  printf("%17.9e %17.9e %17.9e\n", potential, kinetic, (potential + kinetic - E0) / E0);
  /* This is the main time stepping loop */
//  #pragma acc data copy(force[NPARTS], velocity[NPARTS], position[NPARTS], accel[N
  for (i = 0; i < NSTEPS; i++) {
    compute(NPARTS, NDIM, position, velocity, mass, force, &potential, &kinetic);
#if 1
    printf("%17.9e %17.9e %17.9e\n", potential, kinetic, (potential + kinetic - E0) / E0);
#endif
    update(NPARTS, NDIM, position, velocity, force, accel, mass, dt);
  }
//  FRG__printStats();
	// OSCR_timer_stop(0);
  chrono_gettime(&end_clock);
  total_time = diftime(start_clock, end_clock);
	// OSCR_report(1, TIMERS_NAMES);
	printf("\n \t# THREADS \tTIME (nano secs.) \n");
	printf("\t %d \t\t%14.6lf\n", NUMTHREADS, total_time);

  return 0;
}


/*
 * vim:ts=2:sw=2:
 */
