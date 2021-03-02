.text
.align 4

# Execute CMD command, and waits for its execution
# a0 = command
# a1 = parameter pointer
# a2 = parameter length
#
# t0 = CDROM base address
# t1 = work register
# t2 = literal "1"
.globl cd_command
cd_command:
	# Load CDROM I/O port and "1" constant
	lui $t0, 0x1F80
	li $t2, 1

	# Set index to zero
	sb $zero, 0x1800($t0)

_cd_command_busy:
	# Wait for busy bit to go low
	lbu $t1, 0x1800($t0)
	and $t1, 0x80
	bne $t1, $zero, _cd_command_busy

	# Clear parameter FIFO
	li $t1, 0xC0
	sb $t1, 0x1803($t0)

_cd_command_copy_next:
	# If done, exit loop
	beq $a2, $zero, _cd_command_copy_done

	# Load and increment
	lbu $t1, 0($a1)
	add $a1, 1

	# Write to buffer
	sb $t1, 0x1802($t0)

	# Decrement byte count
	sub $a2, 1

	j _cd_command_copy_next

_cd_command_copy_done:
	# Go to index 1
	sb $t2, 0x1800($t0)

	# Disable interrupts as we'll poll
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
.globl cd_wait_int
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

_ret:
	jr $ra

# Reads reply from the CD
# a0 = data pointer
# Returns length on v0
#
# v0 = length
# t0 = CDROM base address
# t1 = response pointer
# t2 = work reg
.globl cd_read_reply
cd_read_reply:
	# Load CDROM I/O port
	lui $t0, 0x1F80

	# Go to index 1
	li $t1, 1
	sb $t1, 0x1800($t0)

	# Read reply now
	li $v0, 0

_cd_read_next:
	# If empty, finish
	lbu $t2, 0x1800($t0)
	and $t2, 0x20
	beq $t2, $zero, _ret

	# Read byte
	lbu $t2, 0x1801($t0)
	sb $t2, 0($a0)

	# Increment counter
	add $v0, $v0, 1
	add $a0, $a0, 1
	j _cd_read_next
