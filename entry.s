.text
.globl start

LOAD_ADDR = 0x801FC000

start:
	# Restore stack pointer
	li $sp, 0x801FFFF0

	# Call FileOpen
	li $t1, 0x00
	la $a0, splname
	li $a1, 0b00000001
	jal 0xA0

	# Die if failed
	beq $v0, -1, fail

	# Save file handle to s0
	move $s0, $v0

	# Load data using FileRead
	li $t1, 0x02
	move $a0, $s0
	li $a1, LOAD_ADDR
	li $a2, 0x2000
	jal 0xA0

	# Die if failed
	bne $v0, 0x2000, fail

	# Close file
	li $t1, 0x04
	move $a0, $s0
	jal 0xA0

	# Jump to it!
	j LOAD_ADDR+0x100

fail:
	j fail

splname:
	.asciiz "bu00:ORCA-SPL"
