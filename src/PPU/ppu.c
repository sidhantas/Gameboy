#include "ppu.h"
#include "cpu.h"
#include "hardware.h"
#include "interrupts.h"
#include "memory.h"
#include "oam_queue.h"
#include "ppu_utils.h"
#include "utils.h"
#include <ncurses.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#define DOTS_PER_LINE 456

PPU ppu;

static bool close_ppu;
pthread_mutex_t dots_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t display_buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

static bool execute_mode_0(void);
static bool execute_mode_1(void);
static bool execute_mode_2(void);
static bool execute_mode_3(void);
static bool consume_dots(uint64_t dots_to_consume);
static void increase_scan_line(void);
static void set_ppu_mode(uint8_t mode);

void initialize_ppu(void) {
    close_ppu = false;
    ppu.line_dots = 0;
    ppu.mode = 2;
    ppu.ready_to_render = false;
    ppu.available_dots = 0;
    ppu.consumed_dots = 0;
    ppu.current_scan_line = 0;
    ppu.window_rendered = false;
    ppu.current_window_line = 0;
    ppu.line_x = 0;
}

void *start_ppu(void *arg) {
    (void)arg;
    render_loop();
    return NULL;
}

void render_loop(void) {
    while (close_ppu == false) {
        switch (ppu.mode) {
            case 0: execute_mode_0(); break;
            case 1: execute_mode_1(); break;
            case 2: execute_mode_2(); break;
            case 3: execute_mode_3(); break;
            default: exit(1);
        }
    }
}

bool (*get_current_mode_func(void))(void) {
    switch (ppu.mode) {
        case 0: return execute_mode_0; break;
        case 1: return execute_mode_1; break;
        case 2: return execute_mode_2; break;
        case 3: return execute_mode_3; break;
        default: exit(1);
    }
}

void run_ppu(uint16_t dots) {
    bool executed;
    if (!get_bit(privileged_get_memory_byte(LCDC), 7)) {
        return;
    }
    ppu.available_dots += dots;
    do {
        uint8_t lcd_status = privileged_get_memory_byte(STAT);
        if (privileged_get_memory_byte(LYC) == privileged_get_memory_byte(LCDY)) {
            set_bit(&lcd_status, 2);
            privileged_set_memory_byte(STAT, lcd_status);
            trigger_stat_source(LYC_INT);
        } else {
            reset_bit(&lcd_status, 2);
            privileged_set_memory_byte(STAT, lcd_status);
            clear_stat_source(LYC_INT);
        }
        bool (*current_mode_func)(void) = get_current_mode_func();
        executed = current_mode_func();
    } while (executed == true);
}

bool consume_dots(uint64_t dots_to_consume) {
    if (ppu.available_dots < dots_to_consume) {
        return false;
    }
    ppu.available_dots -= dots_to_consume;
    ppu.consumed_dots += dots_to_consume;
    ppu.line_dots += dots_to_consume;
    return true;
}

static bool window_enabled = false;
static bool execute_mode_2(void) {
    // wait for 2 dots
    static uint8_t object_index = 0;
    if (!consume_dots(2)) {
        return false;
    }
    add_sprite(object_index++);
    const uint8_t wy = privileged_get_memory_byte(WY);
    if (wy == ppu.current_scan_line) {
        window_enabled = true;
    }
    if (ppu.line_dots >= 80) {
        // Setup for Mode 3
        set_ppu_mode(3);
        object_index = 0;
        ppu.line_x = 0;
    }
    return true;
}

static bool execute_mode_3(void) {
    uint16_t penalty_dots = 0;
    if (ppu.line_x == 0) {
        penalty_dots += privileged_get_memory_byte(SCX) % 8;
    }
    if (ppu.line_x == privileged_get_memory_byte(WX) - 7) {
        penalty_dots += 6;
    }
    // wait an extra dot for the pixel;
    if (!consume_dots(penalty_dots + 1)) {
        return false;
    }
    uint8_t pixel = 0;
    if (get_bit(privileged_get_memory_byte(LCDC), 0)) {
        pixel = get_bg_pixel(ppu.line_x, ppu.current_scan_line);
        const uint8_t wx = privileged_get_memory_byte(WX);
        if (get_bit(privileged_get_memory_byte(LCDC), 5) && ppu.line_x >= (wx - 7) &&
            window_enabled) {
            pixel = get_win_pixel(ppu.line_x, ppu.current_window_line);
            ppu.window_rendered = true;
        }
    }

    const uint8_t object_pixel = get_obj_pixel(ppu.line_x);
    if (object_pixel != TRANSPARENT) {
        if (!get_bit(object_pixel, 7) || pixel == 0) {
            pixel = object_pixel & 0x03;
        }
    }

    pthread_mutex_lock(&display_buffer_mutex);
    set_display_pixel(ppu.line_x, ppu.current_scan_line,
                      get_color_from_byte(pixel));
    pthread_mutex_unlock(&display_buffer_mutex);

    if (++ppu.line_x >= DISPLAY_WIDTH) {
        set_ppu_mode(0);
    }
    return true;
}

static bool execute_mode_0(void) {
    if (!consume_dots(1)) {
        return false;
    }
    if (ppu.line_dots >= DOTS_PER_LINE) {
        increase_scan_line();
        if (ppu.current_scan_line >= DISPLAY_HEIGHT) {
            ppu.ready_to_render = true;
            set_interrupts_flag(VBLANK);
            set_ppu_mode(1);
        } else {
            set_ppu_mode(2);
            initialize_sprite_store();
        }
        ppu.line_dots %= DOTS_PER_LINE;
    }
    return true;
}

static bool execute_mode_1(void) {
    if (!consume_dots(1)) {
        return false;
    }
    if (ppu.line_dots >= 456) {
        increase_scan_line();
        ppu.line_dots %= 456;
        if (ppu.current_scan_line == 0) {
            ppu.current_window_line = 0;
            window_enabled = false;
            initialize_sprite_store();
            set_ppu_mode(2);
        }
    }
    return true;
}

static void increase_scan_line(void) {
    ppu.current_scan_line += 1;
    ppu.current_scan_line %= SCAN_LINES;
    privileged_set_memory_byte(LCDY, ppu.current_scan_line);
    if (ppu.window_rendered == true) {
        ppu.current_window_line++;
        ppu.window_rendered = false;
    }
    return;
}

stat_interrupts_t ppu_mode_to_stat_source(uint8_t mode) {
    switch (mode) {
        case 2: return MODE_2_INT;
        case 1: return MODE_1_INT;
        case 0: return MODE_0_INT;
        default: return INVALID_STAT_SOURCE;
    }
}

static void set_ppu_mode(uint8_t mode) {
    stat_interrupts_t new_mode_stat_source = ppu_mode_to_stat_source(mode);
    if (new_mode_stat_source != INVALID_STAT_SOURCE) {
        trigger_stat_source(new_mode_stat_source);
    }
    stat_interrupts_t old_mode_stat_source = ppu_mode_to_stat_source(ppu.mode);
    if (old_mode_stat_source != INVALID_STAT_SOURCE) {
        clear_stat_source(old_mode_stat_source);
    }

    uint8_t lcd_status = privileged_get_memory_byte(STAT);
    lcd_status &= ~(0x03);
    privileged_set_memory_byte(STAT, lcd_status | mode);
    ppu.mode = mode;
}

uint8_t get_x_pixel(void) { return ppu.line_x; }

uint8_t get_y_pixel(void) { return ppu.current_scan_line; }
uint8_t get_window_line(void) { return ppu.current_window_line; }
void end_ppu(void) { close_ppu = true; }
