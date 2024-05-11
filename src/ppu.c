#include "ppu.h"
#include "hardware.h"
#include "interrupts.h"
#include "memory.h"
#include "oam_queue.h"
#include "utils.h"
#include <ncurses.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#define ADDRESS_MODE_0_BP 0x8000
#define ADDRESS_MODE_1_BP 0x9000
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
static uint8_t get_bg_pixel(uint8_t x, uint8_t y);
static uint8_t get_win_pixel(uint8_t x, uint8_t y);
static uint32_t get_color_from_byte(uint8_t byte);

void initialize_ppu(void) {
    close_ppu = false;
    ppu.line_dots = 0;
    ppu.mode = 2;
    ppu.ready_to_render = false;
    ppu.available_dots = 0;
    ppu.consumed_dots = 0;
    ppu.current_scan_line = 0;
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
    ppu.available_dots += dots;
    do {
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

static bool execute_mode_2(void) {
    // wait for 2 dots
    static uint8_t object_index = 0;
    if (!consume_dots(2)) {
        return false;
    }
    add_sprite(object_index);
    object_index++;
    if (ppu.line_dots >= 80) {
        object_index = 0;
        ppu.line_x = 0;
        set_ppu_mode(3);
        ppu.current_scan_line = get_memory_byte(LCDY);
    }
    return true;
}

uint8_t get_obj_pixel(uint8_t x_pixel) {
    for (uint8_t i = 0; i < get_sprite_store()->length; i++) {
        const uint8_t x_start = get_sprite_store()->selected_objects[i].x_start;
        if (x_start < 8) {
            continue;
        }
        if (x_start - 8 == x_pixel - x_pixel % 8) {
            uint16_t tile_start =
                get_sprite_store()->selected_objects[i].tile_row_index;

            uint8_t y = get_sprite_store()->selected_objects[i].y;
            uint8_t low = get_memory_byte(tile_start + (y % 8) * 2);
            uint8_t hi = get_memory_byte(tile_start + (y % 8) * 2 + 1);
            const uint8_t hi_bit = get_bit(hi, 7 - x_pixel % 8);
            const uint8_t low_bit = get_bit(low, 7 - x_pixel % 8);

            return (uint8_t)(hi_bit << 1 | low_bit);
        }
    }
    return 0x05;
}

static bool execute_mode_3(void) {
    uint16_t penalty_dots = 0;
    if (ppu.line_x == 0) {
        penalty_dots += get_memory_byte(SCX) % 8;
    }
    if (ppu.line_x == get_memory_byte(WX) - 7) {
        penalty_dots += 6;
    }
    // wait an extra dot for the pixel;
    if (!consume_dots(penalty_dots + 1)) {
        return false;
    }
    // Should overlap with win pixel if it exists
    uint8_t pixel = get_bg_pixel(ppu.line_x, ppu.current_scan_line);
    //uint8_t win_pixel = get_win_pixel(ppu.line_x, ppu.current_scan_line);
//    if (win_pixel != 0x05) {
//        pixel = win_pixel;
//    }

    const uint8_t object_pixel = get_obj_pixel(ppu.line_x);
    if (object_pixel != 0x05) {
        pixel = object_pixel;
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
    if (ppu.line_dots >= 456) {
        increase_scan_line();
        if (ppu.current_scan_line >= DISPLAY_HEIGHT) {
            ppu.ready_to_render = true;
            set_interrupts_flag(VBLANK);
            trigger_stat_source(MODE_1_INT);
            set_ppu_mode(1);
        } else {
            trigger_stat_source(MODE_2_INT);
            set_ppu_mode(2);
            initialize_sprite_store();
        }
        set_memory_byte(LCDY, ppu.current_scan_line);
        ppu.line_dots %= 456;
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
        if (ppu.current_scan_line >= SCAN_LINES) {
            set_ppu_mode(2);
        }
    }
    return true;
}

static inline uint16_t get_tile_start(uint8_t relative_tile_address) {
    int32_t tile_start;
    if (get_bit(get_memory_byte(LCDC), 4) == 1) {
        tile_start = ADDRESS_MODE_0_BP + relative_tile_address * 0x10;
    } else {
        tile_start =
            ADDRESS_MODE_1_BP + uint8_to_int8(relative_tile_address) * 0x10;
    }
    return (uint16_t)tile_start;
}

uint8_t get_bg_pixel(uint8_t x, uint8_t y) {
    const uint8_t x_off = get_memory_byte(SCX);
    const uint8_t y_off = get_memory_byte(SCY);

    x = x + x_off;
    y = y + y_off;
    const uint8_t tile_x = x / 8;
    const uint8_t tile_y = y / 8;

    const uint16_t BG_TILE_MAP_AREA =
        get_bit(get_memory_byte(LCDC), 3) ? 0x9C00 : 0x9800;
    const uint16_t TILE_MAP_ADDR = BG_TILE_MAP_AREA + tile_y * 32 + tile_x;
    const uint16_t tile_start = get_tile_start(get_memory_byte(TILE_MAP_ADDR));
    uint8_t low = get_memory_byte(tile_start + (y % 8) * 2);
    uint8_t hi = get_memory_byte(tile_start + (y % 8) * 2 + 1);

    // intertwine bytes
    const uint8_t hi_bit = get_bit(hi, 7 - x % 8);
    const uint8_t low_bit = get_bit(low, 7 - x % 8);
    return (uint8_t)(hi_bit << 1 | low_bit);
}

static uint8_t get_win_pixel(uint8_t x, uint8_t y) {
    const uint8_t x_off = get_memory_byte(WX);
    const uint8_t y_off = get_memory_byte(WY);

    x = x + x_off;
    y = y + y_off;
    const uint8_t tile_x = x / 8;
    const uint8_t tile_y = y / 8;

    const uint16_t BG_TILE_MAP_AREA =
        get_bit(get_memory_byte(LCDC), 6) ? 0x9C00 : 0x9800;
    const uint16_t TILE_MAP_ADDR = BG_TILE_MAP_AREA + tile_y * 32 + tile_x;
    const uint16_t tile_start = get_tile_start(get_memory_byte(TILE_MAP_ADDR));
    if ((tile_start & 0x00FF) == 0x00) {
        return 0x05;
    }
    uint8_t low = get_memory_byte(tile_start + (y % 8) * 2);
    uint8_t hi = get_memory_byte(tile_start + (y % 8) * 2 + 1);

    // intertwine bytes
    const uint8_t hi_bit = get_bit(hi, 7 - x % 8);
    const uint8_t low_bit = get_bit(low, 7 - x % 8);
    return (uint8_t)(hi_bit << 1 | low_bit);

}

static uint32_t get_color_from_byte(uint8_t byte) {
    switch (byte) {
        case 0x03: return 0;
        case 0x02: return 0x55555555;
        case 0x01: return 0xAAAAAAAA;
        case 0x00: return 0xFFFFFFFF;
        default: exit(1);
    }
}

static void increase_scan_line(void) {
    ppu.current_scan_line += 1;
    set_memory_byte(LCDY, ppu.current_scan_line % SCAN_LINES);

    if (privileged_get_memory_byte(LYC) == ppu.current_scan_line) {
        trigger_stat_source(LYC_INT);
    }
    return;
}

stat_interrupts_t ppu_mode_to_stat_source(uint8_t mode) {
    switch (mode) {
        case 2:
            return MODE_2_INT;
        case 1:
            return MODE_1_INT;
        case 0:
            return MODE_0_INT;
        default:
            return INVALID_STAT_SOURCE;
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
    
    uint8_t lcd_status = get_memory_byte(STAT);
    lcd_status &= ~(0x03);
    set_memory_byte(STAT, lcd_status | mode);
    ppu.mode = mode;
}

void end_ppu(void) { close_ppu = true; }
