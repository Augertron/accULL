
int main () {
    int i,j, state = 0;
    int * a;

    a = malloc(100 * sizeof(int));
    a = memset(a, 0, 100 * sizeof(int));


    #pragma acc data copyin(a[0:100])
    {
      #pragma acc kernels loop async
        for (i = 0; i < 100; i++)
            a[i] = i;

      // a[i] in the host must be 0     
      for (i = 0; i < 100; i++) {
           if (a[i] != 0)  {
				state = 1;
			    printf("a should be 0, (a[%d] = %d)\n", i, a[i]);
				break;
           }	
	  }

      #pragma acc wait

      #pragma acc update host(a[0:100]);

     // Now it must be the a[i] = i
     for (i = 0; i < 100; i++)
           if (a[i] != i)  {
			state = 1;
			printf("a should be 1, (a[%d] = %d)\n", i, a[i]);
			break;
           }	
     }
	
    return state;
}
