
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

.global FileOpen
FileOpen:
	li $t1, 0x00
	j 0xA0

.global FileRead
FileRead:
	li $t1, 0x02
	j 0xA0

.global FileClose
FileClose:
	li $t1, 0x04
	j 0xA0

.global strcmp
strcmp:
	li $t1, 0x17
	j 0xA0

.global strncmp
strncmp:
	li $t1, 0x18
	j 0xA0

.global todigit
todigit:
	li $t1, 0x0A
	j 0xA0

.global strlen
strlen:
	li $t1, 0x1B
	j 0xA0

.global strchr
strchr:
	li $t1, 0x1E
	j 0xA0

.global std_out_puts
std_out_puts:
	li $t1, 0x3E
	j 0xA0

.global printf
printf:
	li $t1, 0x3F
	j 0xA0

.global LoadExeFile
LoadExeFile:
	li $t1, 0x42
	j 0xA0

.global DoExecute
DoExecute:
	li $t1, 0x43
	j 0xA0

.global init_a0_b0_c0_vectors
init_a0_b0_c0_vectors:
	li $t1, 0x45
	j 0xA0

.global SetConf
SetConf:
	li $t1, 0x9C
	j 0xA0

.global CdInit
CdInit:
	li $t1, 0x54
	j 0xA0

###############
# C-FUNCTIONS #
###############

.global EnqueueTimerAndVblankIrqs
EnqueueTimerAndVblankIrqs:
	li $t1, 0x00
	j 0xC0

.global EnqueueSyscallHandler
EnqueueSyscallHandler:
	li $t1, 0x01
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
