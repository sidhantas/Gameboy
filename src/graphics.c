#include "graphics.h"
#include "SDL_pixels.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "debug.h"
#include "decoder.h"
#include "hardware.h"
#include "ppu.h"
#include "utils.h"
#include <SDL2/SDL.h>
#include <ncurses.h>
#include <signal.h>
#include <unistd.h>

#define WINDOW_WIDTH 160
#define WINDOW_HEIGHT 144
#define SCAN_LINES 154
SDL_Event event;
SDL_Renderer *renderer;
SDL_Texture *texture;
SDL_Window *window;
uint32_t *pixel_buff;

void open_window(void) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH * RESOLUTION_SCALE,
                                WINDOW_HEIGHT * RESOLUTION_SCALE, 0, &window,
                                &renderer);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH,
                                WINDOW_HEIGHT);
    pixel_buff = calloc(WINDOW_WIDTH * WINDOW_WIDTH, sizeof(uint32_t));
    SDL_RenderSetLogicalSize(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    SDL_RenderClear(renderer);
}

void close_window(void) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void update_pixel_buff(uint16_t dots, uint32_t *exec_count) {
    while (dots > 0) {
        dots = draw_pixel_buff(dots);
        uint8_t y = get_memory_byte(LCDY);
        if (y == 0 && ppu.line_dots == 0 && *exec_count > 100000) {
            SDL_UpdateTexture(texture, NULL, pixel_buff,
                              WINDOW_WIDTH * sizeof(uint32_t));
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
                exit(0);
            }
            ppu.ready_to_render = false;
            SDL_RenderPresent(renderer);
            *exec_count = 0;
        }
    }
}

uint32_t get_color_from_byte(uint8_t byte, uint16_t x) {
    switch (get_crumb(byte, x % 4)) {
        case 0x00: return 0;
        case 0x01: return 0x55555555;
        case 0x02: return 0xAAAAAAAA;
        case 0x03: return 0xFFFFFFFF;
        default: return UINT32_MAX;
    }
}

uint16_t draw_pixel_buff(uint16_t dots) {
    const uint8_t PIXELS_PER_BYTE = 4;
    uint8_t y = get_memory_byte(LCDY);
    if (y >= 144) {
        set_memory_byte(LCDY, (y + 1) % SCAN_LINES);
        return 0;
    }
    for (uint16_t x = ppu.line_dots; x < WINDOW_WIDTH; x++) {
        if (dots <= 0) {
            ppu.line_dots = x;
            return 0;
        }
        dots--;
        const uint8_t byte =
            hardware
                .display_buffer[(TILE_MAP_WIDTH / PIXELS_PER_BYTE) * y + x / 4];
        pixel_buff[y * WINDOW_WIDTH + x] = get_color_from_byte(byte, x);
    }
    set_memory_byte(LCDY, (y + 1) % SCAN_LINES);
    ppu.line_dots = 0;
    return dots;
}
