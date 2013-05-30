#include <mpi.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

// Reserva dinamicamente espacio para un vector de tamanyo m
void ReservaVector (double **v, int m) {

  *v = (double *) malloc(m * sizeof(double) );

}

// Libera el espacio reservado dinamicamente para un vector
void LiberaVector (double *v) {
  free(v);
}

/*
// Reserva dinamicamente espacio para una matrix a de mxn
void ReservaMatriz (double ***a, int m, int n) {
  int i;

  *a = (double **) malloc(m * sizeof(double *) );
  for (i = 0; i < m; i++) 
    (*a)[i] = (double *) malloc(n * sizeof(double) );

}

// Libera el espacio reservado dinamicamente para una matrix a de m filas
void LiberaMatriz( double **a, int m) {
  int i;

  for (i = 0; i < m; i++)
    free(a[i]);
  free(a);


}
*/


/* Otra forma de reservar y liberar matrices */
/* En este caso el espacio ocupado por los elementos es un
bloque de memoria consecutiva */

void ReservaMatriz (double ***a, int m, int n) {
  int i;

//  *espacio = (double *) malloc(m * n * sizeof(double) );
  *a = (double **) malloc(m * sizeof(double *) );
  (*a)[0] = (double *) malloc(m * n * sizeof(double) );
  for (i = 1; i < m; i++) 
    (*a)[i] = (*a)[i-1] + n;
}

/*
void ReservaMatriz (double ***a, int m, int n, double **espacio) {
  int i;

  *espacio = (double *) malloc(m * n * sizeof(double) );
  *a = (double **) malloc(m * sizeof(double *) );
  for (i = 0; i < m; i++) 
    (*a)[i] = &(*espacio)[i*n];
}
*/

// Libera el espacio reservado dinamicamente para una matrix a de m filas
void LiberaMatriz( double **a) {
  free(a[0]);
  free(a);
}

/*
void LiberaMatriz( double **a, double *espacio) {
  free(espacio);
  free(a);
}
*/

// Genera aleatoriamente un vector v de tamanyo m
void GeneraVector (double *v, int m) {
  int i;
  unsigned short semilla[3];
  //time_t t;

  semilla[0] = 1;
  semilla[1] = 2; //(unsigned) time(&t);
  semilla[2] = 3; 

  for (i = 0; i < m; i++) 
    v[i] = 10.0 * erand48(semilla);

}

// Genera aleatoriamente una matriz a de tamanyo mxn
void GeneraMatriz (double **a, int m, int n) {
  int i, j;
  double *ptr;
  unsigned short semilla[3];
  //time_t t;

  semilla[0] = 1;
  semilla[1] = 2;
  semilla[2] = 3; //(unsigned) time(&t);

  for (i = 0; i < m; i++) {
    ptr = &a[i][0];
    for (j = 0; j < n; j++, ptr++)
      *ptr = 10.0 * erand48(semilla);
  }

}

// Copia el vector x en y
void CopiaVector (double *x, int n, double *y) {
  int i;
 
  for (i = 0; i < n; i++) 
    y[i] = x[i]; 

}

// Copia la matriz a en b
void CopiaMatriz (double **a, int m, int n, double **b) {
  int i, j;
  double *ptra, *ptrb;
 
  for (i = 0; i < m; i++) {
    ptra = &a[i][0];
    ptrb = &b[i][0];
    for (j = 0; j < n; j++, ptra++, ptrb++)
      *ptrb = *ptra; 
  }      

}

// Escribe por pantalla una matriz a de tamanyo mxn
void EscribeMatriz (double **a, int m, int n) {
  int i, j;
  double *ptr;
 
  for (i = 0; i < m; i++) {
    ptr = &a[i][0];
    for (j = 0; j < n; j++, ptr++)
      printf(" %9.4g ", *ptr);
    printf("\n");
  }      

}

// Escribe por pantalla un vector v de tamanyo m
void EscribeVector (double *v, int m){
  int i;

  for (i = 0; i < m; i++)
    printf(" %9.4g ", v[i]);
  printf("\n");

}


void  EscribeVectorDistr(int mi_id, int numproc, double *v, int n, char *tex) {
  int i, testigo;
  MPI_Status status;
  // Escritura del vector distribuido en orden de procesador
  if (mi_id == 0) {
    printf("[%d]: %s\n", mi_id, tex);
    for (i = 0; i < n; i++)
      printf("%10.5g ", v[i]);
    printf("\n");
    if (numproc > 1)
      MPI_Send(&testigo, 1, MPI_INT, mi_id+1, 0, MPI_COMM_WORLD);
  }
  else if (mi_id == numproc - 1) {
    MPI_Recv(&testigo, 1, MPI_INT, mi_id-1, 0, MPI_COMM_WORLD, &status);
    printf("[%d]: %s\n", mi_id, tex);
    for (i = 0; i < n; i++) 
      printf("%10.5g ", v[i]);
    printf("\n");
  }
  else {
    MPI_Recv(&testigo, 1, MPI_INT, mi_id-1, 0, MPI_COMM_WORLD, &status);
    printf("[%d]: %s\n", mi_id, tex);
    for (i = 0; i < n; i++)
      printf("%10.5g ", v[i]);
    printf("\n");
    MPI_Send(&testigo, 1, MPI_INT, mi_id+1, 0, MPI_COMM_WORLD);
  }
}

void  EscribeMatrizDistr(int mi_id, int numproc, double **a, int nrow, int ncol,
                         char *tex) {
  int i, j, testigo;
  MPI_Status status;
  // Escritura de la matriz distribuida en orden de procesador
  if (mi_id == 0) {
    printf("[%d]: %s\n", mi_id, tex);
    for (i = 0; i < nrow; i++) {
      for (j = 0; j < ncol; j++)
        printf("%10.5g ", a[i][j]);
      printf("\n");
    }
    if (numproc > 1)
      MPI_Send(&testigo, 1, MPI_INT, mi_id+1, 0, MPI_COMM_WORLD);
  }
  else if (mi_id == numproc - 1) {
    MPI_Recv(&testigo, 1, MPI_INT, mi_id-1, 0, MPI_COMM_WORLD, &status);
    printf("[%d]: %s\n", mi_id, tex);
    for (i = 0; i < nrow; i++) {
      for (j = 0; j < ncol; j++)
        printf("%10.5g ", a[i][j]);
      printf("\n");
    }
  }
  else {
    MPI_Recv(&testigo, 1, MPI_INT, mi_id-1, 0, MPI_COMM_WORLD, &status);
    printf("[%d]: %s\n", mi_id, tex);
    for (i = 0; i < nrow; i++) {
      for (j = 0; j < ncol; j++)
        printf("%10.5g ", a[i][j]);
      printf("\n");
    }
    MPI_Send(&testigo, 1, MPI_INT, mi_id+1, 0, MPI_COMM_WORLD);
  }
}

