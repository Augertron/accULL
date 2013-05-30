/* *********************************************************************
File: llc2.h
Description: Library LLC
Version: 2.0 (MPI)
Date: 27.03.2000
Index:
********************************************************************* */
#ifndef _LLC_H_
#define _LLC_H_

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LLC_NC_RES_TAG      1848

#define LLC_CODE

/**************** WORKQUEUING ******************/
#define LLC_NO_TASK -1
#define LLC_TASK_MASTER 0

#define LLC_TASK_MSG_TAG    30
#define LLC_TASK_DATA_TAG   31

#define LLC_TRUE    1
#define LLC_FALSE   0 

#define LLC_MIN_PROCESSORS_TASKQ 2

/* *********************************************************************
Global Variables Declaration
 * *********************************************************************
  unsigned NUMPROCESSORS, number of processors.
  unsigned NAME, logical processor id.
  MPI_Comm *llc_CurrentGroup, pointer to the current MPI communicator.
********************************************************************* */

typedef struct llc_mem_reg {
  unsigned long addr_start;
  unsigned long addr_stop;
  struct llc_mem_reg *next;
} llc_mem_reg;

typedef struct {
	  llc_mem_reg *ptr;
	  unsigned long size;
} llc_mem_reg_data;


typedef struct llc_var_reg {
  unsigned long base_addr;
  llc_mem_reg *ptr;
	unsigned long size;
	int num;
} llc_var_reg;


typedef struct {
  llc_var_reg *var_list;
	unsigned long total_size;
	int total_regs;
	int num_var;
} llc_var_reg_data;

typedef struct llc_addr_item {
  char *addr;
  struct llc_addr_item *next;
} llc_addr_item;

typedef struct {
  int task_id;
  llc_addr_item *addr_list;
} llc_task_info;
  
  
    


int LLC_GLOBAL_NUMPROCESSORS;
int LLC_GLOBAL_NAME;
int LLC_NUMPROCESSORS;
int LLC_NAME;

MPI_Comm *llc_CurrentGroup;
MPI_Comm llc_GlobalGroup;



#ifndef	NULL 
#define NULL ((void *)0)
#endif


/* ---------------------------------------------------------------------
Macro: LLC_CHECK_MALLOC
Description: Allocate memory and check the allocation process.
--------------------------------------------------------------------- */
#define LLC_CHECK_MALLOC(buf,buf_size,buf_type) {                    \
	(buf) = (buf_type *) malloc ((buf_size) * sizeof(buf_type));   \
	if ((buf) == NULL) {                                           \
     LLC_printMaster("Error allocating memory...\n");            \
     LLC_EXIT(-1);                                               \
  }                                                              \
}                                    

/* ---------------------------------------------------------------------
Macro: LLC_EXIT
Description: Closes a MPI environment and return with an exit code.
--------------------------------------------------------------------- */
#define LLC_EXIT(code) {                                               \
  MPI_Abort (MPI_COMM_WORLD, code);                                    \
  exit(code);                                                          \
}

/* ---------------------------------------------------------------------
Macro: LLC_printMaster
Description: Only master processor displays.
--------------------------------------------------------------------- */
#define LLC_printMaster if(LLC_GLOBAL_NAME == 0) printf

/* ---------------------------------------------------------------------
I/O functions
Only master processor works. Communication is done if it's needed.
--------------------------------------------------------------------- */

#define LLC_fread(ptr,size,items,fd) {  \
	if (LLC_GLOBAL_NAME == 0) {                  \
		fread(ptr, size, items, fd);        \
	}                                     \
	MPI_Bcast (ptr, (size * items), MPI_BYTE, 0, *llc_CurrentGroup); \
}

#define LLC_fwrite 	if(LLC_GLOBAL_NAME == 0) fwrite 




#define LLC_read(fd,ptr,size)	{   \
	if (LLC_GLOBAL_NAME == 0) {            \
		read(fd, ptr, size);          \
	}                               \
	MPI_Bcast (ptr, size, MPI_BYTE, 0, *llc_CurrentGroup); \
}

#define LLC_write 	if(LLC_GLOBAL_NAME == 0) write




#define LLC_fscanf(fd,fmt,ptr)	{ \
	if (LLC_GLOBAL_NAME != 0) {            \
		fscanf(fd, fmt, ptr);         \
	}                               \
	MPI_Bcast (ptr, sizeof((ptr)[0]), MPI_BYTE, 0, *llc_CurrentGroup); \
}

#define LLC_fscanf4(fd,fmt,arg1,arg2)	{ \
	if (LLC_GLOBAL_NAME != 0) {            \
		fscanf(fd, fmt, arg1, arg2);         \
	}                               \
	MPI_Bcast (arg1, sizeof((arg1)[0]), MPI_BYTE, 0, *llc_CurrentGroup); \
	MPI_Bcast (arg2, sizeof((arg2)[0]), MPI_BYTE, 0, *llc_CurrentGroup); \
}

#define LLC_fscanf5(fd,fmt,arg1,arg2,arg3)	{ \
	if (LLC_GLOBAL_NAME != 0) {            \
		fscanf(fd, fmt, arg1, arg2, arg3);         \
	}                               \
	MPI_Bcast (arg1, sizeof((arg1)[0]), MPI_BYTE, 0, *llc_CurrentGroup); \
	MPI_Bcast (arg2, sizeof((arg2)[0]), MPI_BYTE, 0, *llc_CurrentGroup); \
	MPI_Bcast (arg3, sizeof((arg3)[0]), MPI_BYTE, 0, *llc_CurrentGroup); \
}

#define LLC_fscanf6(fd,fmt,arg1,arg2,arg3,arg4)	{ \
	if (LLC_GLOBAL_NAME != 0) {            \
		fscanf(fd, fmt, arg1, arg2, arg3, arg4);         \
	}                               \
	MPI_Bcast (arg1, sizeof((arg1)[0]), MPI_BYTE, 0, *llc_CurrentGroup); \
	MPI_Bcast (arg2, sizeof((arg2)[0]), MPI_BYTE, 0, *llc_CurrentGroup); \
	MPI_Bcast (arg3, sizeof((arg3)[0]), MPI_BYTE, 0, *llc_CurrentGroup); \
	MPI_Bcast (arg4, sizeof((arg4)[0]), MPI_BYTE, 0, *llc_CurrentGroup); \
}


#define LLC_fprintf 					if(LLC_GLOBAL_NAME == 0) fprintf




#define LLC_scanf(fmt,ptr)	{ 		\
	if (LLC_GLOBAL_NAME == 0) {            \
		scanf(fmt, ptr);         			\
	}                               \
	MPI_Bcast (ptr, sizeof((ptr)[0]), MPI_BYTE, 0, *llc_CurrentGroup); \
}

#define LLC_printf 						if(LLC_GLOBAL_NAME == 0) printf


#define LLC_fopen(path,mode)	(LLC_GLOBAL_NAME == 0) ? fopen (path, mode) : NULL	

#define LLC_fclose						if (LLC_GLOBAL_NAME == 0) fclose


#define LLC_open(path,flags)	(LLC_GLOBAL_NAME == 0) ? open (path, flags) : 0

#define LLC_creat(path,mode)	(LLC_GLOBAL_NAME == 0) ? creat (path, mode) : 0


#define LLC_close							if (LLC_GLOBAL_NAME == 0) close


#define LLC_SEND_SLAVE(ptr,size)   MPI_Bcast (ptr, size, MPI_BYTE, 0, *llc_CurrentGroup)


/* ********************************************************************* 
Constants definition
 * *********************************************************************
  SEQUENTIAL: NUMPROCESSORS == 1
  CYCLIC: llc_nT <= NUMPROCESSORS
  GROUPS: llc_nT > NUMPROCESSORS
  LLC_NOTSET: Not set
  ERROR_CASE: 
 * ********************************************************************/
#define LLC_ERROR_CASE 	0
#define LLC_SEQUENTIAL 	1
/*#define LLC_CYCLIC     	2 */
#define LLC_GROUPS     	3
#define LLC_BLOCK     	4
#define LLC_NOTSET  		-1

#define LLC_WEIGHT_TOL  1 	/* Tolerancia para el algoritmo de asignación */
                            /* de tareas por bloques */

	/* Flags para activar/desactivar la asignación equitativa por pesos */
#define LLC_WEIGHTS			0
#define LLC_NO_WEIGHTS	1


#define	LLC_MASTER_REDUCE		0			/* Procesador encargado de recoger los datos de los procesadores */
                                  /* aplicarles la operación de reducción y difundir el resultado */


#define	LLC_TAG_REDUCE_DATA			25	
#define	LLC_TAG_REDUCE_VAL				26	

#define LLC_MAX(a,b)    ((a) > (b) ? (a) : (b))
#define LLC_MIN(a,b)    ((a) < (b) ? (a) : (b))

#define LLC_PROCSGRP(x) (llc_F[(x) + 1] - llc_F[(x)])    

#define LLC_printSpeedup(t) if (LLC_NAME == 0) printf("#%d\t%g\n", LLC_NUMPROCESSORS, (LLC_SEQ_TIME/(t)))	
		
#define LLC_printVerif(res)	printf("@NP = %d, NAME = %d. VERIFICATION = %s\n", \
																			LLC_NUMPROCESSORS, LLC_NAME, res)

/**********************************************************************
 * NC_RESULT 
 **********************************************************************/

#define  llc_nc_result_create_var_list(num) {\
           LLC_CHECK_MALLOC(llc_nc_result_list.var_list,(num),llc_var_reg); \
					 llc_nc_result_list.total_size = 0; \
				   llc_nc_result_list.total_regs = 0;  \
				   llc_nc_result_list.num_var = (num); \
}

#define  llc_nc_result_init_var_list(index,var_addr) {\
					 llc_nc_result_list.var_list[index].ptr = NULL; \
					 llc_nc_result_list.var_list[index].size = 0; \
					 llc_nc_result_list.var_list[index].num = 0; \
					 llc_nc_result_list.var_list[index].base_addr = (unsigned long) (var_addr); \
}


#define  llc_nc_result_list_save_copy(copy)    {                       \
				   copy = llc_nc_result_list;                                  \
           if (llc_nc_result_list.num_var > 0) {                       \
					   register int llc_i;                                       \
					   int llc_num_var_aux;                                      \
																							                         \
						 llc_num_var_aux = llc_nc_result_list.num_var;             \
						 llc_nc_result_create_var_list (llc_num_var_aux);          \
						 for (llc_i = 0; llc_i < llc_num_var_aux; llc_i++) {       \
							 llc_nc_result_init_var_list(llc_i,                      \
									 (copy).var_list[llc_i].base_addr);                  \
             }                                                         \
					 }                                                           \
}


/* *********************************************************************
 * PIPELINE
 *  * ********************************************************************/
#define LLC_NEXT      ((llc_name_save+1) % llc_np_save)
#define LLC_PREVIOUS  ((llc_np_save+llc_name_save-1)%llc_np_save)
#define LLC_FIRST     (*llc_t == 0)
#define LLC_LAST      ((*llc_t == llc_nTasks-1) || llc_pipe_last)
#define LLC_PIPE_TAG  1


/********************************************************************** 
                         Function prototypes 
 **********************************************************************/
/*
void llc_taskq_do_task (int LLC_TASKQ_ID, int task_id, int LLC_NUM_SLAVES, llc_task_info *llc_slave_info);
*/
/********************************************************************** 
                           Global variables
 **********************************************************************/
int llc_case;
int llc_grp;
int llc_nT;
int *llc_F;
int llc_section;
llc_var_reg_data llc_nc_result_list;

#endif







