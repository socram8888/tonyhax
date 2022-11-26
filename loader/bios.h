
#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct exe_header exe_header_t;
typedef struct handler_info handler_info_t;

struct exe_header {
	uint32_t initial_pc; // 0x00
	uint32_t initial_gp; // 0x04
	uint8_t * load_addr; // 0x08
	uint32_t load_size; // 0x0C
	uint32_t _reserved0[2]; // 0x10
	uint32_t memfill_start; // 0x18
	uint32_t memfill_size; // 0x1C
	uint32_t initial_sp_base; // 0x20
	uint32_t initial_sp_offset; // 0x24
	uint8_t _reserved1[20];
};

struct handler_info {
	handler_info_t * next;
	void (*handler)(int);
	int (*verifier)(void);
	uint32_t dummy;
};

#define BIOS_DEFAULT_EVCB 0x10
#define BIOS_DEFAULT_TCB 0x4
#define BIOS_DEFAULT_STACKTOP 0x801FFF00

/*
 * EXTRAS.
 */

/**
 * BIOS author string.
 */
static const char * const BIOS_DEVELOPER = (const char *) 0xBFC0012C;

/**
 * BIOS version string. Not available on SCPH-1000 consoles.
 *
 * Example: "System ROM Version 4.1 12/16/97 E"
 */
static const char * const BIOS_VERSION = (const char *) 0xBFC7FF32;

/**
 * A0-table location.
 */
static void ** const BIOS_A0_TABLE = (void **) 0x200;

/**
 * Executes a full reset of the console's BIOS, as if a WarmBoot was issued.
 */
void bios_reinitialize(void);

/**
 * Returns true if the console is running an original PS1 BIOS.
 *
 * @returns true if console is a PS1.
 */
bool bios_is_ps1(void);

/**
 * Returns true if the console has an European BIOS.
 * @returns true if console is European
 */
bool bios_is_european(void);

/**
 * Copies the relocated kernel code to its destination (0x500).
 */
void bios_copy_relocated_kernel(void);

/**
 * Copies the A call table to its destination (0x200).
 */
void bios_copy_a0_table(void);

/**
 * Returns a pointer to syscall handler information structure.
 * @returns pointer to the mutable structure.
 */
handler_info_t * bios_get_syscall_handler(void);

/**
 * Replaces the disc error handler with one that logs the code to the screen and returns.
 */
void bios_inject_disc_error(void);

/**
 * Restores the disc error handler.
 */
void bios_restore_disc_error(void);

/**
 * Fake version of the EnqueueCdIntr call that we'll use to skip the CD reinitialization in the
 * SetConf call during the BIOS reinitialization.
 */
void FakeEnqueueCdIntr(void);

/*
 * SYSCALLS
 */

/**
 * Enter critical section.
 *
 * Disables interrupts so code executes atomically.
 */
void EnterCriticalSection();

/**
 * Exit critical section.
 *
 * Re-enables interrupts.
 */
void ExitCriticalSection();

/*
 * A-FUNCTIONS
 */

/**
 * File for read access.
 */
#define FILE_READ     0x00000001

/**
 * File for write access.
 */
#define FILE_WRITE    0x00000002

/**
 * When reading from TTY, returns without waiting to read requested amount of bytes.
 */
#define FILE_TTY_LAZY 0x00000004

/**
 * Create a new file.
 */
#define FILE_CREATE   0x00000200

/**
 * Read asynchronously from memory card.
 */
#define FILE_MC_ASYNC 0x00008000

/**
 * Specify size of file in blocks for memory card.
 */
#define FILE_SIZE(blocks) ((blocks) << 16)

/**
 * No error.
 */
#define FILEERR_OK 0x00

/**
 * File not found
 */
#define FILEERR_NOT_FOUND 0x02

/**
 * Bad device port number (tty2 and up)
 */
#define FILEERR_BAD_DEVICE 0x06

/**
 * Invalid or unused file handle
 */
#define FILEERR_UNUSED_HANDLE 0x09

/**
 * General error (physical I/O error, unformatted, disk changed for old fcb)
 */
#define FILEERR_GENERAL 0x10

/**
 * File already exists
 */
#define FILEERR_ALREADY_EXISTS 0x11

/**
 * Cross device rename
 */
#define FILEERR_CROSS_DEV_RENAME 0x12

/**
 * Unknown device name
 */
#define FILEERR_UNKNOWN_DEV 0x13

/**
 * Alignment error
 */
#define FILEERR_ALIGN 0x16

/**
 * Too many open files
 */
#define FILEERR_TOO_MANY_HANDLES 0x18

/**
 * Device full
 */
#define FILEERR_DEV_FULL 0x1C

/**
 * Converts a digit to its numeric value:
 *
 * - '0' to '9' returns 0 to 9
 * - 'A' to 'Z' returns 10 to 35
 * - 'a' to 'z' returns 10 to 35 as well
 *
 * For other values, this returns 9,999,999 in decimal.
 *
 * Table A, call 0x0A.
 *
 * @param c the character
 * @returns parsed digit
 */
uint32_t todigit(char c);

/**
 * Starts a previously loaded executable.
 *
 * @param headerbuf header buffer
 * @param param1 first argument sent to the executable
 * @param param2 second argument sent to the executable
 *
 * Table A, call 0x43.
 */
void __attribute__((noreturn)) DoExecute(const exe_header_t * header, uint32_t param1, uint32_t param2);

/**
 * Flushes the CPU cache. Should be called after modifying code in software.
 *
 * Table A, call 0x44.
 */
void FlushCache(void);

/**
 * Copies the three default four-opcode handlers for the A(NNh),B(NNh),C(NNh) functions to A00000A0h..A00000CFh.
 *
 * Table A, call 0x45.
 */
void init_a0_b0_c0_vectors(void);

/**
 * Copies the width*height 16-bit half words to Vram at x, y coords.
 *
 * width*height MUST be an even number, or else it'll lock up.
 *
 * @param x Vram coord X
 * @param y Vram coord Y
 * @param width texture width, in 16-bit pixels
 * @param height texture height, in lines
 * @param src texture data
 *
 * Table A, call 0x46.
 */
void GPU_dw(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint16_t * src);

/**
 * Copies the width*height 16-bit half words to Vram at x, y coords using DMA.
 *
 * width*height MUST be a multiple of 32, or else it'll lock up.
 *
 * @param x Vram coord X
 * @param y Vram coord Y
 * @param width texture width, in 16-bit pixels
 * @param height texture height, in lines
 * @param src texture data
 *
 * Table A, call 0x47.
 */
void gpu_send_dma(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint16_t * src);

/**
 * Executes the given GPU GP1 command.
 *
 * @param gp1cmd the command
 *
 * Table A, call 0x48.
 */
void SendGP1Command(uint32_t gp1cmd);

/**
 * Synchronizes with the GPU and executes the given GPU GP0 command.
 *
 * @param gp0cmd the command
 * @returns 0 if succeeded, or -1 otherwise.
 *
 * Table A, call 0x49.
 */
int GPU_cw(uint32_t gp0cmd);

/**
 * Synchronizes with the GPU and sends the buffer to the GPU.
 *
 * @param src command + parameter list
 * @param num number of words in list
 *
 * Table A, call 0x4A.
 */
void GPU_cwp(uint32_t * src, uint32_t num);

/**
 * Loads and executes the given file.
 *
 * @param filename executable path
 * @param stackbase stack pointer base
 * @param stackoffset stack pointer offset
 *
 * Table A, call 0x51.
 */
bool LoadAndExecute(const char * filename, uint32_t stack_base, uint32_t stack_offset);

/**
 * Initializes the CD drive and filesystem.
 *
 * @returns true if succeeded, else false
 *
 * Table A, call 0x54.
 */
bool CdInit(void);

/**
 * (Re-)initializes kernel resources.
 *
 * @param evcb max number of events
 * @param tcb max number of threads
 * @param stacktop stack top
 *
 * Table A, call 0x9C.
 */
void SetConf(uint32_t evcb, uint32_t tcb, uint32_t stacktop);

/**
 * Reads the requested amount of sectors from the CD.
 *
 * @param sector_count sector count
 * @param start_sector first sector to read
 * @param buffer destination buffer
 * @returns the amount of sectors read, or -1 on error
 *
 * Table A, call 0xA5.
 */
int32_t CdReadSector(uint32_t sector_count, uint32_t start_sector, void * buffer);

/*
 * B-FUNCTIONS
 */

/**
 * Restores the default exception exit handler.
 *
 * Table B, call 0x18.
 */
void SetDefaultExitFromException(void);

/**
 * Opens a file on the target device for I/O.
 *
 * Table B, call 0x32.
 *
 * @param path file path
 * @param access access flags
 * @returns file handle, or -1 on error.
 */
int32_t FileOpen(const char * filename, uint32_t accessmode);

/**
 * Reads the number of bytes from the specified open file.
 *
 * Must be a multiple of 128 bytes for memory card, and 2048 for CD-ROM.
 *
 * Table B, call 0x34.
 *
 * @param fd file handle
 * @param dst data buffer
 * @param length max data length
 * @returns number of bytes read
 */
int32_t FileRead(int32_t fd, void * dst, uint32_t length);

/**
 * Closes an open file.
 *
 * Table B, call 0x36.
 *
 * @param fd file handle
 */
void FileClose(int32_t fd);

/**
 * Returns the error code for the last failed file operation.
 *
 * @returns the error code for the last failed file operation.
 *
 * Table B, call 0x54.
 */
uint32_t GetLastError(void);

/**
 * Returns the address of the C call table.
 *
 * @returns the address of the C call table.
 *
 * Table B, call 0x56.
 */
void ** GetC0Table(void);

/**
 * Returns the address of the B call table.
 *
 * @returns the address of the B call table.
 *
 * Table B, call 0x57.
 */
void ** GetB0Table(void);

/*
 * C-FUNCTIONS
 */

/**
 * Copies the default four-opcode exception handler to the exception vector at 0x80000080h~0x8000008F.
 *
 * Table C, call 0x07.
 */
void InstallExceptionHandlers(void);

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
void AdjustA0Table(void);
