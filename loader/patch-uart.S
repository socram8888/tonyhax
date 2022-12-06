
#include <regdef.h>

.text

#
# This is a complete replacement for the original std_out_putc the BIOS has.
#
# In this function:
#  - a0: data to bitbang
#  - t0: working registers
#  - t1: bits to send
#  - t2: last IRQ flag value
#  - t3: 0x1F800000 (I/O start address)
#  - t4: COP0 SR value
#
.globl patch_uartputc_start
patch_uartputc_start:
	# Add start (0) and stop (1) bits to byte
	andi a0, 0xFF
	sll a0, 1
	ori a0, 0x200

	# Newline (after processing)
	li t0, (0x0A << 1 | 0x200)

	# Bits to send (1 start + 8 data + 1 stop)
	li t1, 10

	# Compare against newline (0x0A)
	bne a0, t0, notnl

	# If newline, prepend a 0x0D, like the original function did and increment bit count
	sll a0, 10
	ori a0, (0x0D << 1 | 0x200)
	addi t1, 10

notnl:
	# We will directly manipulate the COP0 status registers instead of using EnterCriticalSection
	# to avoid other threads/interrupts from fucking up the timing.
	#
	# The reason is two-fold:
	#  - The kernel does not support reentrant calls - if something calls us while we are
	#    executing kernel code and we generate a syscall, we'd nuke the current thread state.
	#
	#  - SetConf calls printf while re-configuring the TCBs (thread control blocks). Executing
	#    *any* interrupt at that point (which includes syscalls) will cause the interrupt
	#    handler to write the current thread state to the zero address, wiping the interrupt
	#    trampoline at 0x80.
	#
	# By directly manipulating this register we're opening ourselves to all kinds of race
	# conditions, but since this is just for debugging tonyhax, that's good enough for me.

	# Load current SR state in t4
	mfc0 t4, $12

	# Clear bits 10 and 0, the same flags WarmBoot clears
	li t0, 0xFFFFFBFE
	and t0, t4
	mtc0 t0, $12

	# Load I/O start
	lui t3, 0x1F80

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

	# Restore coprocessor flags
	mtc0 t4, $12

	jr ra

.global patch_uartputc_end
patch_uartputc_end:
