#include "hardware.h"
#include "memory.h"
#include "interrupts.h"
#include "utils.h"

static void update_DIV_register(clock_cycles_t clocks);
static void update_TIMA_register(clock_cycles_t clocks);

void update_timer(clock_cycles_t clocks) {
    update_DIV_register(clocks);
    update_TIMA_register(clocks);
}

void update_TIMA_register(clock_cycles_t clocks) {
    static uint16_t TIMA_progress = 0;
    uint8_t TAC_register = get_memory_byte(TAC);
    uint8_t TAC_enabled = get_bit(TAC_register, 2);

    if (!TAC_enabled) {
        return;
    }

    TIMA_progress += clocks;

    uint16_t TIMA_clock_rate = 256;
    uint8_t clock_select = TAC_register & 0x03;

    switch (clock_select) {
        case 0x00: TIMA_clock_rate = 256; break;
        case 0x01: TIMA_clock_rate = 4; break;
        case 0x02: TIMA_clock_rate = 16; break;
        case 0x03: TIMA_clock_rate = 64; break;
        default: break;
    }

    while (TIMA_progress > TIMA_clock_rate) {
        TIMA_progress -= TIMA_clock_rate;
        TIMA_progress = TIMA_progress % TIMA_clock_rate;
        uint16_t TIMA_value = privileged_get_memory_byte(TIMA) + 1;
        if (TIMA_value >= UINT8_MAX) {
            uint8_t TMA_val = privileged_get_memory_byte(TMA);
            TIMA_value = TMA_val;
            set_interrupts_flag(TIMER);
        }
        privileged_set_memory_byte(TIMA, (uint8_t)TIMA_value);
    }

}
void update_DIV_register(clock_cycles_t clocks) {
    static uint16_t DIV_progress = 0;
    DIV_progress += clocks;
    privileged_set_memory_byte(DIV, DIV_progress >> 8);
}
