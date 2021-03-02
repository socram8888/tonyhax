.text
.globl start

start:
	# Restore stack pointer
	li $sp, 0x801FFFF0

	# The following is adapted from the WarmBoot call

	# Do something with the SR
	mfc0 $t0, $12
	and $t0, 0xFFFFFBFE
	mtc0 $t0, $12

	# Call "init_a0_b0_c0_vectors"
	li $t1, 0x45
	jal 0xA0

	# Call "AdjustA0Table"
	li $t1, 0x1C
	jal 0xC0

	# Call "InstallExceptionHandlers"
	li $t1, 0x07
	jal 0xC0

	# Call "InstallDevices" with TTY=1 (DISABLE IN PRODUCTION!!!)
	li $a0, 1
	li $t1, 0x12
	jal 0xC0

	# Call "SysInitMemory"
	li $a0, 0xA000E000
	li $a1, 0x2000
	li $t1, 0x08
	jal 0xC0

	# Call "InitDefInt" with priority=3
	li $a0, 3
	li $t1, 0x0C
	jal 0xC0

	# Call "EnqueueTimerAndVblankIrqs"
	li $a0, 1
	li $t1, 0x00
	jal 0xC0

	# End of code adapted

	# Send start message using puts
	la $a0, startmsg
	jal puts

	# Reset CD, so we can re-init it later
	li $a0, 0x1C
	li $a1, 0
	li $a2, 0
	jal cd_command

	# Wait for int
	jal cd_wait_int
	li $t0, 3
	la $a0, ctrlresetfail
	bne $t0, $v0, log_and_lock

	# Busy loop until it resets
	li $t0, 0x400000
waitreset:
	sub $t0, 1
	bne $t0, $zero, waitreset

	# Get region
	li $a0, 0x19
	la $a1, regsubcmd
	li $a2, 1
	jal cd_command

	# Check INT
	jal cd_wait_int
	la $a0, unsupportedmsg
	li $t0, 3
	bne $t0, $v0, log_and_lock

	# Check reply. If failed it means it is a vC0 and too old
	jal cd_read_reply

	# Check length
	li $t0, 7
	blt $v0, $t0, log_and_lock

	# Add newline for logging
	la $t0, cd_reply
	add $v0, $t0
	li $t0, '\n'
	sb $t0, 0($v0)
	sb $zero, 1($v0)

	# Log region message using puts
	la $a0, regionmsg
	jal puts
	la $a0, cd_reply+4
	jal puts

	# Use fourth byte for telling the version.
	# "for Europe" 
	# "for U/C"
	# "for Japan"
	# "for NETNA"

	lbu $t0, cd_reply+4

	la $t2, p5eur
	li $t1, 'E'
	beq $t0, $t1, start_backdoor

	la $t2, p5usa
	li $t1, 'U'
	beq $t0, $t1, start_backdoor

	la $a0, unsupportedreg
	j log_and_lock

start_backdoor:

	# Save fifth message pointer
	sw $t2, region_msg

	# Initial command, with no data
	li $a0, 0x50
	la $a1, p0
	jal backdoor_cmd

	# First string
	li $a0, 0x51
	la $a1, p1
	jal backdoor_cmd

	# Second
	li $a0, 0x52
	la $a1, p2
	jal backdoor_cmd

	# Third
	li $a0, 0x53
	la $a1, p3
	jal backdoor_cmd

	# Fourth
	li $a0, 0x54
	la $a1, p4
	jal backdoor_cmd

	# Fifth
	li $a0, 0x55
	lw $a1, region_msg
	jal backdoor_cmd

	# Final command, with no data either
	li $a0, 0x56
	la $a1, p0
	jal backdoor_cmd

	# Log message done
	la $a0, backdoormsg
	jal puts

	# Initialize CD unit
	li $t1, 0x54
	jal 0xA0

waitdoorfail:

	# Wait for door open
	la $a0, waitdooropenmsg
	jal puts
	la $a0, 0x10
	jal wait_door_status

	# Wait for door close
	la $a0, waitdoorclosemsg
	jal puts
	la $a0, 0x00
	jal wait_door_status

	# Initialize CD unit
	li $t1, 0x54
	jal 0xA0

	# Call chdir to update table path
	la $a0, cdrompath
	li $t1, 0x40
	jal 0xB0

	# If succeeded then try loading
	bne $v0, 0, tryloadconfig
	
	# Else log error and retry
	la $a0, chdirfail
	jal puts
	j waitdoorfail

tryloadconfig:
	la $a0, donemsg

log_and_lock:
	jal puts

waitforever:
	j waitforever

# Send backdoor command. Locks up if fails.
# a0 = command
# a1 = string
backdoor_cmd:
	# Save saved registers
	sub $sp, $sp, 12
	sw $a0, 0($sp)
	sw $a1, 4($sp)
	sw $ra, 8($sp)

	# Call strlen
	move $a0, $a1
	li $t1, 0x1B
	jal 0xA0

	# Prepare arguments
	lw $a0, 0($sp)
	lw $a1, 4($sp)
	move $a2, $v0

	# Send command
	jal cd_command

	# Wait for int
	jal cd_wait_int

	# Check if INT5, else fail
	la $a0, intfailmsg
	li $t1, 5
	bne $v0, $t1, log_and_lock

	# Read reply
	jal cd_read_reply

	# Check if reply is OK
	la $a0, replyfailmsg
	li $t0, 2
	bne $v0, $t0, log_and_lock

	# TODO: returns 0x03 on the emulator but should be 0x11?
	#lbu $t0, cd_reply+0
	#li $t1, 0x11
	#bne $t0, $t1, log_and_lock

	lbu $t0, cd_reply+1
	li $t1, 0x40
	bne $t0, $t1, log_and_lock

	# Return
	lw $ra, 8($sp)
	add $sp, $sp, 8
	jr $ra

# Execute CMD command, and waits for its execution
# a0 = command
# a1 = parameter pointer
# a2 = parameter length
#
# t0 = CDROM base address
# t1 = work register
# t2 = literal "1"
cd_command:
	# Load CDROM I/O port and "1" constant
	lui $t0, 0x1F80
	li $t2, 1

	# Set index to one
	sb $t2, 0x1800($t0)

	# Clear parameter FIFO
	li $t1, 0x40
	sb $t1, 0x1803($t0)

cd_command_busy:
	# Wait for busy bit to go low
	lbu $t1, 0x1800($t0)
	and $t1, $t1, 0x80
	bne $t1, $zero, cd_command_busy

	# Go to index 0
	sb $zero, 0x1800($t0)

cd_command_copy_next:
	# If done, exit loop
	beq $a2, $zero, cd_command_copy_done

	# Load and increment
	lbu $t1, 0($a1)
	add $a1, 1

	# Write to buffer
	sb $t1, 0x1802($t0)

	# Decrement byte count
	sub $a2, 1

	j cd_command_copy_next

cd_command_copy_done:
	# Go to index 1
	sb $t2, 0x1800($t0)

	# Disable interrupts (we'll poll)
	sb $zero, 0x1802($t0)

	# Acknowledge, if any
	li $t1, 7
	sb $t1, 0x1803($t0)

	# Go back to index 0
	sb $zero, 0x1800($t0)

	# Start command
	sb $a0, 0x1801($t0)

	# Return
	jr $ra

# Reads reply from the CD
# Returns int type on v0
cd_wait_int:
	# Load CDROM I/O port
	lui $t0, 0x1F80

_cd_wait_int_busy:
	# Wait for busy bit to go low again
	lbu $t1, 0x1800($t0)
	and $t1, 0x80
	bne $t1, $zero, _cd_wait_int_busy

	# Go to index 1
	li $t1, 1
	sb $t1, 0x1800($t0)

	# Wait for any interrupt to happen
_cd_wait_int_int:
	lbu $v0, 0x1803($t0)
	and $v0, 0x7
	beq $v0, $zero, _cd_wait_int_int

	jr $ra

# Reads reply from the CD
# Returns length on v0
#
# v0 = length
# t0 = CDROM base address
# t1 = response pointer
# t2 = work reg
cd_read_reply:
	# Load CDROM I/O port
	lui $t0, 0x1F80

	# Go to index 1
	li $t1, 1
	sb $t1, 0x1800($t0)

	# Read reply now
	li $v0, 0
	la $t1, cd_reply

_cd_read_next:
	# If empty, finish
	lbu $t2, 0x1800($t0)
	and $t2, 0x20
	beq $t2, $zero, _cd_read_done

	# Read byte
	lbu $t2, 0x1801($t0)
	sb $t2, 0($t1)

	# Increment counter
	add $v0, $v0, 1
	add $t1, $t1, 1
	j _cd_read_next

_cd_read_done:
	jr $ra

# Puts function
puts:
	li $t1, 0x3E
	j 0xA0

wait_door_status:
	# Save saved registers
	sub $sp, 8
	sw $a0, 0($sp)
	sw $ra, 4($sp)

	move $s0, $a0
	move $s1, $ra

_wait_door_status_loop:
	la $a0, cd_reply
	li $t1, 0xA6
	jal 0xA0

	# If failed for whatever reason, just retry
	beq $v0, -1, _wait_door_status_loop

	# Check if tray open
	lw $a0, 0($sp)
	and $v0, 0x10
	bne $v0, $a0, _wait_door_status_loop

	# If matches wanted, return
	lw $ra, 4($sp)
	add $sp, 8
	jr $ra

region_msg:
	.word 0

cd_reply:
	.space 16

startmsg:
	.asciiz "=START=\n"

ctrlresetfail:
	.asciiz "CDresfail\n"

unsupportedmsg:
	.asciiz "UnsBIOS\n"

unsupportedreg:
	.asciiz "Uns.reg\n"

regionmsg:
	.asciiz "Reg:"

intfailmsg:
	.asciiz "Inv.INT\n"

replyfailmsg:
	.asciiz "Inv.reply\n"

stopfailmsg:
	.asciiz "Stop fail\n"

backdoormsg:
	.asciiz "Backdoored!\n"

waitdooropenmsg:
	.asciiz "Wait door open\n"

waitdoorclosemsg:
	.asciiz "Wait door close\n"

chdirfail:
	.asciiz "Chdir failed\n"

donemsg:
	.asciiz "Done!\n"

regsubcmd:
	.byte 0x22

p0:
	.asciiz ""

p1:
	.asciiz "Licensed by"

p2:
	.asciiz "Sony"

p3:
	.asciiz "Computer"

p4:
	.asciiz "Entertainment"

p5usa:
	.asciiz "of America"

p5eur:
	.asciiz "(Europe)"

systemcnf:
	.asciiz "cdrom:SYSTEM.CNF;1"

cdrompath:
	.asciiz "cdrom:"
