#	code
_TestArrayAssign:
	# void TestArrayAssign()
	      	   push 	r13
	      	   push 	r12
	      	   mov  	r12,r14
	      	   sub  	r14,r0,53
	      	   push 	r3
	      	   push 	r4
	      	   mov  	r5,r12,-23
	      	   mov  	r3,r5
	      	   mov  	r5,r12,-53
	# 	int x[3][5];
	      	   mov  	r4,r5
	# 	y[0] = 1;
	      	   mov  	r5,r0,1
	      	   sto  	r5,r3
	# 	y[1] = 2;
	      	   mov  	r5,r0,2
	      	   sto  	r5,r3,1
	# 	y[2] = 2;
	      	   mov  	r5,r0,2
	      	   sto  	r5,r3,2
	# 	y[3] = 2;
	      	   mov  	r5,r0,2
	      	   sto  	r5,r3,3
	# 	y[4] = 2;
	      	   mov  	r5,r0,2
	      	   sto  	r5,r3,4
	# 	y[5] = 2;
	      	   mov  	r5,r0,2
	      	   sto  	r5,r3,5
	# 	y[6] = 2;
	      	   mov  	r5,r0,2
	      	   sto  	r5,r3,6
	# 	y[7] = 2;
	      	   mov  	r5,r0,2
	      	   sto  	r5,r3,7
	# 	y = {1,2,3,4,5,6,7,8};
	      	   mov  	r5,r3
	      	   mov  	r6,r5
	      	   mov  	r7,r0,1
	      	   sto  	r7,r6,0
	      	   mov  	r7,r0,2
	      	   sto  	r7,r6,1
	      	   mov  	r7,r0,3
	      	   sto  	r7,r6,2
	      	   mov  	r7,r0,4
	      	   sto  	r7,r6,3
	      	   mov  	r7,r0,5
	      	   sto  	r7,r6,4
	      	   mov  	r7,r0,6
	      	   sto  	r7,r6,5
	      	   mov  	r7,r0,7
	      	   sto  	r7,r6,6
	      	   mov  	r7,r0,8
	      	   sto  	r7,r6,7
	# 	x[0][0] = 1;
	      	   mov  	r5,r0,1
	      	   sto  	r5,r12,-15
	# 	x[0][1] = 2;
	      	   mov  	r5,r12,-15
	      	   mov  	r6,r0,2
	      	   sto  	r6,r5,1
	# 	x[0][2] = 2;
	      	   mov  	r5,r12,-15
	      	   mov  	r6,r0,2
	      	   sto  	r6,r5,2
	# 	x[0][3] = 2;
	      	   mov  	r5,r12,-15
	      	   mov  	r6,r0,2
	      	   sto  	r6,r5,3
	# 	x[0][4] = 2;
	      	   mov  	r5,r12,-15
	      	   mov  	r6,r0,2
	      	   sto  	r6,r5,4
	# 	x[1][0] = 3;
	      	   mov  	r5,r12,-15
	      	   mov  	r6,r0,3
	      	   sto  	r6,r5,5
	# 	x[1][1] = 3;
	      	   mov  	r5,r12,-15
	      	   mov  	r6,r0,3
	      	   sto  	r6,r5,6
	# 	x[1][2] = 3;
	      	   mov  	r5,r12,-15
	      	   mov  	r6,r0,3
	      	   sto  	r6,r5,7
	# 	x[1][3] = 3;
	      	   mov  	r5,r12,-15
	      	   mov  	r6,r0,3
	      	   sto  	r6,r5,8
	# 	x[1][4] = 5;
	      	   mov  	r5,r12,-15
	      	   mov  	r6,r0,5
	      	   sto  	r6,r5,9
	# 	x[2][0] = 5;
	      	   mov  	r5,r12,-15
	      	   mov  	r6,r0,5
	      	   sto  	r6,r5,10
	# 	x[2][1] = 5;
	      	   mov  	r5,r12,-15
	      	   mov  	r6,r0,5
	      	   sto  	r6,r5,11
	# 	x[2][2] = 5;
	      	   mov  	r5,r12,-15
	      	   mov  	r6,r0,5
	      	   sto  	r6,r5,12
	# 	x[2][3] = 5;
	      	   mov  	r5,r12,-15
	      	   mov  	r6,r0,5
	      	   sto  	r6,r5,13
	# 	x[2][4] = 5;
	      	   mov  	r5,r12,-15
	      	   mov  	r6,r0,5
	      	   sto  	r6,r5,14
	# 	z[0][0][0] = 1;
	      	   mov  	r5,r0,1
	      	   sto  	r5,r4
	# 	z[0][0][1] = 1;
	      	   mov  	r5,r0,1
	      	   sto  	r5,r4,1
	# 	z[0][0][2] = 1;
	      	   mov  	r5,r0,1
	      	   sto  	r5,r4,2
	# 	z[0][0][3] = 1;
	      	   mov  	r5,r0,1
	      	   sto  	r5,r4,3
	# 	z[0][0][4] = 1;
	      	   mov  	r5,r0,1
	      	   sto  	r5,r4,4
	# 	z[0][1][0] = 2;
	      	   mov  	r5,r0,2
	      	   sto  	r5,r4,5
	# 	z[0][1][1] = 2;
	      	   mov  	r5,r0,2
	      	   sto  	r5,r4,6
	# 	z[0][1][2] = 2;
	      	   mov  	r5,r0,2
	      	   sto  	r5,r4,7
	# 	z[0][1][3] = 2;
	      	   mov  	r5,r0,2
	      	   sto  	r5,r4,8
	# 	z[0][1][4] = 2;
	      	   mov  	r5,r0,2
	      	   sto  	r5,r4,9
	# 	z[0][2][0] = 2;
	      	   mov  	r5,r0,2
	      	   sto  	r5,r4,10
TestArrayAssign_4:
	      	   pop  	r4
	      	   pop  	r3
	      	   mov  	r14,r12
	      	   pop  	r12
	      	   pop  	r13
	      	   mov  	r15,r13




_TestArrayAssign2:
	# void TestArrayAssign()
	      	   push 	r13
	      	   push 	r12
	      	   mov  	r12,r14
	# 	int x[3][5];
	      	   dec  	r14,15
	# 	x[2] = {10,9,8,7,6};
	      	   mov  	r6,r0,10
	      	   mov  	r7,r12,-15
	      	   mov  	r5,r6
	      	   add  	r5,r7
	      	   mov  	r6,r5
	      	   mov  	r7,r0,10
	      	   sto  	r7,r6,0
	      	   mov  	r7,r0,9
	      	   sto  	r7,r6,1
	      	   mov  	r7,r0,8
	      	   sto  	r7,r6,2
	      	   mov  	r7,r0,7
	      	   sto  	r7,r6,3
	      	   mov  	r7,r0,6
	      	   sto  	r7,r6,4
TestArrayAssign_8:
	      	   mov  	r14,r12
	      	   pop  	r12
	      	   pop  	r13
	      	   mov  	r15,r13




_TestArrayAssign3:
	# void TestArrayAssign()
	      	   push 	r13
	      	   push 	r12
	      	   mov  	r12,r14
	      	   sub  	r14,r0,63
	      	   push 	r3
	# 	int j,k,m;
	      	   push 	r4
	# 	for (m = 0; m < 3; m++) {
	      	   sto  	r0,r12,-3
TestArrayAssign_12:
	      	   ld   	r5,r12,-3
	      	   cmp  	r5,r0,3
	      	pl.mov  	r15,r0,TestArrayAssign_13
	# 		for (j = 0; j < 4; j++) {
	      	   mov  	r4,r0,0
TestArrayAssign_16:
	      	   cmp  	r4,r0,4
	      	pl.mov  	r15,r0,TestArrayAssign_17
	# 			for (k = 0; k < 5; k++)
	      	   mov  	r3,r0,0
TestArrayAssign_20:
	      	   cmp  	r3,r0,5
	      	pl.mov  	r15,r0,TestArrayAssign_21
	# 				x[m][j][k] = rand();
	      	   mov  	r6,r0,5
	      	   mov  	r1,r4
	      	   mov  	r2,r6
	      	   jsr  	r13,r0,_mulu
	      	   mov  	r6,r1
	      	   push 	r5
	      	   ld   	r5,r12,-3
	      	   push 	r6
	      	   mov  	r6,r0,20
	      	   mov  	r1,r5
	      	   mov  	r2,r6
	      	   jsr  	r13,r0,_mulu
	      	   mov  	r5,r1
	      	   mov  	r6,r12,-63
	      	   mov  	r7,r5
	      	   add  	r7,r6
	      	   mov  	r5,r6
	      	   add  	r5,r7
	      	   pop  	r6
	      	   mov  	r6,r3
	      	   add  	r3,r5
	      	   pop  	r5
	      	   jsr  	r13,r0,_rand
	      	   sto  	r1,r3
	      	   inc  	r3,1
	      	   mov  	r15,r0,TestArrayAssign_20
TestArrayAssign_21:
	      	   inc  	r4,1
	      	   mov  	r15,r0,TestArrayAssign_16
TestArrayAssign_17:
	      	   ld   	r5,r12,-3
	      	   inc  	r5,1
	      	   sto  	r5,r12,-3
	      	   mov  	r15,r0,TestArrayAssign_12
TestArrayAssign_13:
	      	   pop  	r4
	      	   pop  	r3
	      	   mov  	r14,r12
	      	   pop  	r12
	      	   pop  	r13
	      	   mov  	r15,r13




#	rodata
#	extern	_rand
#	global	_TestArrayAssign
#	global	_TestArrayAssign2
#	global	_TestArrayAssign3
