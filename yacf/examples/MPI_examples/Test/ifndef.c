#ifndef RAND_MAX
#define RAND_MAX 0x7fff
#endif

#ifndef M_PI_2
#define M_PI_2   1.57079632679489661923 
#endif

#define ndim 3
#define nparts 8192
#define nsteps 10

#define NDIM ndim
#define NPARTS nparts
#define NSTEPS nsteps


int main() {
    float x = 3.0;
    if (x < M_PI_2)
       return pow(sin(x), 2.0);
    else
       return 1.0;
    printf("Hello World!");

}
