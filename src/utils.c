#include "utils.h"
#include "hardware.h"
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

void mvwprintwhcenter(WINDOW *win, int row, int row_start,
                      int width, const char *str, ...) {
    char formatted_string[UINT8_MAX];
    va_list args;
    va_start(args, str);
    vsnprintf(formatted_string, UINT8_MAX, str, args);
    int len = (int)strnlen(formatted_string, UINT8_MAX);
    mvwprintw(win, row, row_start + (width / 2) - (len / 2), formatted_string);
}

uint16_t post_inc(uint16_t *val) { return (*val)++; }

void u16_to_two_u8s(uint16_t val, uint8_t *b1, uint8_t *b2) {
    *b2 = (val >> 8) & 0xFF;
    *b1 = val & 0xFF;
}

bool half_carry_on_subtract(uint8_t val_1, uint8_t val_2, uint8_t carry) {
    return ((val_1 & 0xF) - (val_2 & 0xF) - (carry & 0xF) & 0x10);
}

uint8_t sub(uint8_t val_1, uint8_t val_2, uint8_t carry) {
    uint8_t res = val_1 - val_2 - carry;
    (val_2 + carry) > val_1 ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    half_carry_on_subtract(val_1, val_2, carry) ? set_flag(H_FLAG)
                                                : reset_flag(H_FLAG);
    set_flag(N_FLAG);
    res ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    return res;
}

bool half_carry_on_add(uint8_t val_1, uint8_t val_2, uint8_t carry) {
    return (((val_1 & 0xf) + (val_2 & 0xf) + (carry & 0xf)) & 0x10) == 0x10;
}

bool half_carry_on_add16(uint16_t val_1, uint16_t val_2) {
    return (((val_1 & 0xfff) + (val_2 & 0xfff)) & 0x1000) == 0x1000;
}

uint8_t add(uint8_t val_1, uint8_t val_2, uint8_t carry) {
    uint16_t res = val_1 + val_2 + carry;
    half_carry_on_add(val_1, val_2, carry) ? set_flag(H_FLAG)
                                           : reset_flag(H_FLAG);
    reset_flag(N_FLAG);
    if (res > UINT8_MAX) {
        set_flag(C_FLAG);
        res -= (UINT8_MAX + 1);
    } else {
        reset_flag(C_FLAG);
    }
    res ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    return (uint8_t)res;
}

uint16_t addu16(uint16_t val_1, uint16_t val_2) {
    uint32_t res = val_2 + val_1;
    reset_flag(N_FLAG);
    if (res > UINT16_MAX) {
        set_flag(C_FLAG);
        res -= (UINT16_MAX + 1);
    } else {
        reset_flag(C_FLAG);
    }
    half_carry_on_add16(val_2, val_1) ? set_flag(H_FLAG) : reset_flag(H_FLAG);
    return (uint16_t)res;
}

inline uint8_t get_crumb(uint8_t byte, uint8_t crumb) {
    return (byte >> (crumb * 2)) & 0x03;
}

inline uint8_t get_bit(uint8_t byte, uint8_t bit) {
    return (byte >> bit) & 0x01;
}

inline void set_bit(uint8_t *byte, uint8_t bit) { *byte |= (0x01 << bit); }
inline void reset_bit(uint8_t *byte, uint8_t bit) { *byte &= ~(0x01 << bit); }

struct timeval time_diff(struct timeval start, struct timeval end) {
    struct timeval diff;
    diff.tv_sec = end.tv_sec - start.tv_sec;
    if (end.tv_usec < start.tv_usec) {
        diff.tv_usec = 1000000 + end.tv_usec - start.tv_usec;
    } else {
        diff.tv_usec = end.tv_usec - start.tv_usec;
    }
    return diff;
}

int8_t uint8_to_int8(uint8_t n) { return *(int8_t *)&n; }
