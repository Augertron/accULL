#include <stdio.h>

#define N_ELEM 5120000

int main ()
{
  int i;
  int n = N_ELEM;
  double pi, sum, x;
  double mysum = 0.0;
  double h;
  double a[N_ELEM];


  h = 1.0 / (double) n;
  sum = 0.0;
 
  for (i = 0; i <= n; i++)
    {
      a[3] = a[5] * a[i];
      x = h * ((double) i - 0.5);
      sum += 4.0 / (1.0 + x * x);
    }

 pi = h * sum;

 printf ("Pi %f \n", pi);


}
