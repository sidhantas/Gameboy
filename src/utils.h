#pragma once
#include "hardware.h"
#include <ncurses.h>
#include <stdint.h>
#define DMG_SIZE 256
#define SECTOR_SIZE 4096
#define KRED "\x1B[31m"
#define RESET "\033[0m"
void check_alloc(void *allocated_block, const char *msg_on_failure);
static inline uint16_t two_u8s_to_u16(uint8_t byte_0, uint8_t byte_1) {
    return byte_1 << 8 | byte_0;
}

void mvwprintwhcenter(WINDOW *win, uint8_t row, uint8_t row_start,
                      uint8_t width, const char *str, ...);

uint16_t post_inc(uint16_t *val);

bool half_carry_on_subtract(uint8_t val_1, uint8_t val_2);
uint8_t sub(uint8_t val_1, uint8_t val_2);
uint8_t add(uint8_t val_1, uint8_t val_2, uint8_t carry);
uint16_t addu16(long_reg_t r1, long_reg_t r2);
uint8_t get_crumb(uint8_t byte, uint8_t crumb);
uint8_t get_bit(uint8_t byte, uint8_t bit);
void set_bit(uint8_t *byte, uint8_t bit);
struct timeval time_diff(struct timeval start, struct timeval end);
int8_t uint8_to_int8(uint8_t n);
void u16_to_two_u8s(uint16_t val, uint8_t *b1, uint8_t *b2);
