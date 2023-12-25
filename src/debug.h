#include "hardware.h"
#include <ncurses.h>
#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

void *initialize_debugger();
void refresh_debugger(WINDOW *, WINDOW *, WINDOW *, WINDOW *, WINDOW *);
void end_debugger();
