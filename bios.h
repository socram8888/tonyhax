
#pragma once
#include <stdint.h>

/*
 * SYSCALLS
 */
void EnterCriticalSection();

void ExitCriticalSection();

/*
 * A-FUNCTIONS
 */

/**
 * Compares two strings.
 *
 * Table A, call 0x17.
 *
 * @param a first string
 * @param b second string
 * @returns zero if they are equal, or the difference between the first different byte.
 */
int32_t strcmp(const char * a, const char * b);

/**
 * Calculates the length of a string.
 *
 * Table A, call 0x1B.
 *
 * @param str string
 * @returns string length, or zero if null
 */
uint32_t strlen(const char * str);

/**
 * Write string to TTY.
 *
 * Table A, call 0x3E.
 *
 * @param txt text to display.
 */
void std_out_puts(const char * txt);

/**
 * Copies the three default four-opcode handlers for the A(NNh),B(NNh),C(NNh) functions to A00000A0h..A00000CFh.
 *
 * Table A, call 0x45.
 */
void init_a0_b0_c0_vectors();

/*
 * C-FUNCTIONS
 */

/**
 * Configures the Vblank and timer handlers.
 *
 * @param priority IRQ priority.
 *
 * Table C, call 0x00.
 */
void EnqueueTimerAndVblankIrqs(uint32_t priority);

/**
 * Copies the default four-opcode exception handler to the exception vector at 0x80000080h~0x8000008F.
 *
 * Table C, call 0x07.
 */
void InstallExceptionHandlers();

/**
 * Initializes the address and size of the allocatable Kernel Memory region.
 *
 * Table C, call 0x08.
 */
void SysInitMemory(uint32_t start_address, uint32_t size);

/**
 * Configures some IRQ handlers.
 *
 * @param priority IRQ priority.
 *
 * Table C, call 0x0C.
 */
void InitDefInt(uint32_t priority);

/**
 * Initializes the default device drivers for the TTY, CDROM and memory cards.
 *
 * The flags controls whether the TTY device should be a dummy (retail console) or an actual UART (dev console).
 * Note this will call will freeze if the UART is enabled but there is no such device.
 *
 * @param enable_tty 0 to use a dummy TTY, 1 to use a real UART.
 *
 * Table C, call 0x12.
 */
void InstallDevices(uint32_t enable_tty);

/**
 * Fixes the A call table, copying missing entries from the C table.
 *
 * Table C, call 0x1C.
 */
void AdjustA0Table();
