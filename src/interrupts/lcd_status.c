#include "hardware.h"
#include "memory.h"
#include "interrupts.h"
#include "utils.h"

static uint8_t stat_line = 0;
static uint32_t serviced_stat_interrupts[4] = {0, 0, 0, 0};

void trigger_stat_source(stat_interrupts_t stat_source) {
    uint8_t stat_register = get_memory_byte(STAT);
    if (!(stat_register & (1 << stat_source))) {
        return;
    }

    if (!stat_line) {
        serviced_stat_interrupts[stat_source - 3]++;
        set_interrupts_flag(LCD);
    }
    set_bit(&stat_line, (uint8_t)stat_source);
}

void clear_stat_source(stat_interrupts_t stat_source) {
    reset_bit(&stat_line, (uint8_t)stat_source);
}

uint8_t get_stat_line(void) {
    return stat_line;
}

uint8_t update_stat_register(uint8_t byte) {
    //stat_line &= byte;
    return ((byte) & 0x78);
}

uint32_t *get_serviced_stat_interrupts(void) {
    return serviced_stat_interrupts;
}
