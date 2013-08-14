/* ***********************************************************************
LIBRARY MYTIME

File: mytime.h
Version: 0.2
Date:    01.11.1999
Description: This library contains macros for handling time. It is based
  in the system routine gettimeofday() and associated types defined in
  <sys/time.h>.
History:
  21.01.1999 First version of GETTIME and DIFTIME
  01.11.1999 Modifications introduced during the TRACS visit to EPCC.
             CLOCK_Start, CLOCK_End
  19.11.1999 Include mpi.h to perform collective-reduction operations.
             It is necessary to introduce mytime.c to create functions.
************************************************************************ */
#ifndef MYTIME_H
#define MYTIME_H

#include <sys/time.h>

#define MICROSEC_PER_SEC 1.0e6

typedef struct timeval CLOCK_TYPE;

/* ---------------------------------------------------------------------
Macro: GETTIME
Description: Gets system clock time in a data struct of type timeval.
------------------------------------------------------------------------ */
#define GETTIME(tv) gettimeofday(&(tv), NULL)

/* ---------------------------------------------------------------------
Macro: DIFTIME
Description: Compute the time in microsec between the time passing in
  the data structs tv2 and tv1. The result is giving in t (double).
------------------------------------------------------------------------ */
#define DIFTIME(tv2, tv1, t) t=(double)((tv2).tv_sec-(tv1).tv_sec)+    \
               (double)((tv2).tv_usec-(tv1).tv_usec) / MICROSEC_PER_SEC 

/* ---------------------------------------------------------------------
Macro: SEC2USEC
Description: Traslate ts seconds into tus microseconds.
------------------------------------------------------------------------ */
#define SEC2USEC(ts, tus) tus=(double)(ts)*MICROSEC_PER_SEC

/* ---------------------------------------------------------------------
Macro: CLOCK_Start
Description: Starts a chrono
------------------------------------------------------------------------ */
#define CLOCK_Start(ch) gettimeofday(&(ch), NULL)

/* ---------------------------------------------------------------------
Macro: CLOCK_End
Description: Ends a chrono
------------------------------------------------------------------------ */
#define CLOCK_End(ch, tm) {                                            \
  CLOCK_TYPE ch2;                                                      \
                                                                       \
  gettimeofday(&(ch2), NULL);                                          \
  tm=(double)((ch2).tv_sec - (ch).tv_sec) +                            \
     (double)((ch2).tv_usec - (ch).tv_usec) / MICROSEC_PER_SEC;        \
}

/* ---------------------------------------------------------------------
Function: CLOCK_Avg
Description: Computes the average value of t among processors
------------------------------------------------------------------------ */
double CLOCK_Avg(double t);

/* ---------------------------------------------------------------------
Function: CLOCK_Min
Description: Computes the minimum value of t among processors
------------------------------------------------------------------------ */
double CLOCK_Min(double t);

/* ---------------------------------------------------------------------
Function: CLOCK_Max
Description: Computes the maximum value of t among processors
------------------------------------------------------------------------ */
double CLOCK_Max(double t);

#endif  /* MYTIME_H */
