
#pragma once
#include <stdint.h>

#define CD_CMD_TEST 0x19
#define CD_TEST_REGION 0x22

void cd_command(uint_fast8_t cmd, const void * params, uint_fast8_t params_len);
