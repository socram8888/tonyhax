
#pragma once
#include <stdint.h>
#include <stdbool.h>

#define CD_CMD_GETSTAT 0x01
#define CD_CMD_INIT 0x0A
#define CD_CMD_TEST 0x19
#define CD_CMD_RESET 0x1C
#define CD_TEST_REGION 0x22
#define CD_CMD_SET_SESSION 0x12
#define CD_CMD_STOP 0x08
#define CD_CMD_GETID 0x1A
#define CD_CMD_SETMODE 0x0E
#define CD_CMD_GETTN 0x13
#define CD_CMD_GETTD 0x14
/**
 * Starts executing a CD command.
 *
 * @param cmd the command
 * @param params the param buffer, or NULL if length is zero
 * @param params_len parameter length in bytes
 */
void cd_command(uint_fast8_t cmd, const uint8_t * params, uint_fast8_t params_len);

/**
 * Waits for an interrupt to happen, and returns its code.
 *
 * @returns interrupt code
 */
uint_fast8_t cd_wait_int(void);

/**
 * Reads a reply from the controller after an interrupt has happened.
 *
 * @param reply reply buffer
 * @returns reply length
 */
uint_fast8_t cd_read_reply(uint8_t * reply_buffer);

/**
 * Reinitializes the CD drive.
 *
 * @returns true if succeded, or false otherwise.
 */
bool cd_drive_init(void);

// Resets the drive.
void cd_drive_reset();
