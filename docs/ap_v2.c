
inline uint8_t bcd_to_binary(uint8_t val) {
	return (val >> 4) * 10 + (val & 0xF);
}

inline uint8_t binary_to_bcd(uint8_t val) {
	return (val / 10) << 4 | (val % 10);
}

enum {
	CD_RET_TRAY_OPEN = -2,
	CD_RET_FAIL = -1,
	CD_RET_CONTINUE = 0,
	CD_RET_SUCCESS = 1
};

volatile uint8_t * const CD_REGS = (volatile uint8_t *) 0x1F801800;

struct cd_operation_t {
	uint8_t command;
	uint8_t req_len;
	uint8_t reply_len;
	uint8_t expected_ints;
};

enum {
	CD_OP_GET_STATUS = 0,
	CD_OP_GET_TN = 1,
	CD_OP_GET_TD = 2,
	CD_OP_SET_LOCATION = 3,
	CD_OP_SEEK_AUDIO = 4,
	CD_OP_SET_MODE = 5,
	CD_OP_INIT = 6,
	CD_OP_MUTE = 7,
	CD_OP_PLAY = 8,
	CD_OP_SUBFUNC_X = 9,
	CD_OP_SUBFUNC_Y = 10,
	CD_OP_PAUSE = 11,
	CD_OP_READ_TOC = 12,
	CD_OP_GET_ID = 13
};

const struct cd_operation_t[] CD_OPERATIONS = {
	{ 0x01, 0x00, 0x01, 0x03 }, // CD_OP_GETSTAT
	{ 0x13, 0x00, 0x03, 0x03 }, // CD_OP_GET_TN
	{ 0x14, 0x01, 0x03, 0x03 }, // CD_OP_GET_TD
	{ 0x02, 0x03, 0x01, 0x03 }, // CD_OP_SET_LOCATION
	{ 0x16, 0x00, 0x01, 0x05 }, // CD_OP_SEEK_AUDIO
	{ 0x0E, 0x01, 0x01, 0x03 }, // CD_OP_SET_MODE
	{ 0x0A, 0x00, 0x01, 0x05 }, // CD_OP_INIT
	{ 0x0B, 0x00, 0x01, 0x03 }, // CD_OP_MUTE
	{ 0x03, 0x00, 0x01, 0x03 }, // CD_OP_PLAY
	{ 0x19, 0x01, 0x01, 0x03 }, // CD_OP_SUBFUNC_X
	{ 0x19, 0x01, 0x02, 0x03 }, // CD_OP_SUBFUNC_Y
	{ 0x09, 0x00, 0x01, 0x05 }, // CD_OP_PAUSE
	{ 0x1E, 0x00, 0x01, 0x05 }, // CD_OP_READ_TOC
	{ 0x1A, 0x00, 0x01, 0x05 }  // CD_OP_GET_ID
};

uint8_t cd_req_buffer[3];
uint32_t cd_ackd_ints = 0;

void cd_start_op(int op) {
	// Clear all interrupts
	CD_REGS[0] = 0x01;
	CD_REGS[3] = 0x07;

	// Waste some cycles
	for (int i = 0; i < 4; i++) {
		*((uint32_t *) 0) = i;
	}

	// Disable interrupts for responses so we can check them synchronously
	CD_REGS[0] = 0x01;
	CD_REGS[2] = 0x18;

	// Write the request bytes (if any)
	CD_REGS[0] = 0x00;
	for (int i = 0; i < CD_OPERATIONS[op].req_len; i++) {
		CD_REGS[2] = cd_req_buffer[i];
	}

	// Start the operation
	CD_REGS[0] = 0x00;
	CD_REGS[1] = CD_OPERATIONS[op].command;
}

int cd_check_op(int op) {
	// Check current interrupts
	CD_REGS[0] = 0x01;
	int curints  = CD_REGS[3] & 7;
	int curints2 = CD_REGS[3] & 7;
	if (curints != curints2 || curints == 0) {
		return 0;
	}

	// Shouldn't this be an OR?
	cd_ackd_ints += curlen;

	// Acknowledge all of them
	CD_REGS[0] = 0x01;
	CD_REGS[3] = 0x07;

	// Waste some cycles
	for (i = 0; i < 4; i++) {
		*((uint32_t *) 0) = i;
	}

	// This is really bizarre, why a less than?
	if (cd_ackd_ints < CD_OPERATIONS[op].expected_ints) {
		return 0;
	}
	cd_ackd_ints = 0;

	// An INT5 means a fail
	if (curints == 5) {
		// Read fail reason
		cd_reply_buf[0] = CD_REGS[1];
		cd_reply_buf[1] = CD_REGS[1];
		
		// Restore all interrupts
		CD_REGS[0] = 0x01;
		CD_REGS[2] = 0x1F;

		// Check the status byte, and if the lid is open then fail
		if (cd_reply_buf[0] & 0x10) {
			// Failed because of an open lid
			return CD_RET_TRAY_OPEN;
		}

		// Unknown 
		return CD_RET_FAIL;
	} else {
		// Read the reply
		for (int i = 0; i < CD_OPERATIONS[op].reply_len; i++) {
			cd_reply_buf[i] = CD_REGS[1];
		}

		// Restore all interrupts
		CD_REGS[0] = 0x01;
		CD_REGS[2] = 0x1F;

		// Check the status byte
		if (op != 0x0A && (cd_reply_buf[0] & 0x10) != 0) {
			// Failed because of an open lid
			return CD_RET_TRAY_OPEN;
		}

		// Succeeded
		return CD_RET_SUCCESS;
	}
}

int ap_current_step;
int ap_track_count;
uint8_t ap_center_mm, ap_center_ss;

void antipiracy_main(int blocking) {
	int retval = 0;
	int mm, ss, center;

	do {
		switch (ap_current_step) {
			case 0:
				retval = 0;
				break;

			case 1:
				cd_start_op(CD_OP_GET_TN);
				ap_cur_step = 2;
				retval = 1;
				break;

			case 2:
				switch (cd_read_reply(CD_OP_GET_TN)) {
					case CD_RET_FAIL:
						cd_start_op(CD_OP_GET_TN);
						break;

					case CD_RET_TRAY_OPEN:
						cd_start_op(CD_OP_GET_STATUS);
						ap_cur_step = 17;
						break;

					case CD_RET_SUCCESS:
						ap_track_count = cd_reply_buf[2];
						cd_start_op(CD_OP_INIT);
						ap_cur_step = 3;
						break;
				}

				retval = 2;
				break;

			case 3:
				switch (cd_read_reply(CD_OP_INIT)) {
					case CD_RET_FAIL:
						ap_cur_step = 1;
						break;

					case CD_RET_TRAY_OPEN:
						cd_start_op(CD_OP_GET_STATUS);
						ap_cur_step = 17;
						break;

					case CD_RET_SUCCESS:
						// Request end of first track
						cd_req_buffer[0] = ap_track_count < 2 ? 0 : 2;
						cd_start_op(CD_OP_GET_TD);
						ap_cur_step = 4;
						break;
				}

				retval = 3;
				break;

			case 4:
				switch (cd_read_reply(CD_OP_GET_TD)) {
					case CD_RET_FAIL:
						ap_cur_step = 1;
						break;

					case CD_RET_TRAY_OPEN:
						cd_start_op(CD_OP_GET_STATUS);
						ap_cur_step = 17;
						break;

					case CD_RET_SUCCESS:
						// Calculate center of data track
						mm = bcd_to_binary(cd_reply_buf[1]);
						ss = bcd_to_binary(cd_reply_buf[2]);
						center = (60 * mm + ss) / 2;

						// Convert back into minutes and seconds
						ap_center_mm = binary_to_bcd(center / 60);
						ap_center_ss = binary_to_bcd(center % 60);

						cd_start_op(CD_OP_READ_TOC);
						ap_cur_step = 5;
						break;
				}

				retval = 7;
				break;

			case 5:
				retval = cd_read_reply(CD_OP_READ_TOC);

				switch (retval) {
					case CD_RET_FAIL:
						if ((cd_reply_buf[0] & 1) == 0) {
							ap_cur_step = 1;
						} else if ((cd_reply_buf[1] & 0x40) != 0) {
							cd_req_buffer[0] = ap_center_mm;
							cd_req_buffer[1] = ap_center_ss;
							cd_req_buffer[2] = 0;
							cd_start_op(CD_OP_SET_LOCATION);
							ap_cur_step = 7;
							retval = 5;
						}
						break;

					case CD_RET_TRAY_OPEN:
						cd_start_op(CD_OP_GET_STATUS);
						ap_cur_step = 17;
						retval = 5;
						break;

					case CD_RET_SUCCESS:
						cd_start_op(CD_OP_GET_ID);
						ap_cur_step = 7;
						retval = 5;
						break;

					default:
						retval = 5;
				}
				break;

			case 6:
				retval = cd_read_reply(CD_OP_GET_ID);

				switch (retval) {
					case CD_RET_FAIL:
						antipiracy_triggered();
						break;

					case CD_RET_TRAY_OPEN:
						cd_start_op(CD_OP_GET_STATUS);
						ap_cur_step = 17;
						retval = 6;
						break;

					case CD_RET_SUCCESS:
						cd_req_buffer[0] = ap_center_mm;
						cd_req_buffer[1] = ap_center_ss;
						cd_req_buffer[2] = 0;
						cd_start_op(CD_OP_SET_LOCATION);
						ap_cur_step = 7;
						retval = 6;
						break;

					default:
						retval = 6;
				}
				break;

			case 7:
				retval = cd_read_reply(CD_OP_SET_LOCATION);

				switch (retval) {
					case CD_RET_FAIL:
						ap_cur_step = 1;
						break;

					case CD_RET_TRAY_OPEN:
						cd_start_op(CD_OP_GET_STATUS);
						ap_cur_step = 17;
						retval = 7;
						break;

					case CD_RET_SUCCESS:
						cd_req_buf[0] = 1;
						cd_start_op(CD_OP_SET_MODE);
						ap_cur_step = 8;
						retval = 7;
						break;

					default:
						retval = 7;
				}
				break;

			case 8:
				retval = cd_read_reply(CD_OP_SET_MODE);

				switch (retval) {
					case CD_RET_FAIL:
						ap_cur_step = 1;
						break;

					case CD_RET_TRAY_OPEN:
						cd_start_op(CD_OP_GET_STATUS);
						ap_cur_step = 17;
						retval = 8;
						break;

					case CD_RET_SUCCESS:
						retval = 8;
						dword_800738F8 = VSync(-1);
						ap_cur_step = 9;
						break;

					default:
						retval = 8;
						break;
				}
				break;

			case 9:
				if (VSync(-1) - dword_800738F8 >= 3) {
					cd_send_cmd(CD_OP_SEEK_AUDIO);
					ap_cur_step = 10;
				}
				retval = 9;
				break;

			case 10:
			
	} while (blocking && retval != 0);

	return retval;
}
