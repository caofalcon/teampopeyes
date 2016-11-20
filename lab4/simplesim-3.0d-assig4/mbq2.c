#include<stdio.h>

#define ARR_SIZE 1000000

int main() {
	int i;
	int arr[ARR_SIZE];
	int sum = 0;
	for (i=0; i < ARR_SIZE; i+=64) {
		sum += arr[i];
	}
	printf("%d\n", sum);
	return 0;
}
