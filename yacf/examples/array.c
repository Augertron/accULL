
void testfunc (double vnd[10][10]) {
  int i, j;


  for (i = 0; i < 10; i++) {
    for (j = 0; i < 10; i++) 
     vnd[i][j] = -1.0;
    
  }

}

int main () {
  double vnd[10][20];
  int i, j;
  vnd[i][j] = 3.0;
  {
  for (i = 1; i < 10; i++) {
    for (j = i; j < 10; j++) 
     vnd[i][j] = -1.0;
    
  }
  }

  testfunc(vnd);

  printf("val: %f\n", vnd[i - 1][j -1]);
  int tutu[i - 1] = i - 1;

}
