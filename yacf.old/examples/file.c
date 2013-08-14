typedef long int FILE;


int main () {
    FILE * fp = NULL;
    if ((fp  = fopen("test", "wb+")) == NULL) 
        printf("** Error: Could not open file!\n");
    else
        printf("** Could open file \n");

}
