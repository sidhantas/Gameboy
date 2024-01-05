#pragma once
#include "hardware.h"
#include <ncurses.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#define DMG_SIZE 256
#define SECTOR_SIZE 4096
#define KRED "\x1B[31m"
#define RESET "\033[0m"
void check_alloc(void *allocated_block, const char *msg_on_failure);
void load_dmg(FILE *);
void load_rom(FILE *);
static inline uint16_t two_u8s_to_u16(uint8_t byte_0, uint8_t byte_1) {
    return byte_1 << 8 | byte_0;
}

void mvwprintwhcenter(WINDOW *win, uint8_t row, uint8_t row_start,
                      uint8_t width, const char *str, ...);

uint16_t post_inc(uint16_t *val);

void set_long_reg(long_reg_t long_reg, uint8_t b1, uint8_t b2);
void set_long_mem(uint16_t address, uint16_t val);
void set_long_reg_u16(long_reg_t long_reg, uint16_t val);
uint16_t get_long_reg(long_reg_t long_reg);
bool half_carry_on_subtract(uint8_t val_1, uint8_t val_2);
uint8_t sub(uint8_t val_1, uint8_t val_2);
uint8_t add(uint8_t val_1, uint8_t val_2);
uint8_t get_crumb(uint8_t byte, uint8_t crumb);
struct timeval time_diff(struct timeval start, struct timeval end);
