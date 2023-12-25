#include "graphics.h"
#include "hardware.h"
#include <SDL2/SDL.h>
#include <signal.h>

#define WINDOW_WIDTH 256
SDL_Event event;
SDL_Renderer *renderer;
SDL_Window *window;
int i;

void open_window() {

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, 0, &window,
                                &renderer);
    while (1) {
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
            break;
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

static uint8_t get_nibble(uint8_t byte, uint8_t nibble) {
    uint8_t nibble_byte = 0b1 << (7 - nibble);
    return (byte & nibble_byte) >> (7 - nibble);
}

void draw_pixel_buff() {
    uint8_t is_set = 0;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    for (uint16_t y = 0; y < WINDOW_WIDTH; y++) {
        for (uint16_t x = 0; x < WINDOW_WIDTH / 8; x++) {
            const uint8_t byte = hardware.display_buffer[WINDOW_WIDTH / 8 * y + x];
            for (uint8_t n = 0; n < 8; n++) {
                if (get_nibble(byte, n)) {
                    SDL_RenderDrawPoint(renderer, x * 8 + n, y);
                }
            }
        }
    }
    SDL_RenderPresent(renderer);
}
