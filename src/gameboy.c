#include "SDL_events.h"
#include "SDL_scancode.h"
#include "cpu.h"
#include "debug.h"
#include "decoder.h"
#include "graphics.h"
#include "hardware.h"
#include "memory.h"
#include "ppu.h"
#include <getopt.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

uint64_t instructions_left = 0;
void main_loop(void);
void cleanup(void);

int main(int argc, char **argv) {
    pthread_t cpu_id;
    FILE *game;
    int long_index = 0;
    int opt = 0;
    static struct option program_options[] = {
        {"game", required_argument, 0, 'g'}, {0, 0, 0, 0}};

    open_window();
    initialize_hardware();
    initialize_ppu();
    initialize_io();
    while ((opt = getopt_long(argc, argv, "g:", program_options,
                              &long_index)) != -1) {
        switch (opt) {
            case 'g':
                if (!optarg) {
                    fprintf(stderr, "Must provide ROM path\n");
                    exit(1);
                }
                game = fopen(optarg, "r");
                if (!game) {
                    fprintf(stderr, "ROM does not exist\n");
                    exit(1);
                }
                load_rom(game);
                fclose(game);
                break;
            default: exit(1); break;
        }
    }
    atexit(&cleanup);

#ifdef ENABLE_DEBUGGER
    pthread_t debugger_id;
    pthread_create(&debugger_id, NULL, initialize_debugger, NULL);
#endif
    pthread_create(&cpu_id, NULL, start_cpu, NULL);
    main_loop();
    close_window();

#ifdef ENABLE_DEBUGGER
    end_debugger();
    pthread_join(debugger_id, NULL);
#endif
    end_ppu();
    end_cpu();
    pthread_join(cpu_id, NULL);
    save_data();
    return 0;
}

void cleanup(void) {
    destroy_memory();
    destroy_hardware();
}

void main_loop(void) {
    SDL_Event e;
    bool end_main_loop = false;
    while (!end_main_loop) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT: end_main_loop = true; break;
                case SDL_KEYDOWN:
                    switch (e.key.keysym.scancode) {
                        case SDL_SCANCODE_N: instructions_left += 1; break;
                        case SDL_SCANCODE_M: instructions_left += 100; break;
                        case SDL_SCANCODE_B: instructions_left += 1000; break;
#ifdef ENABLE_DEBUGGER
                        case SDL_SCANCODE_O: toggle_step_mode(); break;
#endif
                        case SDL_SCANCODE_W: reset_joypad_state(UP); break;
                        case SDL_SCANCODE_A: reset_joypad_state(LEFT); break;
                        case SDL_SCANCODE_S: reset_joypad_state(DOWN); break;
                        case SDL_SCANCODE_D: reset_joypad_state(RIGHT); break;
                        case SDL_SCANCODE_J:
                            reset_joypad_state(A_BUTTON);
                            break;
                        case SDL_SCANCODE_K:
                            reset_joypad_state(B_BUTTON);
                            break;
                        case SDL_SCANCODE_C: reset_joypad_state(SELECT); break;
                        case SDL_SCANCODE_V: reset_joypad_state(START); break;
                        default: break;
                    }
                    break;
                case SDL_KEYUP:
                    switch (e.key.keysym.scancode) {
                        case SDL_SCANCODE_W: set_joypad_state(UP); break;
                        case SDL_SCANCODE_A: set_joypad_state(LEFT); break;
                        case SDL_SCANCODE_S: set_joypad_state(DOWN); break;
                        case SDL_SCANCODE_D: set_joypad_state(RIGHT); break;
                        case SDL_SCANCODE_J: set_joypad_state(A_BUTTON); break;
                        case SDL_SCANCODE_K: set_joypad_state(B_BUTTON); break;
                        case SDL_SCANCODE_C: set_joypad_state(SELECT); break;
                        case SDL_SCANCODE_V: set_joypad_state(START); break;
                        default: break;
                    }
                    break;
            }
        }
        if (ppu.ready_to_render) {
            update_renderer();
        }
    }
}
