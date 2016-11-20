#include<stdio.h>

int main (int argc, char * argv[]) {
    register int a, b;
    int i, j;
    register int * pointer_to_memory;
    
    for (i=0; i < 1000000; i++)
    {
	a = 5;
        *pointer_to_memory = a;
	b = 2; 
	*pointer_to_memory++ = b;  
    }
    return 0;
}
