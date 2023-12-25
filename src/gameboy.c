#include "SDL.h"
#include "debug.h"
#include "ppu.h"
#include "decoder.h"
#include "graphics.h"
#include "hardware.h"
#include "utils.h"
#include <getopt.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Hardware hardware;

int main(int argc, const char *argv[]) {
    pthread_t debugger_id;
    pthread_t ppu_id;
    initialize_hardware(&hardware);
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
    pthread_create(&ppu_id, NULL, refresh_loop, NULL);
    open_window();
    end_debugger();
    SDL_Quit();
    pthread_join(debugger_id, NULL);
    pthread_join(ppu_id, NULL);

    return 0;
}
