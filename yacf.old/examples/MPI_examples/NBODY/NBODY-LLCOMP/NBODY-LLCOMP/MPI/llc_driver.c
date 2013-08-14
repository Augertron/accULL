#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LLC_CHECK_MALLOC(buf,buf_size,buf_type) {                \
  (buf) = (buf_type *) malloc ((buf_size) * sizeof(buf_type));   \
  if ((buf) == NULL) {                                           \
    fprintf(stderr, "Error allocating memory...\n");             \
    exit(-1);                                                    \
  }                                                              \
}


#define	LLC_ERROR -1
#define	LLC_OK	  0

#define LLC_MAX(a,b)   ((a) > (b) ? (a) : (b))
#define LLC_MIN(a,b)   ((a) < (b) ? (a) : (b))

#define  START_TIME()				gettimeofday(&start_time, 0);
#define  STOP_GET_TIME(a)   { \
															  gettimeofday(&end_time, 0);                            \
                                (a) = end_time.tv_sec - start_time.tv_sec;             \
  														  (a) += (end_time.tv_usec - start_time.tv_usec) / 1e6;  \
}


#define  LLC_NC_RESULT_SIZE(p)  ((p)->addr_stop - (p)->addr_start + 1)
#define  LLC_MAX_COMPACT         2
#define  LLC_NO_MAX_COMPACT	-1


typedef struct llc_mem_reg {
  unsigned long addr_start;
  unsigned long addr_stop;
  struct llc_mem_reg *next;
} llc_mem_reg;

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

void llc_nc_result_insert (void *ptr, unsigned long size, unsigned long elem_size, int index);
void llc_nc_result_destroy_var_list (void);

extern llc_var_reg_data llc_nc_result_list;

void llc_nc_result_insert (void *ptr, unsigned long size, unsigned long elem_size, int index) {
  unsigned long aux_addr_start;
  unsigned long aux_addr_stop;
  llc_mem_reg *paux, *pnew;
	int compact, num_reg;

	if ((index < 0) || (index >= llc_nc_result_list.num_var)) {
		fprintf (stderr, "\n\tError: nc_result pragma specifies a variable with index out of range!!!\n");
		return ;
	}
	

  if (size < 1) {
    fprintf (stderr, "\n\tError: nc_result pragma specifies a region of memory of equal or minor size that zero (%lu)\n\tIgnoring this region of memory\n\n", size);
    return;
  }


	
	/* Calculo la zona de inicio y final de memoria teniendo en cuenta la dirección base */
  aux_addr_start = (unsigned long) (char *) ptr - llc_nc_result_list.var_list[index].base_addr;
  aux_addr_stop = aux_addr_start + size * elem_size - 1;
 
/*	printf ("Insertando [%lu, %lu, %lu], (%lu, %lu)\n", (unsigned long) ptr, size, elem_size, aux_addr_start, aux_addr_stop); 
*/
	if (llc_nc_result_list.var_list[index].ptr == NULL) {
    /* List empty: insert the first memory region */
    LLC_CHECK_MALLOC (pnew, 1, llc_mem_reg);
    pnew->addr_start = aux_addr_start;
    pnew->addr_stop = aux_addr_stop;
    pnew->next = NULL;
    llc_nc_result_list.var_list[index].ptr = pnew;
    llc_nc_result_list.var_list[index].size = LLC_NC_RESULT_SIZE(pnew);
    llc_nc_result_list.var_list[index].num = 1;
		llc_nc_result_list.total_size = LLC_NC_RESULT_SIZE(pnew);
	  llc_nc_result_list.total_regs++;
  }
  else {
    paux = llc_nc_result_list.var_list[index].ptr;
		num_reg = 0;
		compact = LLC_ERROR;
 
    while ((paux!= NULL) && ((num_reg < LLC_MAX_COMPACT) && (LLC_MAX_COMPACT != LLC_NO_MAX_COMPACT))) {
      if (((aux_addr_start >= paux->addr_start) && (aux_addr_start <= (paux->addr_stop + 1))) || 
              ((aux_addr_stop <= paux->addr_stop) && ((aux_addr_stop + 1) >= paux->addr_start)) ||
	      ((aux_addr_start <= paux->addr_start) && (aux_addr_stop >= paux->addr_stop)))  {
	/* the new memory region is contiguous with or including in an existing region. Joining both... */
        llc_nc_result_list.var_list[index].size -= LLC_NC_RESULT_SIZE(paux);
			  llc_nc_result_list.total_size -= LLC_NC_RESULT_SIZE(paux);
        paux->addr_start = LLC_MIN (paux->addr_start, aux_addr_start);
        paux->addr_stop = LLC_MAX (paux->addr_stop, aux_addr_stop);
        llc_nc_result_list.var_list[index].size += LLC_NC_RESULT_SIZE(paux);
			  llc_nc_result_list.total_size += LLC_NC_RESULT_SIZE(paux);
				compact = LLC_OK;
        paux = NULL;
      }
     else {
				num_reg++;
        paux = paux->next;
      }
		}
    if (compact == LLC_ERROR) {
      /* The new memory region is not contiguous */ 
      /* Insert a new memory region */
      LLC_CHECK_MALLOC (pnew, 1, llc_mem_reg);
      pnew->addr_start = aux_addr_start;
      pnew->addr_stop = aux_addr_stop;
      pnew->next = llc_nc_result_list.var_list[index].ptr;
      llc_nc_result_list.var_list[index].ptr = pnew;
      llc_nc_result_list.var_list[index].size += LLC_NC_RESULT_SIZE(pnew);
      llc_nc_result_list.var_list[index].num++;
			llc_nc_result_list.total_regs++;
			llc_nc_result_list.total_size += LLC_NC_RESULT_SIZE(pnew);
    }
  }

  return ;
}


void llc_nc_result_destroy_var_list (void) {
  register int i;
	llc_mem_reg *paux;

	for (i = 0; i < llc_nc_result_list.num_var; i++) {
		while (llc_nc_result_list.var_list[i].ptr != NULL) {
		  paux = llc_nc_result_list.var_list[i].ptr;
		  llc_nc_result_list.var_list[i].ptr = paux->next;
			free (paux);
		}
	}
	free (llc_nc_result_list.var_list);
	llc_nc_result_list.var_list = NULL;
	llc_nc_result_list.total_size = 0;
	llc_nc_result_list.total_regs = 0;
	llc_nc_result_list.num_var = 0;
}
		
	
