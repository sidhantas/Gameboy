#include "ppu.h"
#include "graphics.h"
#include "hardware.h"
#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>

void set_display_buffer(uint16_t tile_start, uint8_t row, uint8_t col) {
    // start = row * tiles_per_row * bytes_per_tile
    uint16_t start = row * 64 * 8 + col;
//    for (uint16_t i = 0; i < 8; i++) {
//        for (uint16_t j = 0; j < 2; j++) {
//            uint8_t tile_byte = hardware.memory[tile_start + i + j];
//            hardware.display_buffer[start + i * 64 + j] = tile_byte;
//        }
//    }
//

    for (uint16_t i = 0; i < 8; i++) {
        for (uint8_t j = 0; j < 2; j++) {
            uint8_t tile_byte = hardware.memory[tile_start + i * 2 + j];
            hardware.display_buffer[start + i * 64 + j] = tile_byte;
        }
    }
}

void draw_background_tilemap() {
    // Tile address at 0x9800 is equivalent to 8 by 8 tile at 0, 0 on display
    // Tile at 0x9801 is 8 by 8 at 0, 8
    // Tile at 0x9820 is 8 by 8 at 8, 0

    for (uint16_t i = 0; i < 32; i++) {
        for (uint16_t j = 0; j < 32; j++) {
            // iterate through every tile in the tile map
            uint16_t tile_map_addr = 0x9800 + i * 32 + j;
            uint16_t relative_tile_address = hardware.memory[tile_map_addr];
            uint16_t tile_start = 0x8000 + relative_tile_address * 0x10;
            set_display_buffer(tile_start, i, j);
        }
    }
}

void *refresh_loop() {
    while (1) {
        draw_background_tilemap();
        draw_pixel_buff();
        usleep(5000);
    }
}
