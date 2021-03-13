.text
.globl start

LOAD_ADDR = 0x801FC000

start:
	# Restore stack pointer
	li $sp, 0x801FFFF0

	# Call ourselves to get the current program counter in $ra
	bal realstart

realstart:
	# Save real start address in $s2
	move $s2, $ra

	# Call StartCard
	li $v0, 0xB0
	li $t1, 0x4B
	jalr $v0

	# Load A table address
	li $s0, 0xA0

	# Call FileOpen
	li $t1, 0x00
	addi $a0, $s2, (splname - realstart)
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
	# Display a red screen using GPU_cwp
	addi $a0, $s2, (redscreen - realstart)
	li $a1, 3
	li $t1, 0x4A
	jalr $s0

lock:
	b lock

splname:
	.asciiz "bu00:TONYHAX-SPL"

redscreen:
	# Fill screen with red
	.word 0x020000FF
	# Start X and Y = 0
	.word 0x00000000
	# Width of 1024, height of 512
	.word 0x01FF03FF
