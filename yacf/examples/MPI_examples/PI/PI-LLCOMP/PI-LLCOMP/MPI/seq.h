#ifdef SEQUENTIAL
#include <mpi.h>
 
int LLC_NUMPROCESSORS = 1;
int LLC_GLOBAL_NUMPROCESSORS = 1;
int LLC_NAME = 0;
int LLC_GLOBAL_NAME = 0;


int main (int argc, char **argv) {
	 
	MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, (int*)&LLC_GLOBAL_NAME);
  MPI_Comm_size(MPI_COMM_WORLD, (int*)&LLC_GLOBAL_NUMPROCESSORS);
							 
  parseq_main (argc, argv);
  MPI_Finalize();
  return 0;
}
 

#define LLC_printMaster printf

#define LLC_printVerif 


/* For sequential execution */
int omp_get_num_threads (void) {
	
	return 1;
}

int omp_get_thread_num (void) {
	
	return 0;
}

int omp_get_num_procs(void) {
	
	return 1;
}

int omp_get_global_thread_num (void) {
	
	return 0;
}

int omp_get_global_num_procs (void) {
	
	return 1;
}

#else
int main(int argc, char **argv) {
	parseq_main(argc,argv);
  return 0;
}
#endif


