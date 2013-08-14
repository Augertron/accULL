
typedef int tete;

int as;

void func (int a, char * b);

void func (int a, char * b)
{
   int i;

   for (i = 0; i < 10; i++) 
      i = i + 1;
}


void test(void (* func)(int, char *));


void test(void (* func)(int, char *))
{
	int a;
	char * b = "tutu";
	(*func)(a, b);
}

void foo(void) 
{
	printf(" Hello Foo ! \n");
}

void test2(void func(void))
{
	func();
}

int main() {

   int x;
   int i;

  for (i = 0; i < 10; i++)
   {
	func(i, "ab");
   }

   test(&func);
   test2(&foo);
}
