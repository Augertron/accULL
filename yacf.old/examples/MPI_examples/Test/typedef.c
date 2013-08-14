int main() {
    FILE *fp;
    char *datafile = "prueba";
    if((fp = fopen(datafile,"r")) == NULL) {
        printf("cannot open 'data' file\n");
        exit(1);
    }
    printf("Hello World!");
}

