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
  LLC_NAME = LLC_GLOBAL_NAME;
  LLC_NUMPROCESSORS = LLC_GLOBAL_NUMPROCESSORS;
 					 
  parseq_main (argc, argv);
  MPI_Finalize();
  return 0;
}
 

#define LLC_printMaster printf

#define LLC_printVerif(res)	printf("@NP = %d, NAME = %d. VERIFICATION = %s\n", \
																			LLC_NUMPROCESSORS, LLC_NAME, res)


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

int main (int argc, char **argv) {
	 
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, (int*)&LLC_GLOBAL_NAME);
  MPI_Comm_size(MPI_COMM_WORLD, (int*)&LLC_GLOBAL_NUMPROCESSORS);
  LLC_NAME = LLC_GLOBAL_NAME;
  LLC_NUMPROCESSORS = LLC_GLOBAL_NUMPROCESSORS;
  llc_GlobalGroup = MPI_COMM_WORLD;
  llc_CurrentGroup = &llc_GlobalGroup;
								 
  parseq_main (argc, argv);
									 
  LLC_EXIT(0);
  return 0;
}
#endif


