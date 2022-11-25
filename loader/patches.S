
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
	lw t0, 0x7C(v0)

	/*
	 * This mask checks for aligned accesses in either the KUSEG (0x00000000-0x007FFFFF)
	 * region, or the KSEG0 (0x80000000-0x807FFFFF) region.
	 *
	 * Most games use the KSEG0, except for Emperors New Groove (U) (SCUS-94571) whose programmers
	 * seem to prefer the KUSEG region.
	 *
	 * We cannot limit ourselves to checking the first 2MB of RAM, because some games, like
	 * Robbit Mon Dieu (J) (SCPS-10103) use a mirror (0x80600000-0x807FFFFF).
	 *
	 * This check excludes doing any reads if called from the the KSEG1 mirror
	 * (0xA0000000-0xA07FFFFF), which corresponds to calls made by the BIOS itself. Without
	 * excluding this area, Elemental Gearbolt (U) (SLUS-00654) seems to crash for not very clear
	 * reasons.
	 */
	li t2, 0x7F800003
	and t1, t2, t0
	bne t1, zero, modchipcontinue

	/*
	 * First, we will attempt to handle a version 1 antimodchip module.
	 *
	 * This checks only for the presence of a dumb modchip, by checking if the SCEx counter
	 * increments when it should not. It is also only capable of displaying the stop screen
	 * in Japanese.
	 *
	 * EnterCriticalSection for this module is called when the antipiracy check failed,
	 * and the call stack looks like this:
	 *
	 * Um Jammer Lammy (PAL-E):
	 *  - ap_check           (0x801D8008)
	 *  - ap_failed          (0x801D83E0, called from 0x801D8174)
	 *  - StopCallback       (0x800356C4, called from 0x801D8400)
	 *  - disable_ints       (0x80035B54, called from 0x800356E0)
	 *  - EnterCriticalSection
	 *
	 * For Saru! Get You (NTSC-J):
	 *  - ap_check           (0x80136950)
	 *  - ap_failed          (0x80136D28, called from 0x80136ABC)
	 *  - StopCallback       (0x8002E814, called from 0x80136D48)
	 *  - disable_ints       (0x8002ECA4, called from 0x8002E82C)
	 *  - EnterCriticalSection
	 *
	 * The return call from StopCallback to ap_failed is located at SP+0x28. We will check if
	 * at this address +0x74 exists a "li v0, 0xE6000002", which is a black rentangle passed to
	 * the DrawPrim function to clear the screen.
	 *
	 * If it exists, we will patch the thread state to return back to ap_check, as if the
	 * ap_failed function had returned.
	 */

	// Load return address from StopCallback to ap_failed
	lw t1, 0x28(t0)

	// Check if the address corresponds to user RAM and is aligned
	and t2, t1
	bne t2, zero, modchipcontinue

	// Compare signature, and test for v2 if does not match
	lw t2, 0x74(t1)
	li t3, 0x3C02E600
	bne t2, t3, modchipv2

	lw t2, 0x78(t1)
	li t3, 0x34420002
	bne t2, t3, modchipv2

	// Load return address from ap_failed to ap_check
	lw t1, 0xE8(t0)

	// Adjust stack pointer
	addi t0, 0xF0

	// Save and return
	b modchipsave

	/*
	 * We will now attempt to patch an antimodchip v2 module.
	 *
	 * This one is smarter and checks that the SCEx wobble is present in the inner tracks,
	 * to detect CD swapping; and for dumb modchips by checking for absence of the wobble
	 * in the outer tracks.
	 *
	 * EnterCriticalSection for this module is called when the antipiracy check failed,
	 * and the call stack looks like this:
	 *
	 * Rockman 2 - Dr. Wily no Nazo (J) (SLPS-02255):
	 *  - ap_check           (0x8006CA58)
	 *  - ap_failed          (0x8006D654, called from 0x8006CE5C and 0x8006D238)
	 *  - StopCallback       (0x80024524, called from 0x8006D66C)
	 *  - disable_ints       (0x800249B4, called from 0x8002453C)
	 *  - EnterCriticalSection
	 *
	 * The return address from StopCallback to ap_failed is located at SP+0x28, exactly as above
	 * so we will not load it again.
	 *
	 * For this other version, we will check if at this return address +0x10 bytes exists a
	 * "sh zero, 0x1F801DAA", which is used to mute the audio.
	 *
	 * If that exists, we will patch the thread state to return back to ap_check.
	 */
modchipv2:
	// Compare signature
	lw t2, 0x18(t1)
	li t3, 0x3C011F80
	bne t2, t3, modchipcontinue

	lw t2, 0x1C(t1)
	li t3, 0xA4201DAA
	bne t2, t3, modchipcontinue

	// Load return address to from ap_failed to ap_check
	lw t1, 0x120(t0)

	// Adjust stack pointer
	addi t0, 0x128

modchipsave:
	// Zero the s0 and s1 stored in the thread state, so the state machine used by ap_check exits
	sw zero, 0x48(v0)
	sw zero, 0x4C(v0)

	// Save adjusted stack pointer and return address
	sw t0, 0x7C(v0)
	sw t1, 0x88(v0)

.globl modchipreturn
modchipreturn:
	j 0x12341234

.globl modchipcontinue
modchipcontinue:
	j 0x12341234
