#include <stdbool.h>
#include <stdint.h>
void *start_cpu(void *);
void end_cpu(void);
void toggle_step_mode(void);
bool get_step_mode(void);

extern uint64_t instructions_left;
