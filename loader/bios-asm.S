
#include <regdef.h>

.text
.align 4

##########
# EXTRAS #
##########

.global FakeEnqueueCdIntr
FakeEnqueueCdIntr:
	lw ra, 0x14(sp)
	addi sp, 0x18
	jr ra

############
# SYSCALLS #
############

.global EnterCriticalSection
EnterCriticalSection:
	li a0, 0x01
	syscall
	jr ra

.global ExitCriticalSection
ExitCriticalSection:
	li a0, 0x02
	syscall
	jr ra

###############
# A-FUNCTIONS #
###############

.global todigit
todigit:
	li t1, 0x0A
	j 0xA0

.global strcmp
strcmp:
	li t1, 0x17
	j 0xA0

.global strncmp
strncmp:
	li t1, 0x18
	j 0xA0

.global strcpy
strcpy:
	li t1, 0x19
	j 0xA0

.global strlen
strlen:
	li t1, 0x1B
	j 0xA0

.global strchr
strchr:
	li t1, 0x1E
	j 0xA0

.global memcpy
memcpy:
	li t1, 0x2A
	j 0xA0

.global DoExecute
DoExecute:
	# Pepsiman (J) crashes if s5 is not zero
	# The BIOS leaves them s1-s6 zeroed, so we'll do the same
	li s1, 0
	li s2, 0
	li s3, 0
	li s4, 0
	li s5, 0
	li s6, 0
	li t1, 0x43
	j 0xA0

.global FlushCache
FlushCache:
	li t1, 0x44
	j 0xA0

.global init_a0_b0_c0_vectors
init_a0_b0_c0_vectors:
	li t1, 0x45
	j 0xA0

.global GPU_dw
GPU_dw:
	li t1, 0x46
	j 0xA0

.global SendGP1Command
SendGP1Command:
	li t1, 0x48
	j 0xA0

.global GPU_cw
GPU_cw:
	li t1, 0x49
	j 0xA0

.global GPU_cwp
GPU_cwp:
	li t1, 0x4A
	j 0xA0

.global LoadAndExecute
LoadAndExecute:
	li t1, 0x51
	j 0xA0

.global CdInit
CdInit:
	li t1, 0x54
	j 0xA0

.global SetConf
SetConf:
	li t1, 0x9C
	j 0xA0

.global CdReadSector
CdReadSector:
	li t1, 0xA5
	j 0xA0

###############
# B-FUNCTIONS #
###############

.global SetDefaultExitFromException
SetDefaultExitFromException:
	li t1, 0x18
	j 0xB0

.global FileOpen
FileOpen:
	li t1, 0x32
	j 0xB0

.global FileRead
FileRead:
	li t1, 0x34
	j 0xB0

.global FileClose
FileClose:
	li t1, 0x36
	j 0xB0

.global GetLastError
GetLastError:
	li t1, 0x54
	j 0xB0

.global GetC0Table
GetC0Table:
	li t1, 0x56
	j 0xB0

.global GetB0Table
GetB0Table:
	li t1, 0x57
	j 0xB0

###############
# C-FUNCTIONS #
###############

.global InstallExceptionHandlers
InstallExceptionHandlers:
	li t1, 0x07
	j 0xC0

.global InstallDevices
InstallDevices:
	li t1, 0x12
	j 0xC0

.global AdjustA0Table
AdjustA0Table:
	li t1, 0x1C
	j 0xC0
