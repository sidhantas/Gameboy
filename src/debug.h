#include "hardware.h"
#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

void *initialize_debugger();
void refresh_debugger();
void end_debugger();
