#include "interrupts.h"
#include "cpu.h"
#include "hardware.h"
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>

interrupts_t get_highest_priority_interrupt(void);
void reset_interrupt_flag(interrupts_t interrupt);
uint16_t get_interrupt_handler(interrupts_t interrupt);

bool end_interrupt_handler = false;

void close_interrupt_handler(void) {
   end_interrupt_handler = true; 
}

void set_interrupts_flag(interrupts_t interrupt) {
    set_memory_byte(IF, get_memory_byte(IF) | (1 << interrupt));
}

void *initialize_interrupt_handler(void *arg) {
    (void)arg;
    while (true) {
        if (end_interrupt_handler) {
            break;
        }
        if (!get_ime_flag() || !(get_memory_byte(IE) & get_memory_byte(IF))) {
            continue;
        }
        pthread_mutex_lock(&cpu_mutex);
        interrupts_t highest_priority_interrupt = get_highest_priority_interrupt();
        if (highest_priority_interrupt >= 0 && highest_priority_interrupt == VBLANK) {
            reset_interrupt_flag(highest_priority_interrupt);
            set_interrupts(false);
            stack_push_u16(get_pc());
            set_pc(get_interrupt_handler(highest_priority_interrupt));
        }
        pthread_mutex_unlock(&cpu_mutex);
    }
    return NULL;
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
        case VBLANK:
            return 0x40;
        case LCD:
            return 0x48;
        case TIMER:
            return 0x50;
        case SERIAL:
            return 0x58;
        case JOYPAD:
            return 0x60;
        default:
            exit(1);
    }
}

void hanlde_interrupts(void) {
    // get Interrupt Enable register
    // for each possible interrupt
    //     check each interrupt based on priority
    //     if IF is true for interrupt
    //         set ime to false
    //         execute 2 nops
    //         push pc to stack
    //         set pc to address of handler
    //         continue execution
    //
}
