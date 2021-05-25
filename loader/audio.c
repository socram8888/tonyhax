
#include "audio.h"
#include "io.h"

#define SPU_MAIN_VOL_LEFT _REG16(0x1F801D80)
#define SPU_MAIN_VOL_RIGHT _REG16(0x1F801D82)

#define SPU_REVERB_VOL_LEFT _REG16(0x1F801D84)
#define SPU_REVERB_VOL_RIGHT _REG16(0x1F801D86)

struct voice_t {
	uint32_t volume;
	uint16_t sample_rate;
	uint16_t start_address;
	uint32_t adsr_settings;
	uint16_t adsr_current_volume;
	uint16_t adsr_repeat_address;
};

struct voice_t * const SPU_VOICES = (struct voice_t *) 0x1F801C00;
#define SPU_VOICE_COUNT 24

void audio_halt(void) {
	// Mute SPU
	SPU_MAIN_VOL_LEFT = 0;
	SPU_MAIN_VOL_RIGHT = 0;
	SPU_REVERB_VOL_LEFT = 0;
	SPU_REVERB_VOL_RIGHT = 0;

	// Halt the playback of every voice
	for (int i = 0; i < SPU_VOICE_COUNT; i++) {
		SPU_VOICES[i].volume = 0;
		SPU_VOICES[i].sample_rate = 0;
	}
}
