#include "graphics.h"
#include "SDL_pixels.h"
#include "SDL_render.h"
#include "SDL_video.h"
#include "debug.h"
#include "decoder.h"
#include "hardware.h"
#include "ppu.h"
#include <SDL.h>
#include <ncurses.h>
#include <unistd.h>

#define MAX_TITLE_SIZE 16

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

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
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

void update_window_title(char *title) {
    char title_buffer[MAX_TITLE_SIZE + 1];
    strncpy(title_buffer, title, MAX_TITLE_SIZE);
    title_buffer[MAX_TITLE_SIZE] = '\0';
    SDL_SetWindowTitle(window, title_buffer);
    return;
}

void update_renderer(void) {
    ppu.ready_to_render = false;
    pthread_mutex_lock(&display_buffer_mutex);
    SDL_UpdateTexture(texture, NULL, get_display_buffer(),
                      DISPLAY_WIDTH * sizeof(uint32_t));
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    pthread_mutex_unlock(&display_buffer_mutex);
    SDL_RenderPresent(renderer);
}
