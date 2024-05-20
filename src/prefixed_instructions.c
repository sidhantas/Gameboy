#include "hardware.h"
#include "instructions.h"
#include "utils.h"
#include <inttypes.h>
#include <ncurses.h>

clock_cycles_t BIT_B_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = (OPCODE & 0x7);
    uint8_t bit = (OPCODE >> 3) & 0x7;

    uint8_t check_bit = get_bit(get_register(src), bit);
    check_bit ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
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
    uint8_t result = get_register(src) << 1 | get_flag(C_FLAG);

    bit7 ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);
    result ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);

    set_register(src, result);

    set_decoded_instruction("RL %c", REGISTER_CHAR(src));

    return EIGHT_CLOCKS;
}

clock_cycles_t RLC_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0x07;

    const uint8_t bit7 = get_bit(get_register(src), 7);
    uint8_t result = get_register(src) << 1 | bit7;

    bit7 ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    result ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);

    set_register(src, result);

    set_decoded_instruction("RLC %c", REGISTER_CHAR(src));
    return EIGHT_CLOCKS;
}

clock_cycles_t RLC_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    uint8_t HL_val = get_memory_byte(get_long_reg(HL));

    const uint8_t bit7 = get_bit(HL_val, 7);
    uint8_t result = HL_val << 1 | bit7;

    bit7 ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    result ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);

    set_memory_byte(get_long_reg(HL), result);

    return SIXTEEN_CLOCKS;
}

clock_cycles_t RRC_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0x07;
    const uint8_t bit0 = get_bit(get_register(src), 0);
    uint8_t result = get_register(src) >> 1 | bit0 << 7;

    bit0 ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    result ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);

    set_register(src, result);

    set_decoded_instruction("RRC %c", REGISTER_CHAR(src));
    return EIGHT_CLOCKS;
}

clock_cycles_t RRC_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    uint8_t HL_val = get_memory_byte(get_long_reg(HL));

    const uint8_t bit0 = get_bit(HL_val, 0);
    uint8_t result = HL_val >> 1 | bit0 << 7;

    bit0 ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    result ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);

    set_memory_byte(get_long_reg(HL), result);

    set_decoded_instruction("RRC (HL)");

    return SIXTEEN_CLOCKS;
}

clock_cycles_t RL_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    uint8_t HL_val = get_memory_byte(get_long_reg(HL));
    uint8_t bit7 = get_bit(HL_val, 7);
    uint8_t result = HL_val << 1 | get_flag(C_FLAG);

    bit7 ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);
    result ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);

    set_memory_byte(get_long_reg(HL), result);

    return SIXTEEN_CLOCKS;
}

clock_cycles_t RR_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    reg_t src = OPCODE & 0x07;
    uint8_t result = get_register(src) >> 1 | get_flag(C_FLAG) << 7;
    get_register(src) & 0x01 ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    result ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);

    set_register(src, result);
    set_decoded_instruction("RR %c", REGISTER_CHAR(src));
    return EIGHT_CLOCKS;
}

clock_cycles_t RR_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    uint8_t HL_val = get_memory_byte(get_long_reg(HL));
    uint8_t result = HL_val >> 1 | get_flag(C_FLAG) << 7;
    HL_val & 0x01 ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    result ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);

    set_memory_byte(get_long_reg(HL), result);
    return SIXTEEN_CLOCKS;
}

clock_cycles_t SLA_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);

    reg_t src = OPCODE & 0x07;
    uint8_t result = get_register(src) << 1;
    get_bit(get_register(src), 7) ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    result ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);

    set_register(src, result);
    set_decoded_instruction("SLA %c", REGISTER_CHAR(src));
    return EIGHT_CLOCKS;
}

clock_cycles_t SLA_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    uint8_t HL_val = get_memory_byte(get_long_reg(HL));
    uint8_t result = HL_val << 1;
    get_bit(HL_val, 7) ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    result ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);

    set_memory_byte(get_long_reg(HL), result);

    return SIXTEEN_CLOCKS;
}

clock_cycles_t SRA_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0x07;
    uint8_t result = (get_register(src) >> 1) | (get_register(src) & 0x80);
    get_bit(get_register(src), 0) ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    result ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);

    set_register(src, result);
    set_decoded_instruction("SRA %c", REGISTER_CHAR(src));
    return EIGHT_CLOCKS;
}

clock_cycles_t SRA_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    uint8_t HL_val = get_memory_byte(get_long_reg(HL));
    uint8_t result = (HL_val >> 1) | (HL_val & 0x80);
    get_bit(HL_val, 0) ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    result ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);

    set_memory_byte(get_long_reg(HL), result);

    return SIXTEEN_CLOCKS;
}

clock_cycles_t SWAP_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);
    const reg_t src = OPCODE & 0x07;

    uint8_t reg_val = get_register(src);
    uint8_t new_val = (reg_val & 0x0F) << 4 | (reg_val & 0xF0) >> 4;
    set_register(src, new_val);
    new_val ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);
    reset_flag(C_FLAG);
    set_decoded_instruction("SWAP %c", REGISTER_CHAR(src));
    return EIGHT_CLOCKS;
}

clock_cycles_t SWAP_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    uint8_t HL_val = get_memory_byte(get_long_reg(HL));
    uint8_t reg_val = HL_val;
    uint8_t new_val = (reg_val & 0x0F) << 4 | (reg_val & 0xF0) >> 4;
    new_val ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);
    reset_flag(C_FLAG);

    set_memory_byte(get_long_reg(HL), new_val);
    return SIXTEEN_CLOCKS;
}

clock_cycles_t SRL_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);
    const reg_t src = OPCODE & 0x07;

    uint8_t reg_val = get_register(src);
    uint8_t bit0 = reg_val & 0x01;
    uint8_t result = reg_val >> 1;
    result ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);
    bit0 ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    set_register(src, result);
    set_decoded_instruction("SRL %c", REGISTER_CHAR(src));
    return EIGHT_CLOCKS;
}

clock_cycles_t SRL_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    uint8_t HL_val = get_memory_byte(get_long_reg(HL));
    uint8_t bit0 = HL_val & 0x01;
    uint8_t result = HL_val >> 1;
    result ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);
    bit0 ? set_flag(C_FLAG) : reset_flag(C_FLAG);

    set_memory_byte(get_long_reg(HL), result);
    return SIXTEEN_CLOCKS;
}

clock_cycles_t BIT_B_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);
    uint8_t bit = (OPCODE >> 3) & 0x7;

    uint8_t check_bit = get_bit(get_memory_byte(get_long_reg(HL)), bit);
    check_bit ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    set_flag(H_FLAG);
    set_decoded_instruction("BIT %d, (HL), %d", bit, check_bit);

    return TWELVE_CLOCKS;
}

clock_cycles_t RES_B_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);
    const reg_t src = OPCODE & 0x07;
    uint8_t bit = (OPCODE >> 3) & 0x7;
    uint8_t reg_val = get_register(src);
    set_register(src, reg_val & ~(1 << bit));

    set_decoded_instruction("RES %" PRIu8 " %c", bit, REGISTER_CHAR(src));
    return EIGHT_CLOCKS;
}

clock_cycles_t RES_B_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);
    uint8_t HL_val = get_memory_byte(get_long_reg(HL));
    uint8_t bit = (OPCODE >> 3) & 0x7;
    set_memory_byte(get_long_reg(HL), HL_val & ~(1 << bit));

    return SIXTEEN_CLOCKS;
}

clock_cycles_t SET_B_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);
    const reg_t src = OPCODE & 0x07;
    uint8_t bit = (OPCODE >> 3) & 0x7;
    uint8_t reg_val = get_register(src);
    set_register(src, reg_val | 1 << bit);

    set_decoded_instruction("SET %" PRIu8 " %c", bit, REGISTER_CHAR(src));
    return EIGHT_CLOCKS;
}

clock_cycles_t SET_B_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);
    uint8_t bit = (OPCODE >> 3) & 0x7;
    uint8_t HL_val = get_memory_byte(get_long_reg(HL));
    set_memory_byte(get_long_reg(HL), HL_val | 1 << bit);

    return SIXTEEN_CLOCKS;
}
