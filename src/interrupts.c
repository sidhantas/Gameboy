#include "interrupts.h"
#include "cpu.h"
#include "hardware.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

interrupts_t get_highest_priority_interrupt(void);
void reset_interrupt_flag(interrupts_t interrupt);
uint16_t get_interrupt_handler(interrupts_t interrupt);

void set_interrupts_flag(interrupts_t interrupt) {
    set_memory_byte(IF, get_memory_byte(IF) | (1 << interrupt));
}

clock_cycles_t handle_interrupts(void) {
    //enum INTERRUPT_STATE interrupt_state = get_interrupt_state();
    //if (interrupt_state == ENABLE) {
    //    set_ime_flag(1);
    //    return 0;
    //} else if (interrupt_state == DISABLE) {
    //    set_ime_flag(0);
    //}
    if (!get_ime_flag() || !(get_memory_byte(IE) & get_memory_byte(IF))) {
        return 0;
    }
    interrupts_t highest_priority_interrupt = get_highest_priority_interrupt();
    if (highest_priority_interrupt >= 0) {
        reset_interrupt_flag(highest_priority_interrupt);
        set_ime_flag(0);
        stack_push_u16(get_pc());
        set_pc(get_interrupt_handler(highest_priority_interrupt));
    }
    return TWENTY_CLOCKS;
}

interrupts_t get_highest_priority_interrupt(void) {
    uint8_t available_interrupts = get_memory_byte(IE) & get_memory_byte(IF);
    for (uint8_t i = 0; i < NUM_OF_INTERRUPTS; i++) {
        if (available_interrupts & (1 << i)) {
            return i;
        }
    }
    return NO_INTERRUPT;
}

void reset_interrupt_flag(interrupts_t interrupt) {
    uint8_t current_interrupt_flags = get_memory_byte(IF);
    set_memory_byte(IF, current_interrupt_flags & ~(1 << interrupt));
}

uint16_t get_interrupt_handler(interrupts_t interrupt) {
    switch (interrupt) {
        case VBLANK: return 0x40;
        case LCD: return 0x48;
        case TIMER: return 0x50;
        case SERIAL: return 0x58;
        case JOYPAD: return 0x60;
        default: exit(1);
    }
}
