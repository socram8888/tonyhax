
#include <regdef.h>

.text

.globl patch_uartputc_start
patch_uartputc_start:

	# Prepare frame
	addi sp, -12
	sw s0, 4(sp)
	sw ra, 8(sp)

	# Save argument in s0
	move s0, a0

	# Call EnterCriticalSection
	li a0, 1
	syscall

	# If "\n", prefix with "\r"
	li t0, 0x0A
	bne t0, s0, notnl

	# Print "\r"
	li a0, 0x0D
	jal writebyte

notnl:
	# Print character
	move a0, s0
	jal writebyte

	# If EnterCriticalSection failed (interrupts were already disabled), do not reenable them
	beq v0, zero, return

	# Else do reenable them with ExitCriticalSection
	li a0, 2
	syscall

return:
	# Restore registers and return
	lw ra, 8(sp)
	lw s0, 4(sp)
	addi sp, 12
	jr ra

#
# UART WRITE
#
# Arguments:
#  - a0: byte to send
#
writebyte:
	# Load I/O start
	lui t3, 0x1F80

	# Add start (0) and stop (1) bits to byte
	andi a0, 0xFF
	sll a0, 1
	ori a0, 0x200

	# Bits to send (1 start + 8 data + 1 stop)
	li t1, 10

	# Set timer 0 target to 293 cycles (33868800Hz/115200bps-1)
	li t0, 293
	sw t0, 0x1108(t3)

	# Start timer 0 in:
	# - Source clock to SysClk (33868800Hz)
	# - Free-running mode
	# - Reset on reaching target value
	# - IRQ on repeat mode (can be fired multiple times)
	# - Toggle IRQ flag (bit 10) on every IRQ
	#
	# We must not use the "reached target value" flag because that seems to be affected by some
	# kind of undocumented hardware errata. In real hardware, that flag can read zero if the
	# elapsed cycles between timer start and read and target values are both even or odd.
	#
	# Also note that although we are using the IRQ bits, interrupts are actually disabled so
	# we will busy poll the corresponding bits.
	li t0, 0x04D8
	sw t0, 0x1104(t3)

	# Current timer IRQ flag status
	li t2, 0x0400

writebit:
	# Emit bit via /JOY pin of port 2.
	# We need to invert it, then put it into JOY_CTRL.13.
	# The XOR also sets the bit JOY_CTRL.2 which enables outputing the /JOY signal
	andi t0, a0, 1
	sll t0, 13
	xori t0, 0x2002
	sh t0, 0x104A(t3)

	# Shift right current buffer
	srl a0, 1

	# Decrement count while we're waiting
	addi t1, -1

	# Wait until the interrupt flag toggles
writewait:
	lw t0, 0x1104(t3)
	andi t0, 0x0400
	beq t0, t2, writewait

	# Save current IRQ flag status
	move t2, t0

	# If not done, keep going
	bne t1, zero, writebit

	jr ra

.global patch_uartputc_end
patch_uartputc_end:
