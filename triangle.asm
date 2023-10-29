	lw $a0, $zero, $imm, 0x100					# $a0 will hold the address A, $s0 will hold the length AB
	lw $a1, $zero, $imm, 0x101					# $a1 will hold the adrdess B
	lw $a2, $zero, $imm, 0x102					# $a2 will hold the address C, $s2 will hold the length BC
	add $t0, $zero, $imm, 255					# $t0=255 which is the white Value
	out $t0, $zero, $imm, 21					# IO[monitordata]<=255 Ready to write the white color (because the screen starts all in black)
CalculateBC:
	sub $s2, $a2, $a1, 0						# $s2 will hold the length of BC (because they are on the same line its just C-B+1)
	add $s2, $s2, $imm, 1						# now $s2 holds the length of BC
	add $t0, $a0, $zero, 0 						# $t0=AdressOfA, preparing to calculate AB
	add $s0, $zero, $zero, 0					# $s0=0 - will hold the length of AB
CalculateAB:
	add $t0, $t0, $imm, 256						# $t0=A+256 meaning we will go to the next row until we will meet the position of B
	add $s0, $s0, $imm, 1						# $s0++ at the end $s0 will hold the length of AB
	bne $imm, $t0, $a1, CalculateAB				# while we didn't reach the address of B with $t0, keep counting
	add $s1, $zero, $zero, 0					# $s1 will hold the floor value of AB/BC or BC/AB (depends who is larger)
	blt $imm, $s0, $s2, Prep2					# if AB<BC go to Prep2
	add $t0, $s0, $zero, 0						# $t0=Length of AB
FloorOfABBC:
	sub $t0, $t0, $s2, 0 						# $t0=$t0-$s2 (AB=AB-BC at start, until we will find the value needed)
	blt $imm, $t0, $zero, GetReady1				# if $t0<0 we got the value needed in $s1 - floor value of AB/BC
	add $s1, $s1, $imm, 1						# $s1++ will hold floor(AB/BC)
	beq $imm, $zero, $zero, FloorOfABBC			# keep calulating, when the loop will end $s1=floor(AB/BC)
GetReady1:
	add $t0, $zero, $a1, 0						# $t0=adress of B (we will move it forward 1 at a time until $t0=address of C) - this will be the column that is moving right
	add $t1, $zero, $s0, 0						# $t1=length of AB - this register will hold the ammount of pixels we will need to paint (every time it will decreased in floor AB/BC)
	add $t2, $zero, $a1, 0						# $t2 will hold the address we are painting (from BC upway)
	add $a1, $zero, $imm, 1						# $a1=1 for the for loop
	beq $imm, $zero, $zero, ForLoop1			# at first we will go direcly the the for loop
WhileLoop1:
	add $t0, $t0, $imm, 1						# $t0++ (move the address of B one to the right) after we finished the for loop
	bgt $imm, $t0, $a2, Finish					# if $t0 got passed the address of C we finished painting the triangle
	sub $t1, $t1, $s1, 0						# $t1=$t1-s1 -> AB - floor(AB/BC)
	add $t2, $t0, $zero, 0 						# the address we will paint now is the previous address we handled
	add $a1, $zero, $imm, 1						# $a1=1 for the for loop
ForLoop1:
	out $t2, $zero, $imm, 20					# IO[monitoraddr]<=the address we are painting
	add $a0, $zero, $imm, 1						# $a0=1
	out $a0, $zero, $imm, 22					# IO[monitorcmd]<=1
	add $a1, $a1, $imm, 1						# a1++ for loop until AB - floor(AB/BC)
	sub $t2, $t2, $imm, 256						# go 1 row up in pixels column
	bgt $imm, $t1, $a1, ForLoop1				# if we haven't reached the number of iterations go back to the for loop
	beq $imm, $zero, $zero, WhileLoop1			# after we finished the for loop go back to the while loop
Prep2:
	add $t0, $s2, $zero, 0						# t0=Length of BC
FloorOfBCAB:
	sub $t0, $t0, $s0, 0 						# $t0=$t0-$s0 (BC=BC-AB at start, until we will find the value needed)
	blt $imm, $t0, $zero, GetReady2				# if $t0<0 we got the value needed in $s1 - floor value of BC/AB
	add $s1, $s1, $imm, 1						# $s1++ will hold floor(BC/AB)
	beq $imm, $zero, $zero, FloorOfBCAB			# keep calulating, when the loop will end $s1=floor(BC/AB)
GetReady2:
	add $t0, $zero, $a1, 0						# $t0=adress of B (we will move it forward 1 at a time until $t0=address of A) - this will be the column that is moving up
	add $t1, $zero, $s2, 0						# $t1=length of BC - this register will hold the ammount of pixels we will need to paint (every time it will decreased in floor AB/BC)
	add $t2, $zero, $a1, 0						# $t2 will hold the address we are painting (from BC rightway)
	add $a1, $zero, $imm, 1						# $a1=1 for the for loop
	beq $imm, $zero, $zero, ForLoop2			# at first we will go direcly the the for loop
WhileLoop2:
	sub $t0, $t0, $imm, 256						# $t0=$t0-256 (move the address of B one row up) after we finished the for loop
	blt $imm, $t0, $a0, Finish					# if $t0 got passed the address of C we finished painting the triangle
	sub $t1, $t1, $s1, 0						# $t1=$t1-s1 -> AB - floor(AB/BC)
	add $t2, $t0, $zero, 0 						# the address we will paint now is the previous address we handled
	add $a1, $zero, $imm, 1						# $a1=1 (for the for loop called SecondCondition1)
ForLoop2:
	out $t2, $zero, $imm, 20					# IO[monitoraddr]<=the address we are painting
	add $a2, $zero, $imm, 1						# $a0=1
	out $a2, $zero, $imm, 22					# IO[monitorcmd]<=1
	add $a1, $a1, $imm, 1						# a1++ for loop until BC - floor(BC/AB)
	add $t2, $t2, $imm, 1						# go 1 column to the right
	bgt $imm, $t1, $a1, ForLoop2				# if we haven't reached the number of iterations go back to the for loop
	beq $imm, $zero, $zero, WhileLoop2			# after we finished the for loop go back to the while loop
Finish:
	halt $zero, $zero, $zero, 0