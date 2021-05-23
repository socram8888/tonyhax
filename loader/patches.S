
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

.globl modchippatch
modchippatch:
	nop

.globl modchipret
modchipret:
	nop
