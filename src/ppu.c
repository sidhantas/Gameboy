#include "ppu.h"
#include "SDL_events.h"
#include "SDL_render.h"
#include "hardware.h"
#include "interrupts.h"
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

SDL_Event event;
SDL_Renderer *renderer;
SDL_Texture *texture;
SDL_Window *window;

bool close_ppu;
pthread_mutex_t dots_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t display_buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

static void render_loop(void);
static void execute_mode_0(void);
static void execute_mode_1(void);
static void execute_mode_2(void);
static void execute_mode_3(void);

static uint8_t get_bg_pixel(uint8_t x, uint8_t y);
static uint32_t get_color_from_byte(uint8_t byte);

void *start_ppu(void *arg) {
    (void)arg;
    ppu.mode = 2;
    render_loop();
    return NULL;
}

void initialize_ppu(void) {
    close_ppu = false;
    ppu.line_dots = 0;
    ppu.mode = 0;
    ppu.ready_to_render = false;
    ppu.available_dots = 0;
    ppu.consumed_dots = 0;
    ppu.line_x = 0;
}

void render_loop(void) {
    while (close_ppu == false) {
        switch (ppu.mode) {
            case 0: execute_mode_0(); break;
            case 1: execute_mode_1(); break;
            case 2: execute_mode_2(); break;
            case 3: execute_mode_3(); break;
            default: exit(1); break;
        }
    }
}

bool consume_dots(uint64_t dots_to_consume) {
    if (ppu.available_dots < dots_to_consume) {
        return false;
    };
    pthread_mutex_lock(&dots_mutex);
    ppu.available_dots -= dots_to_consume;
    pthread_mutex_unlock(&dots_mutex);
    ppu.consumed_dots += dots_to_consume;
    ppu.line_dots += dots_to_consume;
    return true;
}

static void execute_mode_2(void) {
    // wait for 2 dots
    static uint8_t object_index = 0;
    if (!consume_dots(2)) {
        return;
    }
    add_sprite(object_index);
    object_index++;
    if (ppu.line_dots > 80) {
        object_index = 0;
        ppu.line_x = 0;
        ppu.mode = 3;
    }
    return;
}

uint8_t get_obj_pixel(uint8_t x_pixel) {
    for (uint8_t i = 0; i < get_sprite_store()->length; i++) {
        const uint8_t x_start = get_sprite_store()->selected_objects[i].x_start;
        if (x_start < 8) {
            continue;
        }
        if (x_pixel - x_start - 8 < 8) {
            return 0x03;
        }
    }
    return 0x00;
}

void execute_mode_3(void) {
    uint16_t penalty_dots = 0;
    if (ppu.line_x == 0) {
        penalty_dots += get_memory_byte(SCX) % 8;
    }
    if (ppu.line_x == get_memory_byte(WX) - 7) {
        penalty_dots += 6;
    }
    // wait an extra dot for the pixel;
    if (!consume_dots(penalty_dots + 1)) {
        return;
    }
    // Should overlap with win pixel if it exists
    uint8_t pixel = get_bg_pixel(ppu.line_x, get_memory_byte(LCDY));
    const uint8_t object_pixel = get_obj_pixel(ppu.line_x);
    if (object_pixel == 0x03) {
        pixel = object_pixel;
    }

    pthread_mutex_lock(&display_buffer_mutex);
    set_display_pixel(ppu.line_x, get_memory_byte(LCDY),
                      get_color_from_byte(pixel));
    pthread_mutex_unlock(&display_buffer_mutex);
    if (++ppu.line_x > DISPLAY_WIDTH) {
        ppu.mode = 0;
    }
    return;
}

void execute_mode_0(void) {
    if (!consume_dots(1)) {
        return;
    }
    if (ppu.line_dots >= 456) {
        uint8_t current_y = get_memory_byte(LCDY) + 1;
        if (current_y > DISPLAY_HEIGHT) {
            ppu.ready_to_render = true;
            set_interrupts_flag(VBLANK);
            ppu.mode = 1;
        } else {
            ppu.mode = 2;
        }
        initialize_sprite_store();
        set_memory_byte(LCDY, current_y);
        ppu.line_dots %= 456;
    }
    return;
}

void execute_mode_1(void) {
    if (!consume_dots(1)) {
        return;
    }
    if (ppu.line_dots >= 456) {
        uint8_t current_y = get_memory_byte(LCDY) + 1;
        ppu.line_dots %= 456;
        if (current_y >= SCAN_LINES) {
            ppu.mode = 2;
        }
        set_memory_byte(LCDY, current_y % SCAN_LINES);
        // set_memory_byte(LCDY, 0x90);
    }
    return;
}

static inline uint16_t get_tile_start(uint16_t relative_tile_address) {
    int32_t tile_start;
    if (get_bit(get_memory_byte(LCDC), 4) == 1) {
        tile_start = ADDRESS_MODE_0_BP + relative_tile_address * 0x10;
    } else {
        tile_start =
            ADDRESS_MODE_1_BP + uint8_to_int8(relative_tile_address) * 0x10;
    }
    return (uint16_t)tile_start;
}

uint8_t get_pixel(uint8_t hi, uint8_t low, uint8_t n) {
    return (hi >> (n - 1) | low >> n) | 0xFF;
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
    return hi_bit << 1 | low_bit;
}

uint8_t get_object_pixel(uint16_t tile_row_address, uint8_t pixel_num) {

    uint8_t low = get_memory_byte(tile_row_address);
    uint8_t hi = get_memory_byte(tile_row_address + 1);

    // const uint8_t hi_bit = get_bit(hi, 7 - pixel_num % 8);
    // const uint8_t low_bit = get_bit(low, 7 - pixel_num % 8);

    return 0x03;
}

static uint32_t get_color_from_byte(uint8_t byte) {
    switch (byte) {
        case 0x00: return 0;
        case 0x01: return 0x55555555;
        case 0x02: return 0xAAAAAAAA;
        case 0x03: return 0xFFFFFFFF;
        default: return UINT32_MAX;
    }
}

void end_ppu(void) { close_ppu = true; }
