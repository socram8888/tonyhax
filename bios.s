
.text
.align 4

############
# SYSCALLS #
############

.global EnterCriticalSection
EnterCriticalSection:
	li $a0, 0x01
	syscall
	jr $ra

.global ExitCriticalSection
ExitCriticalSection:
	li $a0, 0x02
	syscall
	jr $ra

###############
# A-FUNCTIONS #
###############

.global strcmp
strcmp:
	li $t1, 0x17
	j 0xA0

.global strlen
strlen:
	li $t1, 0x1B
	j 0xA0

.global std_out_puts
std_out_puts:
	li $t1, 0x3E
	j 0xA0

.global init_a0_b0_c0_vectors
init_a0_b0_c0_vectors:
	li $t1, 0x45
	j 0xA0

###############
# C-FUNCTIONS #
###############

.global EnqueueTimerAndVblankIrqs
EnqueueTimerAndVblankIrqs:
	li $t1, 0x00
	j 0xC0

.global InstallExceptionHandlers
InstallExceptionHandlers:
	li $t1, 0x07
	j 0xC0

.global SysInitMemory
SysInitMemory:
	li $t1, 0x08
	j 0xC0

.global InitDefInt
InitDefInt:
	li $t1, 0x0C
	j 0xC0

.global InstallDevices
InstallDevices:
	li $t1, 0x12
	j 0xC0

.global AdjustA0Table
AdjustA0Table:
	li $t1, 0x1C
	j 0xC0
