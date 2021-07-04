
#include <regdef.h>

.text
.globl __start
__start:

/*
 * The anti-FreePSXBoot patch.
 *
 * This patch is called right at the very end of the last step in the read sector finite state
 * machine:
 * https://github.com/grumpycoders/pcsx-redux/blob/f6484e8010a40a81e4019d9bfa1a9d408637b614/src/mips/openbios/sio0/card.c#L194
 *
 * When this code is executed, the registers are as follows:
 *   - v0 contains 1, or "success".
 *   - a1 contains the read buffer
 *   - a2 contains the current sector number
 *
 * If the sector being read is sector 0 and it contains "FPBZ" at +0x7C, we modify the read data
 * so it is detected as corrupted and the game skips reading from it
 *
 * The offsets have been checked against BIOSes 2.2, 3.0, 4.1 and 4.4
 */
.globl antifpxpatch
antifpxpatch:
	lw t0, 0x7C(a1)
	li t1, 0x5A425046
	bne a2, 0, fpxret
	bne t0, t1, fpxret

	sw zero, 0(a1)
fpxret:
	j 0x5B54

/*
 * Intercepts the syscall(1) (aka EnterCriticalSection).
 *
 * When this code is executed, the registers are as follows:
 *   - v0: saved thread registers, must NOT be modified.
 * The rest of the registers are not critical and can be used freely.
 */
.globl modchipstart
modchipstart:
	// Load the SP value
	lw t0, 124(v0)

	/*
	 * Ensure SP is not in BIOS area, or else Elemental Gearbolt (U) (SLUS-00654) crashes.
	 * No idea why this is happening - it works fine in no$psx but fails in real hardware.
	 */
	li t2, 0xFFE00003
	li t3, 0x80000000
	and t1, t2, t0
	bne t1, t3, modchipcontinue

	/*
	 * 0x28 bytes after this position, there might be the return address to the
	 * antipiracy_triggered function.
	 */
	lw t1, 0x28(t0)

	// Check mask to ensure we will cause an unaligned word read.
	and t2, t1
	bne t2, t3, modchipcontinue

	/*
	 * Okay, now that we have established it is safe to read, check if it contains a
	 * "sh zero, 0x1F801DAA" 16 bytes after the return address
	 *
	 * This is used to mute the sound so the antipiracy screen is silent.
	 */
	lw t2, 0x18(t1)
	li t3, 0x3C011F80
	lw t4, 0x1C(t1)
	li t5, 0xA4201DAA
	bne t2, t3, modchipcontinue
	bne t4, t5, modchipcontinue

	// Patch registers!

	// Load return address to antipiracy function, which is at sp+0x120
	lw t1, 0x120(t0)

	// The antipiracy function returns cleanly if s0 and s1 are zero, so clear s0 now
	sw zero, 0x48(v0)

	// Set PC to the return address
	sw t1, 0x88(v0)

	// Clear s1 now
	sw zero, 0x4C(v0)

	// Decrement the SP
	addi t0, 0x128
	sw t0, 0x7C(v0)

.globl modchipreturn
modchipreturn:
	j 0x12341234

.globl modchipcontinue
modchipcontinue:
	j 0x12341234
