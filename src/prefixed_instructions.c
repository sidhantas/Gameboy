#include "hardware.h"
#include "instructions.h"
#include <ncurses.h>

uint8_t BIT_B_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = (OPCODE & 0x7);
    uint8_t bit = ((OPCODE & 0xF8) >> 3) - 8;

    uint8_t check_bit = hardware.registers[src] & (1 << bit);
    if (check_bit > 0) {}
    check_bit > 0 ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    set_flag(H_FLAG);
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "BIT %d, %c, %d", bit, REGISTER_CHAR(src), check_bit);
    return 8;
}

uint8_t RL_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;
    const uint8_t OPCODE = get_opcode(instruction);
    uint8_t src = OPCODE & 0x0F;
    uint8_t bit7 = hardware.registers[src] & (1 << 7);
    bit7 ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);
    hardware.registers[src] <<= 1;
    hardware.registers[src] == 0 ? set_flag(Z_FLAG) : reset_flag(Z_FLAG);

    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
         "RL %c", REGISTER_CHAR(src));

    return 8;
}
