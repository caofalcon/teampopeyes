#include<stdio.h>

int main (void)
{
  register int i, j, k, aa, bb;
  k = 0;
  
  //32 instructions per loop (32M total), 4 conditional branches per loop (4M total) 
  //for the 2 level counter, this benchmark should yield 0 mispredictions
  for (i = 0; i < 1000000; i++) //conditional branch taken taken for 1M iterations 
  {
    	aa = i % 3; //this will be 0 evey third instruction  
  	bb = i % 2; 
  	if(aa == 0) //conditional branch 2 taken every three iterations (taken when not equal) 
		aa = 0; 
	if(bb == 0) //conditional branch 3 taken every two iterations (taken when not equal) 
		bb = 0; 
        if(aa == bb) //conditional branch 4 taken every six instructions global history register will capture this (taken when not equal) 
		aa = 0; 
		bb = 0; 
  }
  return 0;
}
