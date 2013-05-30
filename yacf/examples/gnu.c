
typedef unsigned size_t;
typedef int int8_at;

typedef int int8_ct __attribute__ (( ));
typedef int int8_t __attribute__ ((__mode__ (__QI__))) __attribute__ ((unused));
extern int abs (int __x)  __attribute__ ((__nothrow__)) __attribute__ ((__const__));

struct __attribute__((aligned(2))) char2;


static __inline__ struct cudaPitchedPtr make_cudaPitchedPtr(void *d, size_t p, size_t xsz, size_t ysz)
{
  struct cudaPitchedPtr s;
  s.ptr = d;
  s.pitch = p;
  s.xsize = xsz;
  s.ysize = ysz;
  return s;
}


int main (const char * t) {
   int8_t i;
   i = 3;
   signed char c = '3';
}
