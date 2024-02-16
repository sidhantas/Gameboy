#include "ppu.h"
#include "hardware.h"
#include "utils.h"
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define ADDRESS_MODE_0_BP 0x8000
#define ADDRESS_MODE_1_BP 0x9000

bool close_ppu;

void initialize_ppu(PPU *ppu) {
    close_ppu = false;
    ppu->line_dots = 0;
    ppu->mode = 0;
    ppu->ready_to_render = false;
}

void end_ppu(void) { close_ppu = true; }

static uint8_t get_row_byte_low(uint8_t hi, uint8_t low) {
    uint8_t new = 0;
    for (uint8_t i = 0; i < 4; i++) {
        new |= ((hi >> i) & 0x1) << (2 * i + 1);
        new |= ((low >> i) & 0x1) << (2 * i);
    }
    return new;
}

static uint8_t get_row_byte_hi(uint8_t hi, uint8_t low) {
    uint8_t new = 0;
    for (uint8_t i = 0; i < 4; i++) {
        new |= ((hi >> (4 + i)) & 0x1) << (2 * i + 1);
        new |= ((low >> (4 + i)) & 0x1) << (2 * i);
    }
    return new;
}

void set_display_buffer(uint16_t tile_start, uint8_t row, uint8_t col) {
    uint16_t start = row * 64 * 8 + col * 2;
    for (uint16_t i = 0; i < 8; i++) {
        uint8_t low = get_memory_byte(tile_start + i * 2);
        uint8_t hi = get_memory_byte(tile_start + i * 2 + 1);
        hardware.display_buffer[start + i * 64] = get_row_byte_hi(hi, low);
        hardware.display_buffer[start + i * 64 + 1] = get_row_byte_low(hi, low);
    }
}

void set_win_display_buffer(uint16_t tile_start, uint8_t row, uint8_t col) {
    uint16_t start = row * 64 * 8 + col * 2;
    for (uint16_t i = 0; i < 8; i++) {
        uint8_t low = get_memory_byte(tile_start + i * 2);
        uint8_t hi = get_memory_byte(tile_start + i * 2 + 1);
        hardware.display_buffer[start + i * 64] |= get_row_byte_hi(hi, low);
        hardware.display_buffer[start + i * 64 + 1] |= get_row_byte_low(hi, low);
    }
}

static inline uint16_t get_tile_start(uint16_t relative_tile_address) {
    uint16_t tile_start;
    if (get_bit(get_memory_byte(LCDC), 4) == 1) {
        tile_start = ADDRESS_MODE_0_BP + relative_tile_address * 0x10;
    } else {
        tile_start =
            ADDRESS_MODE_1_BP + uint8_to_int8(relative_tile_address) * 0x10;
    }
    return tile_start;
}

uint16_t draw_background_tilemap(uint16_t dots) {
    // Tile address at 0x9800 is equivalent to 8 by 8 tile at 0, 0 on display
    // Tile at 0x9801 is 8 by 8 at 0, 8
    // Tile at 0x9820 is 8 by 8 at 8, 0
    //
    // TODO: Need to use dots here
    //

    static uint16_t i = 0;
    static uint16_t j = 0;

    const uint8_t LCDY_VAL = get_memory_byte(LCDY);

    while (i < 456) {
        const uint16_t BG_TILE_MAP_AREA = get_bit(get_memory_byte(LCDC), 3) ? 0x9C00 : 0x9800;

        i++;
        dots--;
        if (!dots) {
            return 0;
        }
    }
    for (uint16_t i = 0; i < 32; i++) {
        for (uint16_t j = 0; j < 32; j++) {
            // iterate through every tile in the tile map
            const uint16_t BG_TILE_MAP_AREA = get_bit(get_memory_byte(LCDC), 3) ? 0x9C00 : 0x9800;
            const uint16_t TILE_MAP_ADDR = BG_TILE_MAP_AREA + i * 32 + j;
            const uint16_t tile_start = get_tile_start(get_memory_byte(TILE_MAP_ADDR));
            set_display_buffer(tile_start, i, j);
        }
    }
}


void draw_window_tilemap(uint16_t dots) {
    for (uint16_t i = 0; i < 32; i++) {
        for (uint16_t j = 0; j < 32; j++) {
            const uint16_t WIN_TILE_MAP_AREA = get_bit(get_memory_byte(LCDC), 6) ? 0x9C00 : 0x9800;
            const uint16_t TILE_MAP_ADDR = WIN_TILE_MAP_AREA + i * 32 + j;
            const uint16_t tile_start = get_tile_start(get_memory_byte(TILE_MAP_ADDR));
            set_win_display_buffer(tile_start, i, j);
        }
    }
}

void *refresh_loop(void *arg) {
    (void)arg;
    while (true) {
        if (close_ppu) {
            break;
        }

        set_bit(&hardware.memory[LCDC], 6);
        const uint8_t LCDC_VAL = get_memory_byte(LCDC);
        if (get_bit(LCDC_VAL, 0)) {
            draw_background_tilemap();
            if (get_bit(LCDC_VAL, 5)) {
                draw_window_tilemap();
            }
        }
        usleep(5000);
    }
    return NULL;
}
