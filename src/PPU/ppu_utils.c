#include "ppu_utils.h"
#include "hardware.h"
#include "memory.h"
#include "oam_queue.h"
#include "utils.h"
#include <stdlib.h>

static inline uint16_t get_tile_start(uint8_t relative_tile_address);
static uint8_t get_background_pixel_color(uint8_t pixel_id);
static uint8_t get_obj_pixel_color(uint8_t pixel_id, uint16_t pallete);
static uint8_t get_color_id(uint8_t hi_byte, uint8_t lo_byte,
                            uint8_t pixel_num);

uint8_t get_bg_pixel(uint8_t x, uint8_t y) {
    const uint8_t x_off = get_memory_byte(SCX);
    const uint8_t y_off = get_memory_byte(SCY);

    x = (x + x_off) % DISPLAY_WIDTH;
    y = (y + y_off) % DISPLAY_WIDTH;
    const uint8_t tile_x = x / 8;
    const uint8_t tile_y = y / 8;

    const uint16_t BG_TILE_MAP_AREA =
        get_bit(get_memory_byte(LCDC), 3) ? 0x9C00 : 0x9800;
    const uint16_t TILE_MAP_ADDR = BG_TILE_MAP_AREA + tile_y * 32 + tile_x;
    const uint16_t tile_start = get_tile_start(get_memory_byte(TILE_MAP_ADDR));
    uint8_t low = get_memory_byte(tile_start + (y % 8) * 2);
    uint8_t hi = get_memory_byte(tile_start + (y % 8) * 2 + 1);

    uint8_t color_id = get_color_id(hi, low, x);
    // intertwine bytes
    return get_background_pixel_color(color_id);
}

uint8_t get_win_pixel(uint8_t x, uint8_t y) {
    if (!get_bit(get_memory_byte(LCDC), 5)) {
        return TRANSPARENT;
    }
    const uint8_t x_off = get_memory_byte(WX);
    const uint8_t y_off = get_memory_byte(WY);

    if (x_off > 166 || y_off > 143) {
        return TRANSPARENT;
    }

    x = x + x_off - 7;
    y = y + y_off;
    const uint8_t tile_x = x / 8;
    const uint8_t tile_y = y / 8;

    const uint16_t win_tile_map_area =
        get_bit(get_memory_byte(LCDC), 6) ? 0x9C00 : 0x9800;
    const uint16_t tile_map_addr = win_tile_map_area + tile_y * 32 + tile_x;
    const uint16_t tile_start = get_tile_start(get_memory_byte(tile_map_addr));
    if (tile_start == ADDRESS_MODE_0_BP || tile_start == ADDRESS_MODE_1_BP) {
        return TRANSPARENT;
    }
    uint8_t low = get_memory_byte(tile_start + (y % 8) * 2);
    uint8_t hi = get_memory_byte(tile_start + (y % 8) * 2 + 1);

    uint8_t color_id = get_color_id(hi, low, x);
    return get_background_pixel_color(color_id);
}

uint8_t get_obj_pixel(uint8_t x_pixel) {
    if (!get_bit(get_memory_byte(LCDC), 1)) {
        return TRANSPARENT;
    }
    for (uint8_t i = 0; i < get_sprite_store()->length; i++) {
        struct ObjectRowData obj = get_sprite_store()->selected_objects[i];
        const uint8_t x_start = obj.x_start;
        if (x_start - 8 <= x_pixel && x_pixel < x_start) {
            uint16_t tile_start = obj.tile_start;

            uint8_t relative_y = (obj.y % 8) * 2;
            if (obj.y_flipped) {
                relative_y = 15 - relative_y;
            }

            uint8_t low = get_memory_byte(tile_start + relative_y);
            uint8_t hi = get_memory_byte(tile_start + relative_y + 1);

            uint8_t relative_x = x_start - x_pixel - 1;
            if (!obj.x_flipped) {
                relative_x = 7 - relative_x;
            }

            uint8_t color_id = get_color_id(hi, low, relative_x);
            uint16_t obj_palette = obj.DMG_palette == 1 ? 0xFF49 : 0xFF48;
            enum COLOR_VALUES color =
                get_obj_pixel_color(color_id, obj_palette);
            if (color != TRANSPARENT) {
                return (uint8_t)color;
            }
        }
    }
    return TRANSPARENT;
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

static uint8_t get_color_id(uint8_t hi_byte, uint8_t lo_byte,
                            uint8_t pixel_num) {
    const uint8_t hi_bit = get_bit(hi_byte, 7 - pixel_num % 8);
    const uint8_t low_bit = get_bit(lo_byte, 7 - pixel_num % 8);

    return (uint8_t)(hi_bit << 1 | low_bit);
}
static uint8_t get_background_pixel_color(uint8_t pixel_id) {
    uint8_t bgp = get_memory_byte(0xFF47);
    return (bgp >> pixel_id * 2) & 0x03;
}

static uint8_t get_obj_pixel_color(uint8_t pixel_id, uint16_t pallete) {
    if (pixel_id == 0x00) {
        return TRANSPARENT;
    }

    uint8_t obp = get_memory_byte(pallete);
    return (obp >> pixel_id * 2) & 0x03;
}

uint32_t get_color_from_byte(uint8_t byte) {
    switch (byte) {
        case 0x03: return 0;
        case 0x02: return 0x55555555;
        case 0x01: return 0xAAAAAAAA;
        case 0x00: return 0xFFFFFFFF;
        default: fprintf(stderr, "Invalid color pallete value"); exit(1);
    }
}
