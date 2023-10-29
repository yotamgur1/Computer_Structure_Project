	add $t0, $zero, $imm, 7						# $t0 will hold the sector handled (0-7) - starts with 6 and will go down to 0
	add $t1, $zero, $imm, 7						# $t1 will hold temp arguments - for start we will use 7 that will be the sector we will start from
	add $t2, $zero, $imm, 0						# $t2 will hold the status of the secotor handler (0-8)
InitializeBuffer:
	in $t2, $imm, $zero, 17						# $t2=diskstatus (0 if availble, 1 if busy)	
	bne $imm, $zero, $t2, InitializeBuffer		# if $t2!=0 (meaning can't recieve new data), wait until it's free
	out $t0, $imm, $zero, 15					# IO[diskector]<=7 - initially we will put sector7 values at the buffer and will add sectors 6-0 to it
	add $t1, $zero, $imm, 897					# $t1=897, 897-1024 will be the adresses in the memory that we will save at sector8 at the end of the run
	out $t1, $zero, $imm, 16					# IO[diskbuffer]<=897
	add $t1, $zero, $imm, 1						# $t1=1 - read action
	out $t1, $zero, $imm, 14					# IO[diskcmd]=1 (now addreses 897-1024 will contain the values of sector7)
ReadSectors:
	in $t2, $imm, $zero, 17						# $t2=diskstatus (0 if availble, 1 if busy)	
	bne $imm, $zero, $t2, ReadSectors			# if $t2!=0 (meaning can't recieve new data), wait until it's free
	sub $t0, $t0, $imm, 1						# $t0=$t0-1 (starts at 6 because $t0 initialize to 7 abd the values of sector 7 is at adresses 897-1024
	out $t0, $zero, $imm, 15					# IO[disksector]<=$t0 (runs from 6 to 0)
	add $t1, $zero, $imm, 1025					# $t1=1025 - 1025-1152 will hold the memory that will be added to adresses 897-1024
	out $t1, $zero, $imm, 16					# IO[diskbuffer]<=1025 
	add $t1, $zero, $imm, 1						# $t1=1 - in order to get to IO 14 diskcmd
	out $t1, $zero, $imm, 14					# IO[diskcmd]=1 - read action from the handled sector (now the content of the handled sector will be at adressess 1025-1152)
	add $a0, $zero, $imm, 128					# $a0=128 starts the loop (from 128-1)
	add $a1, $zero, $imm, 897					# $a1=897 (the adress we will add to)
	add $a2, $zero, $imm, 1025					# $a2=1025 (the adress we will add from)
AddLoop:
	lw $t1, $zero, $a1, 0						# $t1 will hold the the value that we will add to
	lw $t2, $zero, $a2, 0						# $t2 will hold the value we will add from
	add $t1, $t1, $t2, 0						# $t1=$t1+$t2
	sw $t1, $a1, $zero, 0						# save $t1 value to the location of $a1
	add $a1, $a1, $imm, 1						# $a1++
	add $a2, $a2, $imm, 1						# $a2++
	sub $a0, $a0, $imm, 1						# $a0--
	bne $imm, $zero, $a0, AddLoop				# if $a0!=0 continue the loop
	bne $imm, $zero, $t0, ReadSectors			# while we didn't hande sector0, go back to handle more sectors
WriteBufferToSector8:
	in $t2, $imm, $zero, 17						# $t2=diskstatus (0 if availble, 1 if busy)	
	bne $imm, $zero, $t0, WriteBufferToSector8	# if $t2!=0 (meaning can't recive new data), wait until it's free
	add $t1, $zero, $imm, 8						# $t1=8 in order to get the sector number
	out $t1, $zero, $imm, 15					# IO[disksector]<=8
	add $t1, $zero, $imm, 897					# $t1=897 (the address in the main memory that we will write to sector 8)
	out $t1, $zero, $imm, 16					# IO[diskbuffer]<=897
	add $t1, $zero, $imm, 2						# $t1=8 in order to put diskcmd=2 : write
	out $t1, $zero, $imm, 14					# IO[diskcmd]<=2, (cmd=write from memory-buffer to disk)
	halt $zero, $zero, $zero, 0					# halt