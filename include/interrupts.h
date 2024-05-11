#pragma once
#include "hardware.h"
typedef enum Interrupts {
    NO_INTERRUPT = -1,
    VBLANK,
    LCD,
    TIMER,
    SERIAL,
    JOYPAD
} interrupts_t;

#define NUM_OF_INTERRUPTS 5

typedef enum {
    LYC_INT = 2,
    MODE_2_INT,
    MODE_1_INT,
    MODE_0_INT,
    INVALID_STAT_SOURCE
} stat_interrupts_t;

clock_cycles_t handle_interrupts(void);
void close_interrupt_handler(void);
void set_interrupts_flag(interrupts_t interrupt);


void trigger_stat_source(stat_interrupts_t stat_source);
void clear_stat_source(stat_interrupts_t stat_source);

