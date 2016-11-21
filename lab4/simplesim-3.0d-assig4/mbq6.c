#include<stdio.h>

#define NUM_STRUCT 1000

typedef struct testStruct {
	int addr[16];
	int next;
	int prev;
} test_s;


int main() {
	int i, j;
	test_s arr[NUM_STRUCT];
	int sum = 0;
    for (j=0; j < 100000; j++) {
	    for (i=0; i < NUM_STRUCT; i+=i+1) {
	    	arr[i].addr[i] = 1; 
	    	arr[i].next = 1; 
	    	arr[i].next = 1; 
	    }
    }

	return 0;
}
