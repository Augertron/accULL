int main()
{
    int i;
    int sum_test[10];

    for (i = 0; i <= 10; i++) {
	sum_test[i] = i;
    }

#pragma omp parallel shared(sum_test) private(i)
{
    #pragma omp for /* reduction(+ : sum_test) */
    for (i = 0; i <= 10; i++) {
	sum_test[i] = i;
    }
}

#pragma omp parallel for shared(sum_test) private(i)
   for (i = 0; i <= 10; i++) {
	sum_test[i] = i;
    }


   sum_test[i] = ' ';
}
