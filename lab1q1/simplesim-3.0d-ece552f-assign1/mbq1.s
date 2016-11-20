	.file	1 "mbq1.c"

 # GNU C 2.7.2.3 [AL 1.1, MM 40, tma 0.1] SimpleScalar running sstrix compiled by GNU C

 # Cc1 defaults:
 # -mgas -mgpOPT

 # Cc1 arguments (-G value = 8, Cpu = default, ISA = 1):
 # -quiet -dumpbase -dumpconfig -O2 -o

gcc2_compiled.:
__gnu_compiled_c:
	.text
	.align	2
	.globl	main

	.extern	stdin, 4
	.extern	stdout, 4

	.text

	.loc	1 3
	.ent	main
main:
	.frame	$sp,40,$31		# vars= 8, regs= 3/0, args= 16, extra= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,40 		#  79 subsi3_internal
	sw	$16,24($sp) 		#  85 movsi_internal2/7
	move	$16,$4 		#  4 movsi_internal2/1
	sw	$31,32($sp) 		#  81 movsi_internal2/7
	sw	$17,28($sp) 		#  83 movsi_internal2/7
	jal	__main 		#  10 call_internal1
	move	$3,$0 		#  68 movsi_internal2/3
	blez	$16,$L15 		#  69 branch_zero
	li	$4,0x0000001c		# 28 		#  74 movsi_internal2/3
$L17:
	sw	$4,0($17) 		#  40 movsi_internal2/7
	addu	$3,$3,1 		#  49 addsi3_internal
	slt	$2,$3,$16 		#  19 slt_si
	bne	$2,$0,$L17 		#  20 branch_zero
$L15:
	move	$2,$0 		#  59 movsi_internal2/3
	lw	$31,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	addu	$sp,$sp,40
	j	$31
	.end	main
