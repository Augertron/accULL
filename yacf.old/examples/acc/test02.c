// Test qeu realice un cómputo simple
// Por ejemplo: suma o producto de vectores
#include <stdio.h>

#define TOL 1.0e-10

int main() {

        int n = 100;
        
        double *a;
        double *b;
        double *c;
        double *d;
        
        a = (double *)malloc(n*sizeof(double));
        b = (double *)malloc(n*sizeof(double));
        c = (double *)malloc(n*sizeof(double));
        d = (double *)malloc(n*sizeof(double));
        
        int i;
        
        /* Initialization */
        for (i = 0; i < n; i++) {
                a[i] = i; b[i] = 1;
                d[i] = a[i] + b[i];
        }

        /* Vector addition with schuduling */
        #pragma acc data copyin(a[0:n],b[0:n],n) copyout(c[0:n]) 
        {
            #pragma acc kernels loop private(i) 
            for (i = 0; i < n; i++) {
                    c[i] = a[i] + b[i];
            }
        }

        /* Test */
        for (i = 0; i < n; i++)
            if (fabs(c[i]-d[i]) > TOL)
            {
                printf("ERROR in %d: result %f != %f\n",
                       i, c[i], d[i]);
                return 1;
            }
        
        printf("OK\n");
        free(a);
        free(b);
        free(c);
        free(d);
        return 0;
}

