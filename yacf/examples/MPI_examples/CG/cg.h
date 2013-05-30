/* this program was originally written by itakura@rccp.tukuba.ac.jp */
/* modified to seq'ed back by msato@trc.rwcp.or.jp */
/*
 *
 * cg.h: Header file (Definition Constant, Include Common file )
 * Kernel CG: Solving and Unstructured Sparse Linear System by
 * the Conjugate Gradient Method (in NAS Parallel Benchmarks)
 * 
 */

#ifdef not
/***** macro *****/
#define daxpy(a,x,y,n) {int i;for(i=0; i<n;i++)*((y)+i)=*((y)+i)+a*(*((x)+i));}
#define daypx(a,x,y,n) {int i;for(i=0; i<n;i++)*((y)+i)=*((x)+i)+a*(*((y)+i));}
#define daxy(a,x,y,n) {int i;for(i=0; i<n;i++)*((y)+i)=a*(*((x)+i));}
#define dxy(x,y,n) {int i;for(i=0; i<n;i++){*((y)+i)=*((x)+i);}}
#define day(a,y,n) {int i;for(i=0; i<n;i++)*((y)+i)=a;}
#define dxpy(x,y,n) {int i;for(i=0; i<n;i++)*((y)+i)=(*((y)+i))+(*((x)+i));}
#define dxmy(x,y,n) {int i;for(i=0; i<n;i++)*((y)+i)=(*((x)+i))-(*((y)+i));}
#endif

/***** Problem Size Definition *****/
#define TINY 1
#define MID 2
#define LARGE 3

#ifndef SIZE
#define SIZE MID
#endif

#if (SIZE==TINY)

#define NN 1400		/* size of matrix */
#define NNZ 180000	/* */
#define NNP1 (NN+1)
#define LENWRK (NNZ+NN+1)
#define ILNWRK (2*NNZ+2*NN+1)    
#define SHIFT 10.0
#define NITCG 25
#define NITER 15 
#define NNZCHK 78148
#define ZETCHK 8.59717750786234
#define ZETTOL 1.0e-10
#define RESCHK 1.0e-10
#define NONZER 7
#define RCOND 1.0e-1

#elif (SIZE==MID)

#define NN 14000
#define NNZ 2030000
#define NNP1 (NN+1)
#define LENWRK (NNZ+NN+1)
#define ILNWRK (2*NNZ+2*NN+1)    
#define SHIFT 20.0
#define NITCG 25
#define NITER 15
#define NNZCHK 1853104
#define ZETCHK 17.13023505380784
#define ZETTOL 1.0e-10
#define RESCHK 1.0e-10
#define NONZER 11
#define RCOND 1.0e-1

#elif (SIZE==LARGE)

#define NN 75000
#define NNZ 20000000
#define NNP1 (NN+1)
#define LENWRK (NNZ+NN+1)
#define ILNWRK (2*NNZ+2*NN+1)    
#define SHIFT 60.0
#define NITCG 25
#define NITER 75
#define NNZCHK 13708072
#define ZETCHK 22.712745482078
#define ZETTOL 1.0e-10
#define RESCHK 1.0e-10
#define NONZER 13
#define RCOND 1.0e-1

#endif

/***** Macro, Grobal Variable *****/

#define min(a,b) ((a<b)?a:b)
double amult, tran;

