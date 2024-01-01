#include "hardware.h"
#include <stdint.h>
uint8_t (*fetch_instruction(void))(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t execute_instruction(uint8_t execute_func(uint8_t instruction[MAX_INSTRUCTION_SIZE]));
