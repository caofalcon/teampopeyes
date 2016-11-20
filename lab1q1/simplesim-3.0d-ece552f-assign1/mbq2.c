#include<stdio.h>

int main (int argc, char * argv[]) {
    register int a, b, c, d;
    int i, j;
    register int * pointer_to_memory;
    
    for (i=0; i < 1000000; i++)
    {
	a = 5;
	b = a + 4;
	c = b + 2;
	d = 1;	
        *pointer_to_memory = b;
    }
    return 0;
}
