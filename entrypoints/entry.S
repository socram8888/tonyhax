
#include <regdef.h>

.text
.globl __start

# s0 = realstart physical address
# s1 = file descriptor
# s2 = load address
# s3 = real save size (excluding useless trailing zeros)

__start:
	# Restore stack pointer
	li sp, 0x801FFF00
	move sp, t0

	# Call ourselves to get the current program counter in ra
	bal realstart

realstart:
	# Save real start address in s0
	move s0, ra

	# Paint blue
	li a0, 0xFF0000
	bal paintscr

	# Keep only VBlank interrupts enabled
	li t1, 0x1
	lui t0, 0x1f80
	sw t1, 0x1074(t0)

waitcardready:
	# Enter critical section
	li a0, 1
	syscall

	# Call get_card_status(0)
	li t1, 0x5C
	li a0, 0
	jal 0xB0

	# If status is ready (0x01), continue
	beq v0, 0x01, restoreirqs

	# Else exit critical section to let the CPU do its magic
	li a0, 2
	syscall

	b waitcardready

restoreirqs:
	# Call SetDefaultExitFromException
	# PsyQ uses a custom exit from exception handler as a hook for VSync and other stuff,
	# which is constantly polling the memory card for changes and fucking us up.
	# Now that we're sure it left the card in a ready state, disable it.
	li t1, 0x18
	jal 0xB0

	# Call ChangeClearPad(1) so the pad IRQ clears the interrupt flags.
	# PsyQ disables it to trap VBlanks.
	li t1, 0x5B
	li a0, 1
	jal 0xB0

	# Exit critical section
	li a0, 2
	syscall

	# Call FileOpen
	li t1, 0x32
	addi a0, s0, (splname - realstart)
	li a1, 0b00000001
	jal 0xB0

	# Save handle
	move s1, v0

	# If less than zero, it failed
	bgez v0, fatalerror

	# The kernel will fail to read if we don't wait a bit (here, ~1/10th of a second)
	# This is a known issue as specified in LIBOVR46.PDF section 5-11:
	# "If read() or write() is issued immediately after open(), an error occurs"
	li t0, 1000000
busywait:
	addi t0, -1
	bnz t0, busywait

	# Load temp buffer address
	li s2, 0x801FA000

	# Load header using FileRead
	li t1, 0x34
	move a0, s1
	move a1, s2
	li a2, 0x100
	jal 0xB0

	# If we did not read the correct amount, lock
	bne v0, 0x100, fatalerror

	# Load executable load address and size
	lw s3, 0x44(s2)
	lw s2, 0x40(s2)

	# Load executable using FileRead
	li t1, 0x34
	move a0, s1
	move a1, s2
	move a2, s3
	jal 0xB0

	# If we did not read the correct amount, lock
	bne v0, s3, fatalerror

	# Paint green
	li a0, 0x00FF00
	bal paintscr

	# Jump to it!
	jr s2

fatalerror:
	# Red
	li a0, 0x0000FF
	bal paintscr

lock:
	b lock

paintscr:
	# Add command byte to a0
	li t0, 0x02000000
	or t0, a0

	# Calculate effective address
	addi a0, s0, (redscreen - realstart)

	# Store color with command on buffer
	sw t0, 0(a0)

	# Tail call GPU_cwp to paint the entire screen
	li a1, 3
	li t1, 0x4A
	j 0xA0

redscreen:
	# Space for color and command
	.word 0
	# Start X and Y = 0
	.word 0x00000000
	# Width of 1024, height of 512
	.word 0x01FF03FF

splname:
	.asciiz "bu00:BESLEM-99999TONYHAX"
