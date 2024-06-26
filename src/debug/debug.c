#include "debug.h"
#include "decoder.h"
#include "hardware.h"
#include "interrupts.h"
#include "memory.h"
#include "oam_queue.h"
#include "ppu.h"
#include "utils.h"
#include <inttypes.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

static void print_register_window(WINDOW *reg_win);
static void print_cpu_window(WINDOW *cpu_win);
static void print_ppu_window(WINDOW *ppu_win);
static void print_flags_window(WINDOW *flags_win);
static void print_stack_window(WINDOW *stack_win);
static void print_memory_window(WINDOW *mem_win, uint16_t start_address);
static void print_oam_window(WINDOW *oam_win);
static void print_display_buffer_window(WINDOW *display_buf_win,
                                        const uint16_t start_address);

void refresh_debugger(void);
WINDOW *display_buff_win;
WINDOW *registers_win;
WINDOW *cpu_win;
WINDOW *ppu_win;
WINDOW *flags_win;
WINDOW *stack_win;
WINDOW *mem_win;
WINDOW *oam_win;
uint16_t mem_win_addr = 0x0000;
uint16_t display_buf_addr = 0x0;

pthread_mutex_t debugger_lock = PTHREAD_MUTEX_INITIALIZER;
bool close_debugger;

void end_debugger(void) {
    close_debugger = true;
    pthread_mutex_lock(&debugger_lock);
    noraw();
    nocbreak();
    echo();
    curs_set(1);
    endwin();
    pthread_mutex_unlock(&debugger_lock);
    return;
}
void *initialize_debugger(void *arg) {
    (void)arg;
    pthread_mutex_lock(&debugger_lock);
    initscr();
    start_color();
    use_default_colors();
    init_pair(1, COLOR_RED, -1);
    raw();
    clear();
    nodelay(stdscr, TRUE);
    curs_set(0);
    refresh();
    display_buff_win = newwin(20, 90, 0, 54);
    registers_win = newwin(17, 21, 0, 0);
    cpu_win = newwin(20, 31, 0, 23);
    ppu_win = newwin(20, 31, 0, 55);
    flags_win = newwin(8, 21, 17, 0);
    mem_win = newwin(20, 90, 21, 23);
    stack_win = newwin(21, 31, 21, 114);
    oam_win = newwin(20, 31, 0, 114);

    while (1) {
        if (close_debugger) {
            break;
        }
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
        refresh_debugger();
        usleep(20000);
    }
    pthread_mutex_unlock(&debugger_lock);
    return NULL;
}

void refresh_debugger(void) {
    print_register_window(registers_win);
    print_flags_window(flags_win);
    print_cpu_window(cpu_win);
    print_ppu_window(ppu_win);
    print_memory_window(mem_win, mem_win_addr);
    // print_display_buffer_window(display_buff_win, display_buf_addr);
    print_stack_window(stack_win);
    print_oam_window(oam_win);
    refresh();
}

static void print_oam_window(WINDOW *oam_win) {
    uint16_t WIDTH = 0;
    uint16_t _HEIGHT = 0;
    werase(oam_win);
    getmaxyx(oam_win, _HEIGHT, WIDTH);
    box(oam_win, 0, 0);
    mvwprintwhcenter(oam_win, 0, 0, WIDTH, "OAM");
    SpriteStore *sprite_store = get_sprite_store();
    mvwprintwhcenter(oam_win, 1, 0, WIDTH / 2, "LY");
    mvwprintwhcenter(oam_win, 1, WIDTH / 2, WIDTH / 2, "0x%x",
                     privileged_get_memory_byte(LCDY));

    mvwprintwhcenter(oam_win, 2, 0, WIDTH / 3, "X Pos");
    mvwprintwhcenter(oam_win, 2, WIDTH / 3, WIDTH / 3, "Y Pos");
    mvwprintwhcenter(oam_win, 2, 2 * WIDTH / 3, WIDTH / 3, "Tile start");
    for (uint16_t i = 3; i < sprite_store->length + 3; i++) {
        mvwprintwhcenter(oam_win, i, 0, WIDTH / 3, "0x%x",
                         sprite_store->selected_objects[i].x_start);
        mvwprintwhcenter(oam_win, i, WIDTH / 3, WIDTH / 3, "0x%x",
                         sprite_store->selected_objects[i].y);
        mvwprintwhcenter(oam_win, i, 2 * WIDTH / 3, WIDTH / 3, "0x%x",
                         sprite_store->selected_objects[i].tile_start);
    }
    wrefresh(oam_win);
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
            if (get_display_buffer()[start_address + 0x10 * (i - 3) +
                                     (j - 2)]) {
                wattron(disp_win, COLOR_PAIR(1));
                mvwprintw(disp_win, i, j * 5 - 1, "0x%0.2X",
                          get_display_buffer()[start_address + 0x10 * (i - 3) +
                                               (j - 2)]);
                wattroff(disp_win, COLOR_PAIR(1));
            } else {
                mvwprintw(disp_win, i, j * 5 - 1, "0x%0.2X",
                          get_display_buffer()[start_address + 0x10 * (i - 3) +
                                               (j - 2)]);
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
        mvwprintw(mem_win, i, 1, "0x%0.4X", start_address + 0x10 * (i - 3));
        for (int j = 2; j < WIDTH / 5; j++) {
            if (privileged_get_memory_byte(start_address + 0x10 * (i - 3) +
                                           (j - 2))) {
                wattron(mem_win, COLOR_PAIR(1));
                mvwprintw(mem_win, i, j * 5 - 1, "0x%0.2X",
                          privileged_get_memory_byte(start_address +
                                                     0x10 * (i - 3) + (j - 2)));
                wattroff(mem_win, COLOR_PAIR(1));
            } else {
                mvwprintw(mem_win, i, j * 5 - 1, "0x%0.2X",
                          privileged_get_memory_byte(start_address +
                                                     0x10 * (i - 3) + (j - 2)));
            }
        }
    }
    wrefresh(mem_win);
}

static void print_stack_window(WINDOW *stack_win) {
    uint16_t WIDTH = 0;
    uint16_t HEIGHT = 0;
    werase(stack_win);
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

    for (uint32_t i = get_sp(); i <= get_base_sp(); i += 2) {
        if (i == get_sp()) {
            mvwprintwhcenter(stack_win, 4 + get_base_sp() - i / 2, 0, WIDTH / 2,
                             "0x%X", i);
            mvwprintwhcenter(stack_win, 4 + get_base_sp() - i / 2, WIDTH / 2,
                             WIDTH / 2, "0x%0.2X 0x%0.2X", get_memory_byte(i),
                             get_memory_byte(i + 1));
            continue;
        }
        mvwprintwhcenter(stack_win, 4 + get_base_sp() - i / 2, 0, WIDTH / 2,
                         "0x%X", i);
        mvwprintwhcenter(stack_win, 4 + get_base_sp() - i / 2, WIDTH / 2,
                         WIDTH / 2, "0x%0.2X 0x%0.2X", get_memory_byte(i),
                         get_memory_byte(i + 1));
    }

    wrefresh(stack_win);
}

static void print_ppu_window(WINDOW *ppu_win) {
    int WIDTH = getmaxx(ppu_win);

    box(ppu_win, 0, 0);

    mvwprintwhcenter(ppu_win, 0, 0, WIDTH, "PPU");
    mvwprintwhcenter(ppu_win, 1, 0, WIDTH / 2, "WX: ");
    mvwprintwhcenter(ppu_win, 1, WIDTH / 2, WIDTH / 2, "%3d",
                     get_memory_byte(WX));
    mvwprintwhcenter(ppu_win, 2, 0, WIDTH / 2, "WY: ");
    mvwprintwhcenter(ppu_win, 2, WIDTH / 2, WIDTH / 2, "%3d",
                     get_memory_byte(WY));

    mvwprintwhcenter(ppu_win, 3, 0, WIDTH / 2, "SCX: ");
    mvwprintwhcenter(ppu_win, 3, WIDTH / 2, WIDTH / 2, "%3d",
                     get_memory_byte(SCX));
    mvwprintwhcenter(ppu_win, 4, 0, WIDTH / 2, "SCY: ");
    mvwprintwhcenter(ppu_win, 4, WIDTH / 2, WIDTH / 2, "%3d",
                     get_memory_byte(SCY));

    mvwprintwhcenter(ppu_win, 5, 0, WIDTH / 2, "X pixel: ");
    mvwprintwhcenter(ppu_win, 5, WIDTH / 2, WIDTH / 2, "%3d", get_x_pixel());
    mvwprintwhcenter(ppu_win, 6, 0, WIDTH / 2, "Y pixel: ");
    mvwprintwhcenter(ppu_win, 6, WIDTH / 2, WIDTH / 2, "%3d", get_y_pixel());
    mvwprintwhcenter(ppu_win, 7, 0, WIDTH / 2, "Window Line: ");
    mvwprintwhcenter(ppu_win, 7, WIDTH / 2, WIDTH / 2, "%3d",
                     get_window_line());

    uint32_t *serviced_interrupts = get_serviced_interrupts();
    mvwprintwhcenter(ppu_win, 8, 0, WIDTH / 2, "VBLANK ");
    mvwprintwhcenter(ppu_win, 8, WIDTH / 2, WIDTH / 2, "%10d",
                     serviced_interrupts[0]);
    mvwprintwhcenter(ppu_win, 9, 0, WIDTH / 2, "LCD ");
    mvwprintwhcenter(ppu_win, 9, WIDTH / 2, WIDTH / 2, "%10d",
                     serviced_interrupts[1]);
    mvwprintwhcenter(ppu_win, 10, 0, WIDTH / 2, "TIMER ");
    mvwprintwhcenter(ppu_win, 10, WIDTH / 2, WIDTH / 2, "%10d",
                     serviced_interrupts[2]);
    mvwprintwhcenter(ppu_win, 11, 0, WIDTH / 2, "JOYPAD ");
    mvwprintwhcenter(ppu_win, 11, WIDTH / 2, WIDTH / 2, "%10d",
                     serviced_interrupts[4]);

    mvwprintwhcenter(ppu_win, 12, 0, WIDTH / 2, "STAT LINE");
    mvwprintwhcenter(ppu_win, 12, WIDTH / 2, WIDTH / 2, "0x%0.2x",
                     get_stat_line());
    mvwprintwhcenter(ppu_win, 13, 0, WIDTH / 2, "STAT REG");
    mvwprintwhcenter(ppu_win, 13, WIDTH / 2, WIDTH / 2, "0x%0.2x",
                     get_memory_byte(STAT));

    uint32_t *serviced_stat_interrupts = get_serviced_stat_interrupts();
    mvwprintwhcenter(ppu_win, 14, 0, WIDTH / 2, "MODE 0 ");
    mvwprintwhcenter(ppu_win, 14, WIDTH / 2, WIDTH / 2, "%10d",
                     serviced_stat_interrupts[0]);
    mvwprintwhcenter(ppu_win, 15, 0, WIDTH / 2, "MODE 1 ");
    mvwprintwhcenter(ppu_win, 15, WIDTH / 2, WIDTH / 2, "%10d",
                     serviced_stat_interrupts[1]);
    mvwprintwhcenter(ppu_win, 16, 0, WIDTH / 2, "MODE 2 ");
    mvwprintwhcenter(ppu_win, 16, WIDTH / 2, WIDTH / 2, "%10d",
                     serviced_stat_interrupts[2]);
    mvwprintwhcenter(ppu_win, 17, 0, WIDTH / 2, "LYC ");
    mvwprintwhcenter(ppu_win, 17, WIDTH / 2, WIDTH / 2, "%10d",
                     serviced_stat_interrupts[3]);

    wrefresh(ppu_win);
}
static void print_cpu_window(WINDOW *cpu_win) {
    int WIDTH = getmaxx(cpu_win);

    box(cpu_win, 0, 0);

    mvwprintwhcenter(cpu_win, 0, 0, WIDTH, "CPU");
    mvwprintwhcenter(cpu_win, 3, 0, WIDTH, "Instruction:");

    mvwprintwhcenter(cpu_win, 4, 0, WIDTH, "0x%0.2x 0x%0.2x 0x%0.2x",
                     get_instruction()[0], get_instruction()[1],
                     get_instruction()[2]);
    mvwprintwhcenter(cpu_win, 6, 0, WIDTH, "Decode:");
    mvwprintwhcenter(cpu_win, 7, 0, WIDTH, "%-22s", "");
    mvwprintwhcenter(cpu_win, 7, 0, WIDTH, "%s", get_decoded_instruction());
    mvwprintwhcenter(cpu_win, 9, 0, WIDTH, "Is Implemented: %-5s",
                     get_is_implemented() ? "True" : "False");
    //    mvwprintwhcenter(cpu_win, 11, 0, WIDTH, "Instruction Count: %"PRIu64,
    //    get_instruction_count()); mvwprintwhcenter(cpu_win, 13, 0, WIDTH,
    //    "%-22s", ""); mvwprintwhcenter(cpu_win, 13, 0, WIDTH, "Available Dots:
    //    %0.5"PRIu64, ppu.available_dots); mvwprintwhcenter(cpu_win, 14, 0,
    //    WIDTH, "Consumed Dots: %0.10"PRIu64, ppu.consumed_dots);
    //    mvwprintwhcenter(cpu_win, 15, 0, WIDTH, "Mode: %"PRIu8,
    //    get_memory_byte(STAT) & 0x03);
    //   mvwprintwhcenter(cpu_win, 16, 0, WIDTH, "Previous Decode:");
    //   mvwprintwhcenter(cpu_win, 17, 0, WIDTH, "%-22s", "");
    //   mvwprintwhcenter(cpu_win, 17, 0, WIDTH, "%s",
    //   get_previous_decoded_instruction());
    mvwprintwhcenter(cpu_win, 14, 0, WIDTH, "Current RTC");
    mvwprintwhcenter(cpu_win, 15, 0, WIDTH, "0x%0.2x 0x%0.2x",
                     get_current_rtc()->minutes, get_current_rtc()->seconds);
    mvwprintwhcenter(cpu_win, 16, 0, WIDTH, "RTC");
    mvwprintwhcenter(cpu_win, 17, 0, WIDTH, "0x%0.2x 0x%0.2x",
                     get_rtc()->minutes, get_rtc()->seconds);

    mvwprintwhcenter(cpu_win, 18, 0, WIDTH, "Latched RTC");
    mvwprintwhcenter(cpu_win, 19, 0, WIDTH, "0x%0.2x 0x%0.2x",
                     get_latched_rtc()->minutes, get_latched_rtc()->seconds);
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
    mvwprintw(registers_win, 3, WIDTH / 2 + 2, "0x%0.2X", get_register(B));

    mvwprintw(registers_win, 4, WIDTH / 4 - 1, "C");
    mvwprintw(registers_win, 4, WIDTH / 2 + 2, "0x%0.2X", get_register(C));

    mvwprintw(registers_win, 5, WIDTH / 4 - 1, "D");
    mvwprintw(registers_win, 5, WIDTH / 2 + 2, "0x%0.2X", get_register(D));

    mvwprintw(registers_win, 6, WIDTH / 4 - 1, "E");
    mvwprintw(registers_win, 6, WIDTH / 2 + 2, "0x%0.2X", get_register(E));

    mvwprintw(registers_win, 7, WIDTH / 4 - 1, "H");
    mvwprintw(registers_win, 7, WIDTH / 2 + 2, "0x%0.2X", get_register(H));

    mvwprintw(registers_win, 8, WIDTH / 4 - 1, "L");
    mvwprintw(registers_win, 8, WIDTH / 2 + 2, "0x%0.2X", get_register(L));

    mvwprintw(registers_win, 9, WIDTH / 4 - 1, "A");
    mvwprintw(registers_win, 9, WIDTH / 2 + 2, "0x%0.2X", get_register(A));

    mvwprintw(registers_win, 10, WIDTH / 4 - 1, "F");
    mvwprintw(registers_win, 10, WIDTH / 2 + 2, "0x%0.2X", get_register(F));

    // Draw PC and SP labels
    mvwprintw(registers_win, 12, WIDTH / 4 - 1, "SP");
    mvwprintw(registers_win, 12, WIDTH / 2 + 2, "0x%0.2X", get_sp());

    mvwprintw(registers_win, 13, WIDTH / 4 - 1, "PC");
    mvwprintw(registers_win, 13, WIDTH / 2 + 2, "0x%0.4X", get_pc());

    mvwprintw(registers_win, 14, WIDTH / 4 - 1, "JOYP");
    mvwprintw(registers_win, 14, WIDTH / 2 + 2, "0x%0.2X", get_joypad_state());

    mvwprintw(registers_win, 15, WIDTH / 4 - 1, "IME");
    mvwprintw(registers_win, 15, WIDTH / 2 + 2, "0x%0.2X", get_ime_flag());
    wrefresh(registers_win);
}

static void print_oam_queue(WINDOW *oam_win) {
    uint16_t WIDTH = 0;
    uint16_t HEIGHT = 0;
    werase(oam_win);
    getmaxyx(oam_win, HEIGHT, WIDTH);
    box(oam_win, 0, 0);

    mvwprintwhcenter(oam_win, 0, 0, WIDTH, "STACK");
    wmove(oam_win, 1, WIDTH / 2);
    wvline(oam_win, 0, HEIGHT - 2);

    // Draw headers
    mvwprintwhcenter(oam_win, 1, 0, WIDTH / 2, "Address");
    mvwprintwhcenter(oam_win, 1, WIDTH / 2, WIDTH / 2, "Value");

    // Draw cell header division
    wmove(oam_win, 2, 1);
    whline(oam_win, 0, WIDTH - 2);
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
