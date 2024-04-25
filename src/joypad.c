#include "hardware.h"
#include "interrupts.h"
#include "utils.h"

Joypad joy;

void initialize_io(void) {
    joy.inputs = 0xFF;
}

uint8_t get_joypad_state(void) { return joy.inputs; }

void set_joypad_state(joypad_t button) { 
    set_bit(&joy.inputs, button);
    uint8_t joypad_mem = privileged_get_memory_byte(JOYP);
    if (joypad_mem & 0x30) {
        set_interrupts_flag(JOYP);
    }
}

void reset_joypad_state(joypad_t button) { joy.inputs &= ~(1 << button); }
