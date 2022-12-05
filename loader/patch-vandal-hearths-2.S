
#include <regdef.h>

.text

#
# This game needs a special treatment, as it has the regular antipiracy, but also an extra check
# by calling CdGetDiskType. If it detects the disc is a burned one, it aborts.
#
# Furthermore, if the BIOS is an European one, it gets stuck on a loop, calling the AP module
# forever.
#
# So we will just nuke the antipiracy call. This function is supposed to return a nonzero, but
# we do not need to patch v0 as there is a load constant into v0 right before the call.
#
.globl patch_vandal_start
patch_vandal_start:
	# Load address where the call to antipiracy sits
	la t0, 0x80040C98

	# Check if it matches "jal 0x80042854"
	lw t1, 0(t0)
	li t2, 0x0C010A15
	bne t1, t2, patch_vandal_return

	# If it does, NOP the opcode
	sw zero, 0(t0)

.globl patch_vandal_return
patch_vandal_return:
	# This will be replaced with the real address
	j 0x12341234

.globl patch_vandal_end
patch_vandal_end:
