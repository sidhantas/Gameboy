#include "graphics.h"
#include "SDL_pixels.h"
#include "SDL_render.h"
#include "debug.h"
#include "decoder.h"
#include "hardware.h"
#include "ppu.h"
#include <SDL2/SDL.h>
#include <ncurses.h>
#include <unistd.h>

SDL_Event event;
SDL_Renderer *renderer;
SDL_Texture *texture;
SDL_Window *window;

void open_window(void) {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow(
        "Gameboy", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        DISPLAY_WIDTH * RESOLUTION_SCALE, DISPLAY_HEIGHT * RESOLUTION_SCALE, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH,
                                DISPLAY_HEIGHT);
    SDL_RenderSetLogicalSize(renderer, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    SDL_RenderClear(renderer);
}

void close_window(void) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void update_pixel_buff(void) {
    ppu.ready_to_render = false;
    pthread_mutex_lock(&display_buffer_mutex);
    SDL_UpdateTexture(texture, NULL, get_display_buffer(),
                      DISPLAY_WIDTH * sizeof(uint32_t));
    pthread_mutex_unlock(&display_buffer_mutex);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

// static uint32_t get_color_from_byte(uint8_t byte, uint16_t x) {
//     switch (get_crumb(byte, 3 - x % 4)) {
//         case 0x00: return 0;
//         case 0x01: return 0x55555555;
//         case 0x02: return 0xAAAAAAAA;
//         case 0x03: return 0xFFFFFFFF;
//         default: return UINT32_MAX;
//     }
// }

// uint16_t draw_pixel_buff(uint16_t dots) {
//     const uint8_t PIXELS_PER_BYTE = 4;
//     uint8_t y = get_memory_byte(LCDY);
//     uint8_t scy = get_memory_byte(SCY);
//     if (y >= 144) {
//         set_memory_byte(LCDY, (y + 1) % SCAN_LINES);
//         return 0;
//     }
//     for (uint16_t x = ppu.line_dots; x < 456; x++) {
//         if (dots <= 0) {
//             ppu.line_dots = x;
//             return 0;
//         }
//         dots--;
//         if (x > 256) {
//             continue;
//         }
//         const uint8_t byte =
//             hardware
//                 .display_buffer[(TILE_MAP_WIDTH / PIXELS_PER_BYTE) * y + x /
//                 4];
//         pixel_buff[(y - scy) * WINDOW_WIDTH + x] = get_color_from_byte(byte,
//         x);
//     }
//     set_memory_byte(LCDY, (y + 1) % SCAN_LINES);
//     ppu.line_dots = 0;
//     return dots;
// }
