#include "SDL_events.h"
#include "cpu.h"
#include "debug.h"
#include "decoder.h"
#include "graphics.h"
#include "hardware.h"
#include "ppu.h"
#include "utils.h"
#include <getopt.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

Hardware hardware;
PPU ppu;
void main_loop(void);

int main(int argc, char **argv) {
    pthread_t debugger_id;
    pthread_t cpu_id;
    pthread_t ppu_id;
    initialize_hardware(&hardware);
    initialize_ppu(&ppu);

    FILE *dmg = fopen("dmg.bin", "r");
    FILE *game;
    load_dmg(dmg);
    int long_index = 0;
    int opt = 0;
    static struct option program_options[] = {
        {"game", required_argument, 0, 'g'}, {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, "g:", program_options,
                              &long_index)) != -1) {
        switch (opt) {
            case 'g':
                game = fopen(optarg, "r");
                load_rom(game);
                fclose(game);
                break;
            default: exit(1); break;
        }
    }

    pthread_create(&debugger_id, NULL, initialize_debugger, NULL);
    open_window();
    pthread_create(&cpu_id, NULL, start_cpu, NULL);
    pthread_create(&ppu_id, NULL, start_ppu, NULL);
    main_loop();
    close_window();
    end_debugger();
    end_ppu();
    end_cpu();
    pthread_join(debugger_id, NULL);
    pthread_join(cpu_id, NULL);
    pthread_join(ppu_id, NULL);

    return 0;
}

void main_loop(void) {
    SDL_Event e;
    bool end_main_loop = false;
    while (!end_main_loop) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    end_main_loop = true;
                    break;
            }
        }
        if (ppu.ready_to_render) {
            update_pixel_buff();
        }
    }
}
