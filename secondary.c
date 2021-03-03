
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "bios.h"
#include "cdrom.h"
#include "gpu.h"

#define BGCOLOR 0x00C0FF

// Set to zero unless you are using an emulator or have a physical UART on the PS1, else it'll freeze
static const uint32_t tty_enabled = 1;

static uint8_t cd_reply[16];

// Buffer right before this executable
static uint8_t * data_buffer = (uint8_t *) 0x801FB800;

void reinit_kernel() {
	// Disable interrupts
	EnterCriticalSection();

	// The following is adapted from the WarmBoot call

	// Restore A, B and C tables
	init_a0_b0_c0_vectors();

	// Fix A table
	AdjustA0Table();

	// Install default exception handlers
	InstallExceptionHandlers();

	// Setup devices
	InstallDevices(tty_enabled);

	// Initialize kernel memory
	SysInitMemory(0xA000E000, 0x2000);

	// Enqueue syscall handler with priority 0
	EnqueueSyscallHandler(0);

	// Initialize default interrupt
	InitDefInt(3);

	// Setup timer and Vblank with priority 1
	EnqueueTimerAndVblankIrqs(1);

	// End of code adapted

	// Re-enable interrupts
	ExitCriticalSection();
}

bool backdoor_cmd(uint_fast8_t cmd, const char * string) {
	// Send command
	cd_command(cmd, (const uint8_t *) string, strlen(string));

	// Check if INT5, else fail
	uint_fast8_t interrupt = cd_wait_int();
	if (cd_wait_int() != 5) {
		printf("Invalid INT %X\n", interrupt);
		return false;
	}

	// Check length
	uint_fast8_t reply_len = cd_read_reply(cd_reply);
	if (reply_len != 2) {
		printf("Invalid len of %d\n", reply_len);
		return false;
	}

	// Check there is an error flagged
	if (!(cd_reply[0] & 0x01)) {
		std_out_puts("Invalid reply\n");
		return false;
	}

	// Check error code
	if (cd_reply[1] != 0x40) {
		std_out_puts("Invalid reply\n");
		return false;
	}

	return true;
}

bool unlock_drive() {

	// Run "GetRegion" test
	uint8_t test = CD_TEST_REGION;
	cd_command(CD_CMD_TEST, &test, 1);

	// Should succeed with 3
	if (cd_wait_int() != 3) {
		std_out_puts("Failed to read region\n");
		return false;
	}

	// Read actual region text
	// No need to bother adding the null terminator as the buffer at this point is all zeros
	cd_read_reply(cd_reply);

	std_out_puts("Region: \"");
	// +4 to skip the "for " at the beginning of the reply
	std_out_puts((char *) (cd_reply + 4));
	std_out_puts("\"\n");

	// Compare which is the fifth string we have to send to the backdoor
	const char * p5_localized;
	if (strcmp((char *) cd_reply, "for Europe") == 0) {
		p5_localized = "(Europe)";
	} else if (strcmp((char *) cd_reply, "for U/C") == 0) {
		p5_localized = "of America";
	} else {
		std_out_puts("Unsupported region\n");
		return false;
	}

	// Note the kernel's implementation of strlen returns 0 for nulls.
	if (
			!backdoor_cmd(0x50, NULL) ||
			!backdoor_cmd(0x51, "Licensed by") ||
			!backdoor_cmd(0x52, "Sony") ||
			!backdoor_cmd(0x53, "Computer") ||
			!backdoor_cmd(0x54, "Entertainment") ||
			!backdoor_cmd(0x55, p5_localized) ||
			!backdoor_cmd(0x56, NULL)
	) {
		std_out_puts("Backdoor failed\n");
		return false;
	}

	return true;
}

void wait_door_status(bool open) {
	printf("Wait door %s\n", open ? "open" : "close");

	uint8_t expected = open ? 0x10 : 0x00;
	do {
		// Issue Getstat command
		// We cannot issue the BIOS CD commands yet because we haven't called CdInit
		cd_command(CD_CMD_GETSTAT, NULL, 0);

		// Always returns 3, no need to check
		cd_wait_int();

		// Always returns one, no need to check either
		cd_read_reply(cd_reply);

	} while ((cd_reply[0] & 0x10) != expected);
}

bool config_get_hex(const char * config, const char * wanted, uint32_t * value) {
	uint32_t wanted_len = strlen(wanted);

	while (true) {
		// Check if first N characters match
		if (strncmp(config, wanted, wanted_len) == 0) {
			// Perfect, we're on the right line. Advance.
			config += wanted_len;

			// Keep parsing until we hit the end of line or the end of file
			uint32_t parsed = 0;
			while (*config != '\n' && *config != '\0') {
				uint32_t digit = todigit(*config);
				if (digit < 0x10) {
					parsed = parsed << 4 | digit;
				}
				config++;
			}

			// Log
			printf("%s = %x\n", wanted, parsed);
			*value = parsed;
			return true;

		} else {
			// No luck. Advance until next line.
			config = strchr(config, '\n');
			
			// If this is the last line, abort.
			if (config == NULL) {
				printf("Missing %s\n", wanted);
				return false;
			}

			// Advance to skip line feed.
			config++;
		}
	}
}

bool config_get_string(const char * config, const char * wanted, char * value) {
	uint32_t wanted_len = strlen(wanted);

	while (true) {
		// Check if first N characters match
		if (strncmp(config, wanted, wanted_len) == 0) {
			// Perfect, we're on the right line. Advance.
			config += wanted_len;

			// Advance until the start
			while (1) {
				// Skip spaces and equals
				if (*config == ' ' || *config == '=') {
					config++;
				} else if (*config == '\0' || *config == '\n' || *config == '\r') {
					printf("Corrupted %s\n", wanted);
					return false;
				} else {
					break;
				}
			}

			// Copy until space or end of file
			char * valuecur = value;
			while (*config != '\0' && *config != '\n' && *config != '\r' && *config != ' ') {
				*valuecur = *config;
				config++;
				valuecur++;
			}

			// Null terminate
			*valuecur = '\0';

			// Log
			printf("%s = %s\n", wanted, value);
			return true;

		} else {
			// No luck. Advance until next line.
			config = strchr(config, '\n');
			
			// If this is the last line, abort.
			if (config == NULL) {
				printf("Missing %s\n", wanted);
				return false;
			}

			// Advance to skip line feed.
			config++;
		}
	}
}

void try_boot_cd() {
	wait_door_status(true);

	wait_door_status(false);

	std_out_puts("Initializing CD... ");
	CdInit();
	std_out_puts("success\n");

	std_out_puts("Loading system config... ");
	int32_t fd = FileOpen("cdrom:SYSTEM.CNF;1", FILE_READ);
	if (fd == -1) {
		std_out_puts("open error\n");
		return;
	}

	int32_t read = FileRead(fd, data_buffer, 2048);
	FileClose(fd);

	if (read == -1) {
		std_out_puts("read error\n");
		return;
	}

	printf("read %d bytes\n", read);

	// Null terminate
	data_buffer[read] = '\0';

	uint32_t tcb, event, stacktop;
	char bootfile[32];
	if (
			!config_get_hex((char *) data_buffer, "TCB", &tcb) ||
			!config_get_hex((char *) data_buffer, "EVENT", &event) ||
			!config_get_hex((char *) data_buffer, "STACK", &stacktop) ||
			!config_get_string((char *) data_buffer, "BOOT", bootfile)
	) {
		return;
	}

	std_out_puts("Configuring kernel... ");
	SetConf(event, tcb, stacktop);
	std_out_puts("success\n");

	std_out_puts("Loading executable... ");
	LoadExeFile(bootfile, data_buffer);
	std_out_puts("success\n");

	std_out_puts("Starting!\n");
	DoExecute(data_buffer, 0, 0);
}

void loadfont() {
	// Font is 1bpp. We have to convert each character to 4bpp.
	const uint8_t * rom_charset = (const uint8_t *) 0xBFC7F8DE;
	uint16_t * short_buffer = (uint16_t *) data_buffer;

	// Iterate through the 48x2 character table
	for (uint_fast8_t tabley = 0; tabley < 2; tabley++) {
		for (uint_fast8_t tablex = 0; tablex < 48; tablex++) {
			uint16_t * bufferpos = short_buffer;

			// Iterate through each line of the 8x15 character
			for (uint_fast8_t chary = 0; chary < 15; chary++) {
				uint_fast8_t char1bpp = *rom_charset;
				rom_charset++;

				// Iterate through each column of the character
				for (uint_fast8_t bpos = 0; bpos < 8; bpos += 4) {
					uint_fast16_t char4bpp = 0;

					if (char1bpp & 0x80) {
						char4bpp |= 0x1000;
					}
					if (char1bpp & 0x40) {
						char4bpp |= 0x0100;
					}
					if (char1bpp & 0x20) {
						char4bpp |= 0x0010;
					}
					if (char1bpp & 0x10) {
						char4bpp |= 0x0001;
					}

					*bufferpos = char4bpp;
					bufferpos++;
					char1bpp = char1bpp << 4;
				}
			}

			// At 4bpp, each character uses 8 * 4 / 16 = 2 shorts, so the texture width is set to 2.
			GPU_dw(512 + tablex * 2, tabley * 15, 2, 15, short_buffer);
		}
	}

	// Load CLUT
	for (int i = 0; i < 16; i++) {
		// Black
		short_buffer[i] = 0x0000;
	}

	// Make 1 white
	short_buffer[1] = 0x7FFF;

	// Load the palette to Vram
	GPU_dw(512, 30, 16, 1, short_buffer);
}

void main() {
	// Turn off screen so the user knows we've successfully started.
	gpu_display(false);

	// Tell the user we've successfully launched
	std_out_puts("=== SECONDARY PROGRAM LOADER ===\n");

	std_out_puts("Reinitializing kernel... ");

	// Undo all possible fuckeries during exploiting
	reinit_kernel();

	gpu_reset();

	// Clear entire VRAM
	gpu_fill_rectangle(0, 0, 1023, 511, 0x000000);

	// Enable display
	gpu_display(true);

	// Load font
	loadfont();

	// Set drawing area
	gpu_set_drawing_area(0, 0, 256, 240);

	// Configure Texpage
	// - Texture page to X=512 Y=0
	// - Colors to 4bpp
	// - Allow drawing to display area (fuck Vsync)
	GPU_cw(0xE1000408);

	// Draw text
	struct gpu_tex_rect rect;
	rect.x = 128;
	rect.y = 120;
	rect.width = 32;
	rect.height = 15;
	rect.clut_x = 512;
	rect.clut_y = 30;
	rect.tex_x = 0;
	rect.tex_y = 0;
	rect.semi_transp = 0;
	rect.raw_tex = 1;
	gpu_draw_tex_rect(&rect);

	std_out_puts("success\n");

	std_out_puts("Unlocking drive...\n");

	if (!unlock_drive()) {
		return;
	}

	std_out_puts("Unlocked!\n");

	while (1) {
		try_boot_cd();
	}
}

void __attribute__((section(".start"))) start() {
	main();
	while(1);
}
