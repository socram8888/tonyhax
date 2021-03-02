.text
.globl start

LOAD_ADDR = 0x801FC000

start:
	# Restore stack pointer
	li $sp, 0x801FFFF0

	# Load table address
	li $s0, 0xA0

	# Call FileOpen
	li $t1, 0x00
	la $a0, splname
	li $a1, 0b00000001
	jalr $s0

	# Die if failed
	beq $v0, -1, fail

	# Save file handle to s1
	move $s1, $v0

	# Load data using FileRead
	li $t1, 0x02
	move $a0, $s1
	li $a1, LOAD_ADDR
	li $a2, 0x2000
	jalr $s0

	# Die if failed
	bne $v0, 0x2000, fail

	# Close file
	li $t1, 0x04
	move $a0, $s1
	jalr $s0

	# Jump to it!
	li $s0, LOAD_ADDR+0x100
	jr $s0

fail:
	b fail

splname:
	.asciiz "bu00:ORCA-SPL"
