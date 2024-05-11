#include "hardware.h"
#include "memory.h"
#include "interrupts.h"
#include "utils.h"

static uint8_t stat_line = 0;

void trigger_stat_source(stat_interrupts_t stat_source) {
    uint8_t stat_register = get_memory_byte(STAT);
    if (!(stat_register & 1 << stat_source)) {
        return;
    }

    if (!stat_line) {
        set_interrupts_flag(LCD);
    }
    stat_line |= 1 << stat_source;
    set_bit(&stat_line, (uint8_t)stat_source);
}


void clear_stat_source(stat_interrupts_t stat_source) {
    uint8_t stat_register = get_memory_byte(STAT);
    if (!(stat_register & 1 << stat_source)) {
        return;
    }
    reset_bit(&stat_line, (uint8_t)stat_source);
}

