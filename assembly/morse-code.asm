.text


main:	



# STUDENTS MAY MODIFY CODE BELOW
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

	## Test code that calls procedure for part A
	#jal save_our_souls

	## morse_flash test for part B
	#addi $a0, $zero, 0x42   # dot dot dash dot
	#jal morse_flash
	
	## morse_flash test for part B
	# addi $a0, $zero, 0x37   # dash dash dash
	# jal morse_flash
		
	## morse_flash test for part B
	# addi $a0, $zero, 0x32  	# dot dash dot
	 #jal morse_flash
			
	## morse_flash test for part B
	# addi $a0, $zero, 0x11   # dash
	# jal morse_flash	
	
	# flash_message test for part C
	#la $a0, test_buffer
	#jal flash_message
	
	# letter_to_code test for part D
	# the letter 'P' is properly encoded as 0x46.
	#addi $a0, $zero, 'P'
	#jal letter_to_code
	
	# letter_to_code test for part D
	# the letter 'A' is properly encoded as 0x21
	# addi $a0, $zero, 'A'
	# jal letter_to_code
	
	# letter_to_code test for part D
	# the space' is properly encoded as 0xff
	#addi $a0, $zero, ' '
	#jal letter_to_code
	
	# encode_message test for part E
	# The outcome of the procedure is here
	# immediately used by flash_message
	 la $a0, message04
	 la $a1, buffer01
	 jal encode_message
	 la $a0, buffer01
	 jal flash_message
	
	
	# Proper exit from the program.
	addi $v0, $zero, 10
	syscall

	
	
###########
# PROCEDURE
save_our_souls:

	jal seven_segment_on
	jal delay_short
	jal seven_segment_off
	jal delay_short
	jal seven_segment_on
	jal delay_short
	jal seven_segment_off
	jal delay_short
	jal seven_segment_on
	jal delay_short
	jal seven_segment_off
	jal delay_long	
	
	jal seven_segment_on
	jal delay_long
	jal seven_segment_off
	jal delay_long
	jal seven_segment_on
	jal delay_long
	jal seven_segment_off
	jal delay_long
	jal seven_segment_on
	jal delay_long
	jal seven_segment_off
	jal delay_long
	

	jal seven_segment_on
	jal delay_short
	jal seven_segment_off
	jal delay_short
	jal seven_segment_on
	jal delay_short
	jal seven_segment_off
	jal delay_short
	jal seven_segment_on
	jal delay_short
	jal seven_segment_off
	jal delay_long	
	
	
	
	jr $31


# PROCEDURE
morse_flash:

	addi $sp, $sp, -36
	sw $s0, 0($sp)
	sw $ra, 4($sp)
	sw $t3, 8 ($sp)
	sw $t4, 12 ($sp)
	sw $t5, 16 ($sp)
	sw $t6, 20 ($sp)
	sw $t7, 24 ($sp)
	sw $t8, 28($sp)
	sw $t9, 36 ($sp)
	
	li $t7, 8		# and with 0b1000 to check bit 
	li $t9, 0		#set count to 0
	li $t5, 4
	
	move $t8, $a0		#copy $a0 to $t8 to get length 
	srl $t8, $t8, 4		#to calculate offset
	
	sub $t6, $t5, $t8	#calculate the offset (amnt to shift left)
	
	sllv $a0, $a0, $t6	#shift by that amount 

loop:
	beq $t9, $t8, done	#check if loop should end
	and $t3, $a0, $t7	#check for set bit
	beq $t3, 8, equalsdash
	bne $t3, 8, equalsdot
	

	
done:	
	lw $s0, 0($sp)
	lw $ra, 4($sp)
	lw $t3, 8 ($sp)
	lw $t4, 12 ($sp)
	lw $t5, 16 ($sp)
	lw $t6, 20 ($sp)
	lw $t7, 24 ($sp)
	lw $t8, 28($sp)
	lw $t9, 32 ($sp)
	addi $sp, $sp, 36
	jr $31


equalsdash:	
	jal seven_segment_on
	jal delay_long
	jal seven_segment_off
	jal delay_short
	
	sll $a0, $a0, 1		#shift to get new bit, increment count
	add $t9, $t9, 1
	
	j loop
	

equalsdot:

	jal seven_segment_on
	jal delay_short
	jal seven_segment_off
	jal delay_short
	
	sll $a0, $a0, 1		#shift to get new bit, increment count
	add $t9, $t9, 1

	j loop
	
	
###########
# PROCEDURE
flash_message:

	addi $sp, $sp, -12
	sw $s0, 0($sp)
	sw $ra, 4($sp)
	sw $t4, 8($sp)
	sw $a0, 12($sp)
	
	move $t4, $a0
loop2:
	
	beq $a0, $zero, done2
	lbu $a0, ($t4)	
	beq $a0, 0xff, space #dont flash, take short delay to indicate space between words
	jal morse_flash

	continue1:
	addi $t4, $t4, 1
	b loop2
	
	
space:
jal delay_long
jal delay_long
j continue1


done2:
	lw $s0, 0($sp)
	lw $ra, 4($sp)
	lw $t4, 8($sp)
	lw $a0, 12($sp)
	addi $sp, $sp, 12
	jr $ra
	
	
###########
# PROCEDURE
letter_to_code:
	
	addi $sp, $sp, -32
	sw $s0, 0($sp)
	sw $ra, 4($sp)
	sw $t1, 8($sp)
	sw $t2, 12($sp)
	sw $t3, 16($sp)
	sw $t4, 20($sp)
	sw $t5, 24($sp)
	sw $t6, 28($sp)
	sw $t7, 32($sp)
	
	la $t1, codes	#load adress of codes segment
	
			
	searchList:	#traverse code segment until correct letter is found
		lbu $t2, ($t1)
		beq $t2, $a0, foundLetter	#branch if we find it
		beq $a0, 0x20, foundSpace
		addi $t1, $t1, 1	#else continue traversing down
		b searchList
foundLetter:
	li $t7, 0
	li $t3, 0 #counter
	move $t5, $t1 #preserve location
	
	looping:
		lb $t4, 1($t1)		#count how many morse characters to encode
		beq $t4, 0, decoding
		addi $t1, $t1, 1
		addi $t3, $t3, 1
		b looping
foundSpace:
	li $t3, 0xff
	j decode_done		
		
decoding:	
		li $t6, 4
		sub $t6, $t6, $t3
		sllv $t3, $t3, $t6	#create space for rest of encoding via offset shift
		
	decode_loop:
		lb $t4, 1($t5)	#traverse .byte and encode accordingly
		beq $t4, 0, decode_done
		beq $t4, '-', issaDash
		beq $t4, '.', issaDot
		continue:
		addi $t5, $t5, 1
		b decode_loop
		
issaDot:
	sll $t3, $t3, 1	#adds a zero
	b continue
issaDash:	
	sll $t3, $t3, 1	 
	ori $t3, $t3, 1	 #adds a 1
	b continue

decode_done:
	move $v0, $t3
	lw $s0, 0($sp)
	lw $ra, 4($sp)
	lw $t1, 8($sp)
	lw $t2, 12($sp)
	lw $t3, 16($sp)
	lw $t4, 20($sp)
	lw $t5, 24($sp)
	lw $t6, 28($sp)
	lw $t7, 32($sp)
	addi $sp, $sp, 32
	jr $ra	


###########
# PROCEDURE
encode_message:
	addi $sp, $sp, -16
	sw $s0, 0($sp)
	sw $ra, 4($sp)
	sw $a0, 8($sp)
	sw $a1, 12($sp)
	
	
	encode_loop:
	lbu $t1, 0($a0) #read byte from message into $t1
	beq $t1, 0x00, finish	#continue so long as byte isnt a 0
	sw $a0, 8($sp) 	#preserve $a0
	move $a0, $t1   #set $a0 to parameter value and call letter_to_code
	jal letter_to_code
	lw $a0, 8($sp) #restore $a0
	sb $v0, 0($a1) #store returned value from letter_to_code in buffer at $a1
	add $a1, $a1, 1 #increment addresses
	add $a0, $a0, 1
	j encode_loop
	
	


finish:
	lw $s0, 0($sp)
	lw $ra, 4($sp)
	lw $a0, 8($sp)
	lw $a1, 12($sp)
	addi $sp, $sp, 16

	jr $ra

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# STUDENTS MAY MODIFY CODE ABOVE

#############################################
# DO NOT MODIFY ANY OF THE CODE / LINES BELOW

###########
# PROCEDURE
seven_segment_on:
	la $t1, 0xffff0010     # location of bits for right digit
	addi $t2, $zero, 0xff  # All bits in byte are set, turning on all segments
	sb $t2, 0($t1)         # "Make it so!"
	jr $31


###########
# PROCEDURE
seven_segment_off:
	la $t1, 0xffff0010	# location of bits for right digit
	sb $zero, 0($t1)	# All bits in byte are unset, turning off all segments
	jr $31			# "Make it so!"
	

###########
# PROCEDURE
delay_long:
	add $sp, $sp, -4	# Reserve 
	sw $a0, 0($sp)
	addi $a0, $zero, 600
	addi $v0, $zero, 32
	syscall
	lw $a0, 0($sp)
	add $sp, $sp, 4
	jr $31

	
###########
# PROCEDURE			
delay_short:
	add $sp, $sp, -4
	sw $a0, 0($sp)
	addi $a0, $zero, 200
	addi $v0, $zero, 32
	syscall
	lw $a0, 0($sp)
	add $sp, $sp, 4
	jr $31




#############
# DATA MEMORY
.data
codes:
	.byte 'A', '.', '-', 0, 0, 0, 0, 0
	.byte 'B', '-', '.', '.', '.', 0, 0, 0
	.byte 'C', '-', '.', '-', '.', 0, 0, 0
	.byte 'D', '-', '.', '.', 0, 0, 0, 0
	.byte 'E', '.', 0, 0, 0, 0, 0, 0
	.byte 'F', '.', '.', '-', '.', 0, 0, 0
	.byte 'G', '-', '-', '.', 0, 0, 0, 0
	.byte 'H', '.', '.', '.', '.', 0, 0, 0
	.byte 'I', '.', '.', 0, 0, 0, 0, 0
	.byte 'J', '.', '-', '-', '-', 0, 0, 0
	.byte 'K', '-', '.', '-', 0, 0, 0, 0
	.byte 'L', '.', '-', '.', '.', 0, 0, 0
	.byte 'M', '-', '-', 0, 0, 0, 0, 0
	.byte 'N', '-', '.', 0, 0, 0, 0, 0
	.byte 'O', '-', '-', '-', 0, 0, 0, 0
	.byte 'P', '.', '-', '-', '.', 0, 0, 0
	.byte 'Q', '-', '-', '.', '-', 0, 0, 0
	.byte 'R', '.', '-', '.', 0, 0, 0, 0
	.byte 'S', '.', '.', '.', 0, 0, 0, 0
	.byte 'T', '-', 0, 0, 0, 0, 0, 0
	.byte 'U', '.', '.', '-', 0, 0, 0, 0
	.byte 'V', '.', '.', '.', '-', 0, 0, 0
	.byte 'W', '.', '-', '-', 0, 0, 0, 0
	.byte 'X', '-', '.', '.', '-', 0, 0, 0
	.byte 'Y', '-', '.', '-', '-', 0, 0, 0
	.byte 'Z', '-', '-', '.', '.', 0, 0, 0
	
message01:	.asciiz "A A A"
message02:	.asciiz "SOS"
message03:	.asciiz "WATERLOO"
message04:	.asciiz "DANCING QUEEN"
message05:	.asciiz "CHIQUITITA"
message06:	.asciiz "THE WINNER TAKES IT ALL"
message07:	.asciiz "MAMMA MIA"
message08:	.asciiz "TAKE A CHANCE ON ME"
message09:	.asciiz "KNOWING ME KNOWING YOU"
message10:	.asciiz "FERNANDO"

buffer01:	.space 128
buffer02:	.space 128
test_buffer:	.byte 0x30 0x37 0x30 0x00    # This is SOS
