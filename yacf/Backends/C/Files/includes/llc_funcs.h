extern void llc_set_num_threads (int);

extern int llc_get_num_threads (void) ;
extern int llc_get_max_threads (void) ;
extern int llc_get_thread_num (void) ;
extern int llc_get_num_procs (void) ;

extern int llc_in_parallel (void) ;

extern void llc_set_dynamic (int) ;
extern int llc_get_dynamic (void) ;

extern void llc_set_nested (int) ;
extern int llc_get_nested (void) ;

/* Locks are not implemented in llc
extern void llc_init_lock (omp_lock_t *) ;
extern void llc_destroy_lock (omp_lock_t *) ;
extern void llc_set_lock (omp_lock_t *) ;
extern void llc_unset_lock (omp_lock_t *) ;
extern int llc_test_lock (omp_lock_t *) ;
extern void omp_init_nest_lock (omp_nest_lock_t *) ;
extern void omp_destroy_nest_lock (omp_nest_lock_t *) ;
extern void omp_set_nest_lock (omp_nest_lock_t *) ;
extern void omp_unset_nest_lock (omp_nest_lock_t *) ;
extern int omp_test_nest_lock (omp_nest_lock_t *) ;
*/


extern double llc_get_wtime (void) ;
extern double llc_get_wtick (void) ;

void omp_set_schedule (omp_sched_t, int) ;
void omp_get_schedule (omp_sched_t *, int *) ;
int omp_get_thread_limit (void) ;
void omp_set_max_active_levels (int) ;
int omp_get_max_active_levels (void) ;
int omp_get_level (void) ;
int omp_get_ancestor_thread_num (int) ;
int omp_get_team_size (int) ;
int omp_get_active_level (void) ;
