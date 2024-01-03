#include "SDL.h"
#include "debug.h"
#include "decoder.h"
#include "graphics.h"
#include "hardware.h"
#include "instructions.h"
#include "ppu.h"
#include "utils.h"
#include <getopt.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

Hardware hardware;
PPU ppu;
void main_loop(void);

int main(int argc, char **argv) {
    pthread_t debugger_id;
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
    pthread_create(&ppu_id, NULL, refresh_loop, NULL);
    open_window();
    main_loop();
    close_window();
    end_debugger();
    pthread_join(debugger_id, NULL);
    pthread_join(ppu_id, NULL);

    return 0;
}

void main_loop(void) {
    uint32_t exec_count = 0;
    struct timeval start, end, diff;
    gettimeofday(&start, NULL);
    while (1) {
        if (hardware.pc == 0x100) {
            break;
        }
        clock_cycles_t (*func)(uint8_t *) = fetch_instruction();
        clock_cycles_t clocks = execute_instruction(func);
        uint16_t dots = clocks * 4;
        update_pixel_buff(dots, &exec_count);
        gettimeofday(&end, NULL);
        if (end.tv_usec < start.tv_usec) {
            diff.tv_usec = 1000000 + end.tv_usec - start.tv_usec;
        } else {
            diff.tv_usec = end.tv_usec - start.tv_usec;
        }
        usleep((1000000 - diff.tv_usec) / CLOCK_RATE);
        gettimeofday(&start, NULL);
        exec_count++;
    }
}
