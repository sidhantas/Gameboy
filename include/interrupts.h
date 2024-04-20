#include "instructions.h"
typedef enum Interrupts {
    NO_INTERRUPT = -1,
    VBLANK,
    LCD,
    TIMER,
    SERIAL,
    JOYPAD
} interrupts_t;

clock_cycles_t handle_interrupts(void);
void close_interrupt_handler(void);
void set_interrupts_flag(interrupts_t interrupt);
#define NUM_OF_INTERRUPTS 5

