#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void check_alloc(void *allocated_block, const char *msg_on_failure) {
    if (!allocated_block) {
        fprintf(stderr, "%s", msg_on_failure);
        exit(1);
    }
}

void load_dmg(FILE *rom) {
    uint16_t bytes_read =
        fread(&(hardware.memory[BOOT_ROM_BEGIN]), DMG_SIZE, 1, rom);
    if (bytes_read != 1) {
        fprintf(stderr, "Unable To Read DMG, %d\n", bytes_read);
        exit(1);
    }
}

void mvwprintwhcenter(WINDOW *win, uint8_t row, uint8_t row_start,
                      uint8_t width, const char *str, ...) {
    char formatted_string[UINT8_MAX];
    va_list args;
    va_start(args, str);
    vsnprintf(formatted_string, UINT8_MAX, str, args);
    uint16_t len = strnlen(formatted_string, UINT8_MAX);
    mvwprintw(win, row, row_start + (width / 2) - (len / 2), formatted_string);
}

uint16_t post_inc(uint16_t *val) { return (*val)++; }

void u16_to_two_u8s(uint16_t val, uint8_t *b1, uint8_t *b2) {
    *b2 = (val & 0xFF00) >> 8;
    *b1 = val & 0xFF;
}

void set_long_reg_u16(long_reg_t long_reg, uint16_t val) {
    uint8_t b1, b2;
    u16_to_two_u8s(val, &b1, &b2);
    set_long_reg(long_reg, b1, b2);
}

void set_long_mem(uint16_t address, uint16_t val) {
    uint8_t b1, b2;
    u16_to_two_u8s(val, &b1, &b2);
    hardware.memory[address] = b1;
    hardware.memory[address + 1] = b2;
}

void set_long_reg(long_reg_t long_reg, uint8_t b1, uint8_t b2) {
    switch (long_reg) {
        case BC:
            hardware.registers[B] = b2;
            hardware.registers[C] = b1;
            break;
        case DE:
            hardware.registers[D] = b2;
            hardware.registers[E] = b1;
            break;
        case HL:
            hardware.registers[H] = b2;
            hardware.registers[L] = b1;
            break;
        case SP: hardware.sp = two_u8s_to_u16(b1, b2); break;
        default: return;
    }
}

uint16_t get_long_reg(long_reg_t long_reg) {
    switch (long_reg) {
        case SP: return hardware.sp;
        case BC:
            return two_u8s_to_u16(hardware.registers[C], hardware.registers[B]);
        case DE:
            return two_u8s_to_u16(hardware.registers[E], hardware.registers[D]);
        case HL:
            return two_u8s_to_u16(hardware.registers[L], hardware.registers[H]);
        default: return 0;
    }
}

void load_rom(FILE *rom) {
    uint16_t sectors_read = 0;
    int16_t read_size;
    fseek(rom, ROM_START, SEEK_SET);
    do {
        read_size = fread(&hardware.memory[ROM_START + sectors_read * SECTOR_SIZE],
                 SECTOR_SIZE, 1, rom);
    }
    while (read_size == SECTOR_SIZE);
}

bool half_carry_on_subtract(uint8_t val_1, uint8_t val_2) {
   return (int)(val_1 & 0x0F) - (int)(val_2 & 0x0F) & 0x10; 
}

uint8_t sub(uint8_t val_1, uint8_t val_2) {
    uint8_t res = val_1 - val_2;
    half_carry_on_subtract(val_1, val_2) ? set_flag(H_FLAG) : reset_flag(H_FLAG);
    set_flag(N_FLAG);
    res > 0 ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(C_FLAG);
    return res;
}

bool half_carry_on_add(uint8_t val_1, uint8_t val_2) {
    return (((val_1 & 0xf) + (val_2 & 0xf)) & 0x10) == 0x10;
}

uint8_t add(uint8_t val_1, uint8_t val_2) {
    uint16_t res = val_1 + val_2;
    half_carry_on_add(val_1, val_2) ? set_flag(H_FLAG) : reset_flag(H_FLAG);
    set_flag(N_FLAG);
    if (res > 255) {
        set_flag(C_FLAG);
        res -= 256;
    } else {
        reset_flag(C_FLAG);
    }
    res ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    return res;
}
uint8_t get_crumb(uint8_t byte, uint8_t crumb) {
    return (byte >> ((3 - crumb) * 2)) & 0x03;
}
