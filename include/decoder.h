#include "hardware.h"
#include "instructions.h"
#include <stdint.h>
clock_cycles_t (*fetch_instruction(void))(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t execute_instruction(clock_cycles_t execute_func(uint8_t instruction[MAX_INSTRUCTION_SIZE]));
