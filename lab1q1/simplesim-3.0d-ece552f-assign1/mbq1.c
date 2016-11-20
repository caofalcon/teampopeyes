#include<stdio.h>

int main (int argc, char * argv[]) {
    register int a, b, c, d;
    int i, j;
    register int * pointer_to_memory;

// All commented instructions will run in the loop
/*    
  400230:	28 00 00 00 	lw $2,16($30)
  400238:	a2 00 00 00 	lui $8,1            Assuming ALU op
  400240:	51 00 00 00 	ori $8,$8,34463     STALL: 2 cycles q1, 1 cycle q2
  400248:	5b 00 00 00 	slt $2,$8,$2        STALL: 2 cycles q1, 1 cycle q2
  400250:	05 00 00 00 	beq $2,$0,400260 <main+0x70>    STALL: 2 cycles q1, 1 cycle q2
  400258:	01 00 00 00 	j 4002d8 <main+0xe8> *** NOT IN THE LOOP
*/


    for (i=0; i < 100000; i++)
    {
	// 400260:	43 00 00 00 	addiu $3,$0,5	a = 5;
	a = 5;	// ADDI: write to register r0 + imm

	// 400268:	43 00 00 00 	addiu $4,$3,4	b = a + 4;
	b = a + 4; //read after write 2 stalls in q1, 1 stall in q2 

	// 400270:	43 00 00 00 	addiu $6,$0,1  	d = 1;
	d = 1; //ADDI: no effect from previous

  	// 400278:	43 00 00 00 	addiu $4,$3,2 	b = a + 2;
	b = a + 2;  //ADDI: no effect from previous

  	// 400280:	43 00 00 00 	addiu $6,$0,1	d = 1;
	d = 1; //ADDI: no effect from previous	

  	// 400288:	42 00 00 00 	addu $5,$3,$4	c = a + b;
	c = a + b; //read after write with 1 stall for q1, no stalls with q2

  	// 400290:	43 00 00 00 	addiu $7,$7,4
	pointer_to_memory++;

  	// 400298:	34 00 00 00 	sw $5,0($7)
        *pointer_to_memory = c; // 2 stalls for q1, 1 stall for q2

  	// 4002a0:	28 00 00 00 	lw $2,0($7)	LOAD *
  	// 4002a8:	42 00 00 00 	addu $6,$5,$2	d = * + c
        d = *pointer_to_memory + c; // alu op after memory, 2 stalls for q1, 2 stalls for q2

    }

/*  
  4002b0:	28 00 00 00 	lw $8,16($30)
  4002b8:	43 00 00 00 	addiu $2,$8,1   STALL: 2 cycles q1/q2
  4002c0:	42 00 00 00 	addu $8,$0,$2   STALL: 2 cycles q1, 1 cycle q2
  4002c8:	34 00 00 00 	sw $8,16($30)   STALL: 2 cycles q1, 0 cycles q2
  4002d0:	01 00 00 00 	j 400230 <main+0x40>
*/

// 10 RAW hazards total for q1: 1 single cycle stall, 9 two cycle stalls
// 8 RAW hazards total for q2: 6 single cycle stalls, 2 two cycle stalls

    return 0;
}
