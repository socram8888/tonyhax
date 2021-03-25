
#include <regdef.h>

.text
.globl __start

LOAD_ADDR = 0x801FC000

# s0 = A table address
# s1 = file descriptor
# s2 = realstart physical address
# s3 = B table address (on MCINIT only)

__start:
	# Restore stack pointer
	li sp, 0x801FFF00

#ifdef MCINIT

	# Disable interrupts
	mfc0 t0, $12
	li t1, 0xFFFFFBFE
	and t0, t1
	mtc0 t0, $12

#endif

	# Call ourselves to get the current program counter in ra
	bal realstart

realstart:
	# Save real start address in s2
	move s2, ra

	# Load A table address
	li s0, 0xA0

#ifdef MCINIT

#ifndef NOPAINT
	# Paint purple
	li a0, 0xFF00FF
	bal paintscr
#endif

	# Load B table address
	li s3, 0xB0

	# Call InitCard(0)
	li t1, 0x4A
	li a0, 0
	jalr s3

	# Call StartCard
	li t1, 0x4B
	jalr s3

	# Call _bu_init
	li t1, 0x55
	jalr s0

#endif

#ifndef NOPAINT
	# Paint blue
	li a0, 0xFF0000
	bal paintscr
#endif

	# Call FileOpen
	li t1, 0x00
	addi a0, s2, (splname - realstart)
	li a1, 0b00000001
	jalr s0

	# Die if failed
	blt v0, 0, fail

	# Save file handle to s1
	move s1, v0

	# Load data using FileRead
	li t1, 0x02
	move a0, s1
	li a1, LOAD_ADDR
	li a2, 0x2000
	jalr s0

	# Die if failed
	bne v0, 0x2000, fail

	# Close file
	li t1, 0x04
	move a0, s1
	jalr s0

#ifndef NOPAINT
	# Paint green
	li a0, 0x00FF00
	bal paintscr
#endif

	# Jump to it!
	li s0, LOAD_ADDR+0x100
	jr s0

fail:
#ifndef NOPAINT
	# Red
	li a0, 0x0000FF
	bal paintscr
#endif

lock:
	b lock

#ifndef NOPAINT
paintscr:
	# Add command byte to a0
	li t0, 0x02000000
	or t0, a0

	# Calculate effective address
	addi a0, s2, (redscreen - realstart)

	# Store color with command on buffer
	sw t0, 0(a0)

	# Tail call GPU_cwp to paint the entire screen
	li a1, 3
	li t1, 0x4A
	jr s0

redscreen:
	# Space for color and command
	.word 0
	# Start X and Y = 0
	.word 0x00000000
	# Width of 1024, height of 512
	.word 0x01FF03FF
#endif

splname:
	.asciiz "bu00:BESLEM-99999TONYHAX"
