#include "hardware.h"
#include <ncurses.h>

void *initialize_debugger(void *arg);
void end_debugger(void);

extern uint16_t mem_win_addr;
extern uint16_t display_buf_addr;
