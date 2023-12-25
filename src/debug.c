#include "debug.h"
#include "decoder.h"
#include "graphics.h"
#include "hardware.h"
#include "utils.h"
#include <inttypes.h>
#include <ncurses.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

static void print_register_window(WINDOW *reg_win);
static void print_cpu_window(WINDOW *cpu_win);
static void print_flags_window(WINDOW *flags_win);
static void print_stack_window(WINDOW *stack_win);
static void print_memory_window(WINDOW *mem_win, uint16_t start_address);
static void print_display_buffer_window(WINDOW *display_buf_win,
                                        const uint16_t start_address);

uint16_t mem_win_addr = 0x8000;
uint16_t display_buf_addr = 0x0;
void end_debugger() { endwin(); }
void *initialize_debugger() {
    initscr();
    start_color();
    use_default_colors();
    init_pair(1, COLOR_RED, -1);
    cbreak();
    noecho();
    clear();
    nodelay(stdscr, TRUE);
    curs_set(0);
    refresh();

    WINDOW *display_buff_win = newwin(20, 90, 0, 54);
    WINDOW *registers_win = newwin(15, 21, 0, 0);
    WINDOW *cpu_win = newwin(20, 31, 0, 23);
    WINDOW *flags_win = newwin(8, 21, 17, 0);
    WINDOW *mem_win = newwin(21, 90, 21, 23);

    int instruction_count = 0;
    while (1) {
        char c = getch();
        if (c == 'm') {
            mem_win_addr += 0x100;
        } else if (c == 'M') {
            mem_win_addr -= 0x100;
        } else if (c == 'd') {
            display_buf_addr += 0x100;
        } else if (c == 'D') {
            display_buf_addr -= 0x100;
        }
        fetch_instruction();
        refresh_debugger(registers_win, flags_win, cpu_win, mem_win,
                         display_buff_win);
    }
    return NULL;
}

void refresh_debugger(WINDOW *registers_win, WINDOW *flags_win, WINDOW *cpu_win,
                      WINDOW *mem_win, WINDOW *disp_win) {
    print_register_window(registers_win);
    print_flags_window(flags_win);
    print_cpu_window(cpu_win);
    print_memory_window(mem_win, mem_win_addr);
    print_display_buffer_window(disp_win, display_buf_addr);
}

static void print_display_buffer_window(WINDOW *disp_win,
                                        const uint16_t start_address) {

    uint16_t WIDTH = 0;
    uint16_t HEIGHT = 0;

    getmaxyx(disp_win, HEIGHT, WIDTH);
    box(disp_win, 0, 0);
    mvwprintwhcenter(disp_win, 0, 0, WIDTH, "Display");
    for (int i = 2; i < WIDTH / 5; i++) {
        mvwprintw(disp_win, 1, i * 5, "0x%X", i - 2);
        wmove(disp_win, 1, i * 5 - 2);
        wvline(disp_win, 0, HEIGHT - 2);
    }
    wmove(disp_win, 2, 1);
    whline(disp_win, 0, WIDTH - 2);

    for (int i = 3; i < HEIGHT - 1; i++) {
        mvwprintw(disp_win, i, 1, "0x%0.4X", start_address + 0x10 * (i - 3));
        for (int j = 2; j < WIDTH / 5; j++) {
            if (hardware
                    .display_buffer[start_address + 0x10 * (i - 3) + (j - 2)]) {
                wattron(disp_win, COLOR_PAIR(1));
                mvwprintw(disp_win, i, j * 5 - 1, "0x%0.2X",
                          hardware.display_buffer[start_address +
                                                  0x10 * (i - 3) + (j - 2)]);
                wattroff(disp_win, COLOR_PAIR(1));
            } else {
                mvwprintw(disp_win, i, j * 5 - 1, "0x%0.2X",
                          hardware.display_buffer[start_address +
                                                  0x10 * (i - 3) + (j - 2)]);
            }
        }
    }
    wrefresh(disp_win);
}

static void print_memory_window(WINDOW *mem_win, uint16_t start_address) {
    uint16_t WIDTH = 0;
    uint16_t HEIGHT = 0;

    getmaxyx(mem_win, HEIGHT, WIDTH);
    box(mem_win, 0, 0);
    mvwprintwhcenter(mem_win, 0, 0, WIDTH, "Memory");
    for (int i = 2; i < WIDTH / 5; i++) {
        mvwprintw(mem_win, 1, i * 5, "0x%X", i - 2);
        wmove(mem_win, 1, i * 5 - 2);
        wvline(mem_win, 0, HEIGHT - 2);
    }
    wmove(mem_win, 2, 1);
    whline(mem_win, 0, WIDTH - 2);
    for (int i = 3; i < HEIGHT - 1; i++) {
        mvwprintw(mem_win, i, 1, "0x%X", start_address + 0x10 * (i - 3));
        for (int j = 2; j < WIDTH / 5; j++) {
            if (hardware.memory[start_address + 0x10 * (i - 3) + (j - 2)]) {
                wattron(mem_win, COLOR_PAIR(1));
                mvwprintw(
                    mem_win, i, j * 5 - 1, "0x%0.2X",
                    hardware.memory[start_address + 0x10 * (i - 3) + (j - 2)]);
                wattroff(mem_win, COLOR_PAIR(1));
            } else {
                mvwprintw(
                    mem_win, i, j * 5 - 1, "0x%0.2X",
                    hardware.memory[start_address + 0x10 * (i - 3) + (j - 2)]);
            }
        }
    }
    wrefresh(mem_win);
}

static void print_stack_window(WINDOW *stack_win) {
    uint16_t WIDTH = 0;
    uint16_t HEIGHT = 0;

    getmaxyx(stack_win, HEIGHT, WIDTH);
    box(stack_win, 0, 0);

    mvwprintwhcenter(stack_win, 0, 0, WIDTH, "STACK");
    wmove(stack_win, 1, WIDTH / 2);
    wvline(stack_win, 0, HEIGHT - 2);

    // Draw headers
    mvwprintwhcenter(stack_win, 1, 0, WIDTH / 2, "Address");
    mvwprintwhcenter(stack_win, 1, WIDTH / 2, WIDTH / 2, "Value");

    // Draw cell header division
    wmove(stack_win, 2, 1);
    whline(stack_win, 0, WIDTH - 2);

    for (uint32_t i = hardware.sp; i <= 0xFFFE; i += 2) {
        if (i == hardware.sp) {
            mvwprintwhcenter(stack_win, 4 + 0xFFFE - i / 2, 0, WIDTH / 2,
                             "0x%X", i);
            mvwprintwhcenter(stack_win, 4 + 0xFFFE - i / 2, WIDTH / 2,
                             WIDTH / 2, "0x00 0x00");
            continue;
        }
        mvwprintwhcenter(stack_win, 4 + 0xFFFE - i / 2, 0, WIDTH / 2, "0x%X",
                         i);
        mvwprintwhcenter(stack_win, 4 + 0xFFFE - i / 2, WIDTH / 2, WIDTH / 2,
                         "0x%0.2X 0x%0.2X", hardware.memory[i],
                         hardware.memory[i + 1]);
    }

    wrefresh(stack_win);
}

static void print_cpu_window(WINDOW *cpu_win) {
    uint16_t WIDTH = 0;
    uint16_t HEIGHT = 0;

    getmaxyx(cpu_win, HEIGHT, WIDTH);
    box(cpu_win, 0, 0);

    mvwprintwhcenter(cpu_win, 0, 0, WIDTH, "CPU");
    mvwprintwhcenter(cpu_win, 3, 0, WIDTH, "Instruction:");

    mvwprintwhcenter(cpu_win, 4, 0, WIDTH, "0x%0.2x 0x%0.2x 0x%0.2x",
                     hardware.instruction[0], hardware.instruction[1],
                     hardware.instruction[2]);
    mvwprintwhcenter(cpu_win, 6, 0, WIDTH, "Decode:");
    mvwprintwhcenter(cpu_win, 7, 0, WIDTH, "%s", hardware.decoded_instruction);
    mvwprintwhcenter(cpu_win, 9, 0, WIDTH, "Is Implemented: %s",
                     hardware.is_implemented ? "True" : "False");
    wrefresh(cpu_win);
}

static void print_register_window(WINDOW *registers_win) {
    uint16_t WIDTH = 0;
    uint16_t HEIGHT = 0;

    getmaxyx(registers_win, HEIGHT, WIDTH);
    box(registers_win, 0, 0);
    // Draw dividing line
    wmove(registers_win, 1, WIDTH / 2);
    wvline(registers_win, 0, HEIGHT - 2);

    // Draw headers
    mvwprintw(registers_win, 1, 2, "Register");
    mvwprintw(registers_win, 1, WIDTH / 2 + 2, "Value");

    // Draw cell header division
    wmove(registers_win, 2, 1);
    whline(registers_win, 0, WIDTH - 2);

    // Draw Register Labels and Values
    mvwprintw(registers_win, 3, WIDTH / 4 - 1, "B");
    mvwprintw(registers_win, 3, WIDTH / 2 + 2, "0x%0.2X",
              hardware.registers[B]);

    mvwprintw(registers_win, 4, WIDTH / 4 - 1, "C");
    mvwprintw(registers_win, 4, WIDTH / 2 + 2, "0x%0.2X",
              hardware.registers[C]);

    mvwprintw(registers_win, 5, WIDTH / 4 - 1, "D");
    mvwprintw(registers_win, 5, WIDTH / 2 + 2, "0x%0.2X",
              hardware.registers[D]);

    mvwprintw(registers_win, 6, WIDTH / 4 - 1, "E");
    mvwprintw(registers_win, 6, WIDTH / 2 + 2, "0x%0.2X",
              hardware.registers[E]);

    mvwprintw(registers_win, 7, WIDTH / 4 - 1, "H");
    mvwprintw(registers_win, 7, WIDTH / 2 + 2, "0x%0.2X",
              hardware.registers[H]);

    mvwprintw(registers_win, 8, WIDTH / 4 - 1, "L");
    mvwprintw(registers_win, 8, WIDTH / 2 + 2, "0x%0.2X",
              hardware.registers[L]);

    mvwprintw(registers_win, 9, WIDTH / 4 - 1, "F");
    mvwprintw(registers_win, 9, WIDTH / 2 + 2, "0x%0.2X",
              hardware.registers[F]);

    mvwprintw(registers_win, 10, WIDTH / 4 - 1, "A");
    mvwprintw(registers_win, 10, WIDTH / 2 + 2, "0x%0.2X",
              hardware.registers[A]);

    // Draw PC and SP labels
    mvwprintw(registers_win, 12, WIDTH / 4 - 1, "SP");
    mvwprintw(registers_win, 12, WIDTH / 2 + 2, "0x%0.2X", hardware.sp);

    mvwprintw(registers_win, 13, WIDTH / 4 - 1, "PC");
    mvwprintw(registers_win, 13, WIDTH / 2 + 2, "0x%0.2X", hardware.pc);
    wrefresh(registers_win);
}

static void print_flags_window(WINDOW *flags_win) {
    uint16_t WIDTH = 0;
    uint16_t HEIGHT = 0;

    getmaxyx(flags_win, HEIGHT, WIDTH);
    box(flags_win, 0, 0);

    // Draw dividing line
    wmove(flags_win, 1, WIDTH / 2);
    wvline(flags_win, 0, HEIGHT - 2);

    wmove(flags_win, 2, 1);
    whline(flags_win, 0, WIDTH - 2);
    // Draw headers
    mvwprintw(flags_win, 1, 2, "FLAG");
    mvwprintw(flags_win, 1, WIDTH / 2 + 2, "Value");

    mvwprintw(flags_win, 3, WIDTH / 4 - 1, "Z");
    mvwprintw(flags_win, 3, WIDTH / 2 + 2, "0x%0.2X", get_flag(Z_FLAG));

    mvwprintw(flags_win, 4, WIDTH / 4 - 1, "N");
    mvwprintw(flags_win, 4, WIDTH / 2 + 2, "0x%0.2X", get_flag(N_FLAG));

    mvwprintw(flags_win, 5, WIDTH / 4 - 1, "H");
    mvwprintw(flags_win, 5, WIDTH / 2 + 2, "0x%0.2X", get_flag(H_FLAG));

    mvwprintw(flags_win, 6, WIDTH / 4 - 1, "C");
    mvwprintw(flags_win, 6, WIDTH / 2 + 2, "0x%0.2X", get_flag(C_FLAG));

    wrefresh(flags_win);
}
