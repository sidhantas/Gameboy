#include "hardware.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initialize_hardware(Hardware *hardware) {
    hardware->memory = calloc(MEMORY_SIZE, sizeof(uint8_t));
    if (!(hardware->memory)) {
        fprintf(stderr, "Unable to allocate memory for memory\n");
    }
    hardware->display_buffer = calloc(DISPLAY_SIZE, sizeof(uint8_t));
    if (!(hardware->display_buffer)) {
        fprintf(stderr, "Unable to allocate memory for display buff\n");
    }
    memset(hardware->registers, 0, REGISTER_COUNT);
    hardware->is_double_speed = false;
    hardware->sp = 0;
    hardware->pc = 0;
}

inline uint8_t get_memory_byte(uint16_t address) {
    return hardware.memory[address];
}

inline uint8_t get_flag(flags_t flag) {
    const uint8_t FLAGS_REGISTER = hardware.registers[F];
    return (FLAGS_REGISTER >> (7 - flag)) & 1;
}

inline void set_flag(flags_t flag) {
    uint8_t *FLAGS_REGISTER = &hardware.registers[F];
    *FLAGS_REGISTER |= (1 << (7 - flag));
}

inline uint8_t reset_flag(flags_t flag) {
    uint8_t *FLAGS_REGISTER = &hardware.registers[F];
    *FLAGS_REGISTER &= ~(1 << (7 - flag));
}
