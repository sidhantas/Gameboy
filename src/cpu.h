#include <stdbool.h>
#include <stdint.h>
void *start_cpu(void *);
void end_cpu(void);

extern uint64_t instructions_left;
extern bool step_mode;
