#include "hardware.h"
#include "instructions.h"
#include "utils.h"
#include <ncurses.h>

clock_cycles_t BIT_B_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = (OPCODE & 0x7);
    uint8_t bit = ((OPCODE & 0xF8) >> 3) - 8;

    uint8_t check_bit = get_bit(get_register(src), bit);
    check_bit > 0 ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    set_flag(H_FLAG);
    set_decoded_instruction("BIT %d, %c, %d", bit, REGISTER_CHAR(src),
                            check_bit);
    return EIGHT_CLOCKS;
}

clock_cycles_t RL_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0x0F;
    uint8_t bit7 = get_bit(get_register(src), 7);
    bit7 ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);
    uint8_t result = get_register(src) << 1;
    set_register(src, result);
    result ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);

    set_decoded_instruction("RL %c", REGISTER_CHAR(src));

    return EIGHT_CLOCKS;
}

clock_cycles_t RLC_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t RLC_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t RRC_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t RRC_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t RL_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t RR_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t RR_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t SLA_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t SLA_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t SRA_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t SRA_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t SWAP_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t SWAP_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t SRL_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t SRL_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t BIT_B_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t RES_B_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t RES_B_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t SET_B_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}

clock_cycles_t SET_B_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    return -1;

}


