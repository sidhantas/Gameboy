#include "ppu.h"
#include "graphics.h"
#include "hardware.h"
#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>

void initialize_ppu(PPU *ppu) {
    ppu->line_dots = 0;
    ppu->mode = 0;
    ppu->ready_to_render = false;
}

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

void draw_background_tilemap(void) {
    // Tile address at 0x9800 is equivalent to 8 by 8 tile at 0, 0 on display
    // Tile at 0x9801 is 8 by 8 at 0, 8
    // Tile at 0x9820 is 8 by 8 at 8, 0

    for (uint16_t i = 0; i < 32; i++) {
        for (uint16_t j = 0; j < 32; j++) {
            // iterate through every tile in the tile map
            uint16_t tile_map_addr = 0x9800 + i * 32 + j;
            uint16_t relative_tile_address = get_memory_byte(tile_map_addr);
            uint16_t tile_start = 0x8000 + relative_tile_address * 0x10;
            set_display_buffer(tile_start, i, j);
        }
    }
}

void *refresh_loop(void *arg) {
    (void)arg;
    while (1) {
        draw_background_tilemap();
        usleep(5000);
    }
}
