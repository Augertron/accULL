// Autor: Rayco Abad Martin <rayco.abad@gmail.com>

// Compilar con: mpicc -o mxm_mpi mxm_mpi.c
// Ejecutar con: mpiexec -n 4 ./mxm_mpi

// Descripcion: Cada procesador tiene una matriz donde almacena B 
// El procesador 0 enviara a cada procesador una fila de A para obtener
// la fila de C que debe calcular, estos procesadores cuando finalicen el 
// calculo se la enviaran al 0 y solicitaran una nueva fila de A.

#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define TAG_LOAD 0
#define TAG_FINISH 20000

/* -----------------------------------------------------*/

double r8_uniform_01(int *seed) {
  int k;
  double r;
  k = *seed / 127773;
  *seed = 16807 * (*seed - k * 127773) - k * 2836;
  if (*seed < 0)
    *seed = *seed + 2147483647;
  r = (double) (*seed) * 4.656612875E-10;
  return r;
}

/* -----------------------------------------------------*/

void r8_mxm_seq(double **a, double **b, double **c, int l, int n, int m) {
    int i, j, k;
    double R;

    for (j = 0; j < n; j++) {
        for (i = 0; i < l; i++) {
        R = 0.0;
            for (k = 0; k < m; k++)
                R += b[i][k] * a[k][j];
            c[i][j] = R;
        }
    }
    return;
}

/* -----------------------------------------------------*/
/* This is the master                                   */
/* -----------------------------------------------------*/

void master(double **A, double **C, int rows, int columns) {
    int size;
    int i;
    int dummy, load;
	double *row;

    MPI_Status status;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

	// Se envia una fila de A a cada procesador
    for (i = 0; (i < (size - 1)) && (i < rows); i++) {
    	MPI_Send(&A[i][0], columns, MPI_DOUBLE, i + 1, i, MPI_COMM_WORLD);
	}

	// Se envian las siguientes filas de A bajo demanda
    for (i = (size - 1); i < rows; i++) {
		// Se recibe una fila de C junto a la demanda de una nueva fila de A
		row = (double *) malloc(columns * sizeof(double));
        MPI_Recv(row, columns, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
		C[status.MPI_TAG] = row;
		// Se envia una nueva fila de A
        MPI_Send(&A[i][0], columns, MPI_DOUBLE, status.MPI_SOURCE, i, MPI_COMM_WORLD );
	}

	// Caso menos tareas que procesadores esclavos
	if (rows < (size - 1))
		size = rows + 1;

    for (i = 1; i < size; i++) {
		// Se reciben las ultimas filas de C calculadas
		row = (double *) malloc(columns * sizeof(double));
        MPI_Recv(row, columns, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
		C[status.MPI_TAG] = row;
		// Se envia confirmacion de finalizacion
        MPI_Send(&dummy, 1, MPI_INT, status.MPI_SOURCE, TAG_FINISH, MPI_COMM_WORLD );
        MPI_Recv(&load, 1, MPI_INT, status.MPI_SOURCE, TAG_LOAD, MPI_COMM_WORLD, &status );
    }
}

/* -----------------------------------------------------*/
/* This is the slave                                    */
/* -----------------------------------------------------*/

int slave(double **B, int rows, int columns) {
	int myid;
	int more_tasks = 1;
	int x, load;
	int filas = 0;
	int i, j;
	double *row, *result;
    double R;
	
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &myid );

	while (more_tasks) {

		row = (double *) malloc(columns * sizeof(double));
		MPI_Recv(row, columns, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		x = status.MPI_TAG;
		
		if (status.MPI_TAG != TAG_FINISH) {
		
			// Se realizan los calculos para la fila de C
			result = (double *) malloc(columns * sizeof(double));
			for (i = 0; i < columns; i++) {
				R = 0.0;
				for (j = 0; j < rows; j++)
					R += (row[j] * B[j][i]);
				result[i] = R;
			}

			// Se envia el resultado del calculo y se solicita una nueva fila
			MPI_Send(&result[0], columns, MPI_DOUBLE, 0, x, MPI_COMM_WORLD );

			filas++;

		} else if (status.MPI_TAG == TAG_FINISH) {
			more_tasks = 0;
			// Se envia confirmacion de finalizacion
            MPI_Send(&load, 1, MPI_INT, 0, TAG_LOAD, MPI_COMM_WORLD );
		}
	}
	return filas;
}

/* -----------------------------------------------------*/
/* Main				                                    */
/* -----------------------------------------------------*/

int main(int argc, char *argv[]) {
	int myid, numprocs;
	double startwtime = 0.0, endwtime = 0.0;
	int namelen;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int i, j, filas;
	int columns = 500, rows = 500;
    int seed;
	double *vectA, *vectB, *vectC;
	double **A, **B, **C;
    
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Get_processor_name(processor_name, &namelen);
	MPI_Status status;

	if (myid <= rows) {
		// Se reserva memoria contigua para B
   		vectB = (double *) malloc(rows * columns * sizeof(double));
		B = (double **) malloc (rows * sizeof(double *));
		for (i = 0; i < rows; i++)
			B[i] = vectB + (i * columns);

		if (myid == 0) {
			// Reserva de memoria contigua para A y C, y se inicializa A y B
   			vectA = (double *) malloc(rows * columns * sizeof(double));
			A = (double **) malloc (rows * sizeof(double *));
			for (i = 0; i < rows; i++)
				A[i] = vectA + (i * columns);
   	
			vectC = (double *) malloc(rows * columns * sizeof(double));
			C = (double **) malloc (rows * sizeof(double *));
			for (i = 0; i < rows; i++)
				C[i] = vectC + (i * columns);
		
            seed = 123456789;
			for (i = 0; i < rows; i++)
				for (j = 0; j < columns; j++)
					A[i][j] = r8_uniform_01 (&seed);
				
			for (i = 0; i < rows; i++)
				for (j = 0; j < columns; j++)
					B[i][j] = r8_uniform_01 (&seed);
					
			// Se envia la matriz B al resto de procesadores
			for (i = 1; i < numprocs; i++)
				MPI_Send(&B[0][0], (rows  * columns ), MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
		
			startwtime = MPI_Wtime();
            if (numprocs > 1)			
			    master(A, C, rows, columns);
			else
                r8_mxm_seq(A, B, C, rows, rows, columns);
			endwtime = MPI_Wtime();
           
            /*
            printf("\nA");
            for (i = 0; i < rows; i++) {
                printf("\n");
                for (j = 0; j < columns; j++)
                    printf("%f ", A[i][j]);
            }
            printf("\n");
    
            printf("\nB");
            for (i = 0; i < rows; i++) {
                printf("\n");
               for (j = 0; j < columns; j++)
                    printf("%f ", B[i][j]);
            }
            printf("\n");

            printf("\nC = B * A");
            for (i = 0; i < rows; i++) {
                printf("\n");
                for (j = 0; j < columns; j++)
                    printf("%f ", C[j][i]);
            }
            printf("\n");
            */ 
		
		} else {
			// Se recibe B desde el 0
			MPI_Recv(&B[0][0], (rows * columns), MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
			filas = slave(B, rows, columns);
		}
	}

	if (myid == 0)
		printf("Tiempo de ejecucion: %f\n", endwtime - startwtime);
	
	MPI_Finalize();
	return 0;
}

