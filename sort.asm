	add $a0, $zero, $imm, 256	 	  #$s0=array location
	add $a1, $zero, $imm, 15 	  	  #$a1= array size
	add $t0, $zero, $zero, 0          #index set to zero
loop:
	add $t1, $t0, $imm, 1     	  #index in the +1 place
	beq $imm, $t0, $a1, exit 	  #if index=16 its the end of the array and we need to exit.
	add $t2, $t0, $a0, 0	  	  #$t2 = array location + index
	lw  $a2, $t2, $zero, 0        #load number in index location (index+1)
	add $t2, $a0, $t1, 0	  	  #$t2 = array location + (index+1)
	lw  $a3, $t2, $zero, 0	  	  #load number in index+1 location
	bgt $imm, $a2, $a3, swap	  #id the number in index location is bigger the the number in the index+1 location, go to swap
	add $t0, $zero, $t1, 0	 	  #index= index+1
	beq $imm, $zero, $zero, loop  #jump back to loop
swap:
	add $t2, $a0, $t1, 0		   #$t2 = array location + (index+1)
	sw  $a2, $t2, $zero, 0		   #store in index place the (index+1) value
	add $t2, $a0, $t0, 0 	  	   #save cuurent number in index+1 location
	sw  $a3, $t2, $zero, 0	  	   #save index+1 number in current location
	add $t0, $zero, $zero, 0	   #index back to be zero
	beq $imm, $zero, $zero, loop   #jump back to loop

exit:
	halt $zero, $zero, $zero, 0