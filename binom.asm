	add $t2, $zero, $imm, 1
	sll $sp, $t2, $imm, 11					#set sp to 2048
	lw $a0, $zero, $imm, 256	 			#$a0=n value
	lw $a1, $zero, $imm, 257 				#$a1=k value
	jal $ra, $imm, $zero, rec_binom		#jump to rec_binom and store the adress in $ra
	sw $s2, $zero, $imm, 258				#store the outcome in MEM[258]
	halt $zero, $zero, $zero, 0				#when its back from the halt below, the program has finished

rec_binom:
	sub $sp, $sp, $imm, 4	 				#allocate space in stack
	sw $ra, $sp, $zero, 0					#store adress to return to
	sw $a0, $sp, $imm, 1					#store n 
	sw $a1, $sp, $imm, 2					#store k
	sw $v0, $sp, $imm, 3					#store $v0- going to be equal to binom(n-1,k-1)
	beq $imm, $a1, $zero, final				#if (k==0) go to final
	beq $imm, $a1, $a0, final				#if (n==k) go to final
	sub $a0, $a0, $imm, 1					#n=n-1
	sub $a1, $a1, $imm, 1					#k=k-1
	jal $ra, $imm, $zero, rec_binom			#binom(n-1,k-1)
	add $v0, $s2, $zero, 0					#$v0=binom(n-1,k-1)
	add $a1, $a1, $imm, 1					#k-1 is back to k
	jal $ra, $imm, $zero, rec_binom			#binom(n-1,k)
	add $s2, $s2, $v0, 0					#$s2 = $s2 + $v0 = binom(n-1,k) + binom(n-1,k-1)
	beq $imm, $zero, $zero, return_rec		#if got here, make a recursive call with return_rec label


final:
	add $s2, $zero, $imm, 1					#return 1
	lw $ra, $sp, $zero, 0					#restore the return address from stack
	lw $a0, $sp, $imm, 1					#restore the origianl n value from stack
	lw $a1, $sp, $imm, 2					#restore the value of the second argument from stack
	lw $v0, $sp, $imm, 3					#restore $v0
	add $sp, $sp, $imm, 4					#sp back to origianl location
	beq $ra, $zero, $zero, 0				#jump back to the adress in $ra
	
return_rec:
	lw $ra, $sp, $zero, 0					#restore the return address from stack
	lw $a0, $sp, $imm, 1					#restore the origianl n value from stack
	lw $a1, $sp, $imm, 2					#restore the value of the second argument from stack
	lw $v0, $sp, $imm, 3					#restore $v0
	add $sp, $sp, $imm, 4					#sp back to origianl location
	beq $ra, $zero, $zero, 0				#jump back to the adress in $ra

