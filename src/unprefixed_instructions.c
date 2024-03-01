#include "hardware.h"
#include "instructions.h"
#include "utils.h"
#include <inttypes.h>
#include <stdlib.h>

clock_cycles_t LD_RR(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0x7;
    reg_t dst = ((OPCODE & 0xF8) >> 3) - 8;

    set_register(dst, get_register(src));

    set_decoded_instruction("LD %c %c", REGISTER_CHAR(dst), REGISTER_CHAR(src));

    return FOUR_CLOCKS;
}

clock_cycles_t XOR_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0x7;
    uint8_t result = get_register(A) ^ get_register(src);
    set_register(A, result);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);
    reset_flag(C_FLAG);
    get_register(A) == 0 ? set_flag(Z_FLAG) : reset_flag(Z_FLAG);
    set_decoded_instruction("XOR A, %c", REGISTER_CHAR(src));
    return FOUR_CLOCKS;
}

clock_cycles_t XOR_A_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    uint8_t result = get_register(A) ^ get_memory_byte(get_long_reg(HL));
    set_register(A, result);
    set_decoded_instruction("XOR A, (HL)");
    return EIGHT_CLOCKS;
}

clock_cycles_t LD_ADDR_HL_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0xF;
    set_memory_byte(get_long_reg(HL), get_register(src));
    set_decoded_instruction("LD (HL), %c", REGISTER_CHAR(src));
    return EIGHT_CLOCKS;
}

clock_cycles_t ADD_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0x7;

    uint8_t result = add(get_register(A), get_register(A), 0);
    set_register(A, result);
    set_decoded_instruction("ADD A, %c", REGISTER_CHAR(src));
    return FOUR_CLOCKS;
}

clock_cycles_t ADD_A_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    uint8_t result = add(get_register(A), get_memory_byte(get_long_reg(HL)), 0);
    set_register(A, result);
    set_decoded_instruction("ADD A, %s", LONG_REGISTER_STR(HL));

    return EIGHT_CLOCKS;
}

clock_cycles_t AND_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0x7;

    uint8_t result = get_register(A) & get_register(src);
    set_register(A, result);
    result ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    set_flag(H_FLAG);
    reset_flag(C_FLAG);

    set_decoded_instruction("AND A, %c", REGISTER_CHAR(src));

    return FOUR_CLOCKS;
}

clock_cycles_t AND_A_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("AND A, %s", LONG_REGISTER_STR(HL));

    return EIGHT_CLOCKS;
}

clock_cycles_t LD_R_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    reg_t dst = (OPCODE >> 3) & 0xF;
    set_register(dst, instruction[1]);
    set_decoded_instruction("LD %c, 0x%X", REGISTER_CHAR(dst), instruction[1]);

    return EIGHT_CLOCKS;
}

clock_cycles_t INC_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = (OPCODE >> 3) & 0xF;
    set_register(src, add(get_register(src), 1, 0));
    set_decoded_instruction("INC %c", REGISTER_CHAR(src));

    return FOUR_CLOCKS;
}

clock_cycles_t DEC_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = (OPCODE >> 3) & 0xF;
    uint8_t result = sub(get_register(src), 1);
    set_register(src, result);

    set_decoded_instruction("DEC %c", REGISTER_CHAR(src));

    return FOUR_CLOCKS;
}

clock_cycles_t SUB_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0xF;
    uint8_t result = sub(get_register(A), get_register(src));
    set_register(A, result);
    set_decoded_instruction("SUB A, %c", REGISTER_CHAR(src));

    return FOUR_CLOCKS;
}

clock_cycles_t LD_R_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = (OPCODE >> 3) & 0x7;
    set_register(src, get_memory_byte(get_long_reg(HL)));

    set_decoded_instruction("LD %c, (HL)", REGISTER_CHAR(src));
    return EIGHT_CLOCKS;
}

clock_cycles_t ADC_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0x7;
    set_decoded_instruction("ADC A, %c", REGISTER_CHAR(src));

    return -1;
}

clock_cycles_t SBC_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0x7;
    set_decoded_instruction("SBC A, %c", REGISTER_CHAR(src));

    return -1;
}

clock_cycles_t CP_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0x7;

    set_decoded_instruction("CP A, %c", REGISTER_CHAR(src));

    return -1;
}

clock_cycles_t LD_A_DEREF_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    long_reg_t src = (OPCODE & 0xF0) >> 4;
    set_register(A, get_memory_byte(get_long_reg(src)));
    set_decoded_instruction("LD A, (%s)", LONG_REGISTER_STR(src));

    return EIGHT_CLOCKS;
}

clock_cycles_t LD_A_DEREF_HL_INC(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    const uint16_t HL_val = get_long_reg(HL);
    set_register(A, get_memory_byte(HL_val));
    set_long_reg_u16(HL, HL_val + 1);

    set_decoded_instruction("LD A, (HL+)");

    return EIGHT_CLOCKS;
}

clock_cycles_t LD_A_DEREF_HL_DEC(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("LD A, (HL-)");

    return -1;
}

clock_cycles_t LD_ADDR_LONG_R_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    long_reg_t dst = (OPCODE & 0xF0) >> 4;
    set_memory_byte(get_long_reg(dst), get_register(A));
    set_decoded_instruction("LD (%s), A", LONG_REGISTER_STR(dst));

    return EIGHT_CLOCKS;
}

clock_cycles_t LD_ADDR_HL_INC_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_memory_byte(get_long_reg(HL), get_register(A));
    set_long_reg_u16(HL, get_long_reg(HL) + 1);
    set_decoded_instruction("LD (HL+), A");

    return EIGHT_CLOCKS;
}

clock_cycles_t LD_ADDR_HL_DEC_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_memory_byte(get_long_reg(HL), get_register(A));
    set_long_reg_u16(HL, get_long_reg(HL) - 1);
    set_decoded_instruction("LD (HL-), A");

    return EIGHT_CLOCKS;
}

clock_cycles_t JR_NZ_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    int8_t jump = *(int8_t *)&instruction[1]; // changes meaning to signed
    set_decoded_instruction("JR NZ, %c0x%X", "+-"[jump < 0],
                            (unsigned)abs(jump));
    if (!get_flag(Z_FLAG)) {
        set_pc(get_pc() + jump);
        return TWELVE_CLOCKS;
    }

    return EIGHT_CLOCKS;
}

clock_cycles_t JR_NC_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("JR NC, 0x%X", instruction[1]);
    return -1;
}

clock_cycles_t
LD_ADDR_FF00_PLUS_IMM_REGISTER_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_memory_byte(JOYP + instruction[1], get_register(A));
    set_decoded_instruction("LD (0x%X + 0x%X), A", JOYP, instruction[1]);

    return TWELVE_CLOCKS;
}

clock_cycles_t
LD_DEREF_FF00_PLUS_C_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_memory_byte(JOYP + get_register(C), get_register(A));

    set_decoded_instruction("LD (0x%X + C), A", JOYP);

    return EIGHT_CLOCKS;
}

clock_cycles_t LD_LONG_R_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    long_reg_t long_reg = (OPCODE & 0xF0) >> 4;

    set_long_reg(long_reg, instruction[1], instruction[2]);

    set_decoded_instruction("LD %s, 0x%X", LONG_REGISTER_STR(long_reg),
                            two_u8s_to_u16(instruction[1], instruction[2]));
    return TWELVE_CLOCKS;
}

clock_cycles_t INC_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    const long_reg_t long_reg = (OPCODE & 0xF0) >> 4;
    set_long_reg_u16(long_reg, get_long_reg(long_reg) + 1);
    set_decoded_instruction("INC %s", LONG_REGISTER_STR(long_reg));

    return EIGHT_CLOCKS;
}

clock_cycles_t DEC_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    const long_reg_t long_reg = (OPCODE & 0xF0) >> 4;

    set_long_reg_u16(long_reg, get_long_reg(BC) - 1);

    set_decoded_instruction("DEC %s", LONG_REGISTER_STR(long_reg));
    return EIGHT_CLOCKS;
}

clock_cycles_t PUSH_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    long_reg_t long_reg = ((OPCODE & 0xF0) >> 4) - 12;

    set_long_mem(get_sp(), get_long_reg(long_reg));
    set_sp(get_sp() - 2);
    set_decoded_instruction("PUSH %s", LONG_REGISTER_STR(long_reg));
    return SIXTEEN_CLOCKS;
}

clock_cycles_t POP_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint8_t OPCODE = get_opcode(instruction);
    long_reg_t long_reg = ((OPCODE & 0xF0) >> 4) - 12;
    set_sp(get_sp() + 2);
    set_long_reg_u16(long_reg, two_u8s_to_u16(get_memory_byte(get_sp()),
                                              get_memory_byte(get_sp() + 1)));
    set_decoded_instruction("POP %s", LONG_REGISTER_STR(long_reg));
    return TWELVE_CLOCKS;
}

clock_cycles_t CP_A_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    sub(get_register(A), instruction[1]);
    set_decoded_instruction("CP A, 0x%X", instruction[1]);

    return EIGHT_CLOCKS;
}

clock_cycles_t CALL_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    set_long_mem(get_sp(), get_pc());
    set_sp(get_sp() - 2);
    set_pc(two_u8s_to_u16(instruction[1], instruction[2]));
    set_decoded_instruction("CALL 0x%X",
                            two_u8s_to_u16(instruction[1], instruction[2]));

    return TWENTY_FOUR_CLOCKS;
}

clock_cycles_t LD_ADDR_IMM_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    set_memory_byte(two_u8s_to_u16(instruction[1], instruction[2]),
                    get_register(A));
    set_decoded_instruction("LD (0x%X), A",
                            two_u8s_to_u16(instruction[1], instruction[2]));
    return SIXTEEN_CLOCKS;
}

clock_cycles_t JR_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    int8_t jump = uint8_to_int8(instruction[1]);
    set_pc(get_pc() + jump);
    set_decoded_instruction("JR 0x%0.2X", instruction[1]);

    return TWELVE_CLOCKS;
}

clock_cycles_t JR_Z_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    int8_t jump = uint8_to_int8(instruction[1]);
    set_decoded_instruction("JR Z, 0x%0.2X", instruction[1]);
    if (get_flag(Z_FLAG)) {
        set_pc(get_pc() + jump);
        return TWELVE_CLOCKS;
    }
    return EIGHT_CLOCKS;
}

clock_cycles_t
LD_A_DEREF_FF00_PLUS_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    set_register(A, get_memory_byte(JOYP + instruction[1]));
    set_decoded_instruction("LD A, (0x%x + 0x%0.2X)", JOYP, instruction[1]);

    return TWELVE_CLOCKS;
}

clock_cycles_t CP_A_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    const uint8_t val = get_memory_byte(get_long_reg(HL));

    sub(get_register(A), val);
    set_decoded_instruction("CP A, (HL)");
    return EIGHT_CLOCKS;
}

clock_cycles_t RLA(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    reg_t src = A;
    uint8_t bit7 = get_bit(get_register(src), 7);
    uint8_t result = get_register(src) << 1 | get_flag(C_FLAG);
    set_register(src, result);
    bit7 ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);
    reset_flag(Z_FLAG);
    set_decoded_instruction("RLA");
    return FOUR_CLOCKS;
}

clock_cycles_t ADC_A_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    uint8_t imm = instruction[1];

    set_register(A, add(get_register(A), imm, get_flag(C_FLAG)));

    set_decoded_instruction("ADC A, 0x%X", instruction[1]);
    return -1;
}

clock_cycles_t RET(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_sp(get_sp() + 2);
    set_pc(two_u8s_to_u16(get_memory_byte(get_sp()),
                          get_memory_byte(get_sp() + 1)));
    set_decoded_instruction("RET");

    return SIXTEEN_CLOCKS;
}

clock_cycles_t RETI(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("RETI");
    return -1;
}

clock_cycles_t CALL_Z_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("CALL Z, 0x%X",
                            two_u8s_to_u16(instruction[1], instruction[2]));
    return -1;
}

clock_cycles_t LD_ADDR_IMM_SP(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("LD (0x%X), SP",
                            two_u8s_to_u16(instruction[1], instruction[2]));
    return -1;
}

clock_cycles_t JP_NZ_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    uint16_t addr = two_u8s_to_u16(instruction[1], instruction[2]);

    if (!get_flag(Z_FLAG)) {
        set_pc(addr);
    }
    set_decoded_instruction("JP NZ 0x%X", addr);
    return TWELVE_CLOCKS;
}
clock_cycles_t RLCA(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("RLCA");

    get_bit(get_register(A), 7) ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    uint8_t result = get_register(A) << 1;
    set_register(A, result);
    return -1;
}

clock_cycles_t RRCA(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("RRCA");
    get_bit(get_register(A), 0) ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    uint8_t result = get_register(A) >> 1;
    set_register(A, result);
    return -1;
}

clock_cycles_t STOP(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("STOP");

    return -1;
}

clock_cycles_t ADD_HL_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);
    long_reg_t dst = HL;
    long_reg_t src = ((OPCODE & 0xF0) >> 4);

    set_decoded_instruction("ADD HL, %s", LONG_REGISTER_STR(src));

    set_long_reg_u16(HL, addu16(dst, src));

    return EIGHT_CLOCKS;
}

clock_cycles_t RRA(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("RRA");

    return -1;
}

clock_cycles_t DAA(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("DAA");

    return -1;
}

clock_cycles_t CPL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_register(A, ~get_register(A));
    set_flag(N_FLAG);
    set_flag(H_FLAG);
    set_decoded_instruction("CPL");

    return FOUR_CLOCKS;
}

clock_cycles_t INC_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("INC (HL)");

    return -1;
}

clock_cycles_t DEC_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("DEC (HL)");
    const uint16_t HL_val = get_long_reg(HL);
    set_memory_byte(HL_val, sub(get_memory_byte(HL), 1));

    return TWELVE_CLOCKS;
}

clock_cycles_t LD_ADDR_HL_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    uint8_t imm = instruction[1];
    uint16_t addr = get_long_reg(HL);

    set_memory_byte(addr, imm);
    set_decoded_instruction("LD (HL), 0x%X", imm);
    return TWELVE_CLOCKS;
}

clock_cycles_t SCF(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("SCF");

    return -1;
}

clock_cycles_t JR_C_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    uint8_t imm = instruction[1];
    set_decoded_instruction("JR C 0x%X", imm);

    return -1;
}

clock_cycles_t CCF(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("CCF");

    return -1;
}

clock_cycles_t HALT(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("HALT");

    return -1;
}

clock_cycles_t ADC_A_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("ADC A, (HL)");

    return -1;
}

clock_cycles_t SUB_A_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("SUB A, (HL)");

    return -1;
}

clock_cycles_t SBC_A_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("SBC A, (HL)");

    return -1;
}

clock_cycles_t OR_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = get_opcode(instruction);
    const reg_t src = OPCODE & 0x7;

    uint8_t result = get_register(A) ^ get_register(src);
    set_register(A, result);
    result ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);
    reset_flag(C_FLAG);

    set_decoded_instruction("OR A, %c", REGISTER_CHAR(src));

    return FOUR_CLOCKS;
}

clock_cycles_t OR_A_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("OR A, (HL)");

    return -1;
}

clock_cycles_t RET_NZ(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("RET NZ");

    return -1;
}

clock_cycles_t CALL_NZ_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    uint8_t imm = instruction[1];
    set_decoded_instruction("CALL NZ, 0x%X", imm);

    return -1;
}

clock_cycles_t ADD_A_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    uint8_t imm = instruction[1];
    set_decoded_instruction("ADD A, 0x%X", imm);

    return -1;
}

clock_cycles_t RST_x0h(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    const uint8_t OPCODE = get_opcode(instruction);
    const uint8_t x = (OPCODE & 0x30) >> 4;

    set_decoded_instruction("RST %" PRIu8 "0h", x);

    return -1;
}

clock_cycles_t RET_Z(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("RET Z");

    if (get_flag(Z_FLAG)) {
        set_sp(get_sp() + 2);
        set_pc(two_u8s_to_u16(get_memory_byte(get_sp()),
                              get_memory_byte(get_sp() + 1)));
        return TWENTY_CLOCKS;
    }

    return FOUR_CLOCKS;
}

clock_cycles_t RET_C(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("RET C");

    return -1;
}

clock_cycles_t JP_Z_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint16_t imm = two_u8s_to_u16(instruction[1], instruction[2]);
    set_decoded_instruction("JP Z, 0x%X", imm);
    if (get_flag(Z_FLAG)) {
        set_pc(imm);
        return -1;
    }
    return -1;
}

clock_cycles_t JP_C_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint16_t imm = two_u8s_to_u16(instruction[1], instruction[2]);
    set_decoded_instruction("JP C, 0x%X", imm);

    return -1;
}

clock_cycles_t RST_x8h(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    const uint8_t OPCODE = get_opcode(instruction);
    uint16_t x = (OPCODE & 0x30) | 0x008;
    set_long_mem(get_sp(), get_pc());
    set_sp(get_sp() - 2);
    set_pc(x);

    set_decoded_instruction("RST %xh", x);
    return THIRTY_TWO_CLOCKS;
}

clock_cycles_t RET_NC(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("RET NC");

    return -1;
}

clock_cycles_t CALL_NC_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    const uint8_t imm = two_u8s_to_u16(instruction[1], instruction[2]);
    set_decoded_instruction("CALL NC 0x%X");

    return -1;
}

clock_cycles_t SUB_A_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    const uint8_t imm = instruction[1];
    set_decoded_instruction("SUB A, 0x%X", imm);

    return -1;
}

clock_cycles_t CALL_C_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    const uint8_t imm = two_u8s_to_u16(instruction[1], instruction[2]);

    set_decoded_instruction("CALL C, 0x%X", imm);

    return -1;
}

clock_cycles_t SBC_A_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    const uint8_t imm = instruction[1];
    set_decoded_instruction("SBC A, 0x%X", imm);

    return -1;
}

clock_cycles_t AND_A_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    const uint8_t imm = instruction[1];
    uint8_t result = get_register(A) & imm;
    set_register(A, result);
    result ? reset_flag(Z_FLAG) : set_flag(Z_FLAG);
    reset_flag(N_FLAG);
    set_flag(H_FLAG);
    reset_flag(C_FLAG);

    set_decoded_instruction("AND A, 0x%X", imm);

    return EIGHT_CLOCKS;
}

clock_cycles_t ADD_SP_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    const uint8_t imm = instruction[1];
    set_decoded_instruction("ADD SP, 0x%X", imm);

    return -1;
}

clock_cycles_t JP_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_pc(get_long_reg(HL));
    set_decoded_instruction("JP HL");

    return FOUR_CLOCKS;
}

clock_cycles_t XOR_A_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    const uint8_t imm = instruction[1];
    set_decoded_instruction("XOR A, 0x%X", imm);

    return -1;
}

clock_cycles_t
LD_A_DEREF_FF00_PLUS_C(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("LD A, (FF00 + C)");

    return -1;
}

clock_cycles_t DI(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_interrupts(false);
    set_decoded_instruction("DI");

    return FOUR_CLOCKS;
}

clock_cycles_t OR_A_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    uint8_t imm = instruction[1];
    set_decoded_instruction("OR A, 0x%X", imm);

    return -1;
}

clock_cycles_t LD_HL_SP_PLUS_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    int8_t imm = *(int8_t *)&instruction[1]; // changes meaning to signed
    set_decoded_instruction("LD HL, SP + %#X", imm);

    return -1;
}

clock_cycles_t LD_SP_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("LD SP, HL");

    return -1;
}

clock_cycles_t LD_A_DEREF_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    uint16_t imm = two_u8s_to_u16(instruction[1], instruction[2]);
    set_register(A, get_memory_byte(imm));
    set_decoded_instruction("LD A, (0x%X)", imm);

    return SIXTEEN_CLOCKS;
}

clock_cycles_t EI(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_interrupts(true);
    set_decoded_instruction("EI");

    return FOUR_CLOCKS;
}

clock_cycles_t JP_NC_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    uint16_t imm = two_u8s_to_u16(instruction[1], instruction[2]);
    set_decoded_instruction("JP NC, 0x%X", imm);

    return -1;
}

clock_cycles_t JP_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {

    const uint16_t imm = two_u8s_to_u16(instruction[1], instruction[2]);
    set_pc(imm);

    set_decoded_instruction("JP 0x%X", imm);
    return TWELVE_CLOCKS;
}

clock_cycles_t NOP(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("NOP");
    return 0;
}

clock_cycles_t UNK(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;

    set_decoded_instruction("UNK");
    return 0;
}
