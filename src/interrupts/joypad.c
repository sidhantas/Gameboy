#include "hardware.h"
#include "interrupts.h"
#include "memory.h"
#include "utils.h"

Joypad joy;

void initialize_io(void) { joy.inputs = 0xFF; }

uint8_t get_joypad_state(void) { return joy.inputs; }

void set_joypad_state(joypad_t button) {
    set_bit(&joy.inputs, (uint8_t)button);
}

void reset_joypad_state(joypad_t button) {
    reset_bit(&joy.inputs, (uint8_t)button);
    uint8_t joypad_mem = privileged_get_memory_byte(JOYP);
    if (!(joypad_mem & 0x10 || joypad_mem & 0x20)) {
        set_interrupts_flag(JOYPAD);
    }
}
