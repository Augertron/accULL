

int main() {
    int a[100];    

    #pragma acc parallel num_workers(3) num_gangs(2) vector_length(2)
    for (int i = 0; i < 100; i++) {
        a[i] = 3;
    }

}
