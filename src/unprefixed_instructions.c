#include "hardware.h"
#include "instructions.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

uint8_t LD_RR(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;

    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0x7;
    reg_t dst = ((OPCODE & 0xF8) >> 3) - 8;

    hardware.registers[dst] = hardware.registers[src];
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "LD %c %c", REGISTER_CHAR(dst), REGISTER_CHAR(src));

    return FOUR_CLOCKS;
}

uint8_t XOR_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0x7;

    hardware.registers[A] ^= hardware.registers[src];
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "XOR A, %c", REGISTER_CHAR(src));
    return FOUR_CLOCKS;
}

uint8_t LD_DEREF_HL_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0xF;
    hardware.memory[get_long_reg(HL)] = hardware.registers[src];
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "LD (HL), %c", REGISTER_CHAR(src));
    return EIGHT_CLOCKS;
}

uint8_t ADD_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0x7;
    hardware.registers[A] = add(hardware.registers[A], hardware.registers[src]);

    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "ADD A, %c", REGISTER_CHAR(src));
    return FOUR_CLOCKS;
}

uint8_t ADD_A_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = true;

    hardware.registers[A] = add(hardware.registers[A], get_memory_byte(get_long_reg(HL)));
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "ADD A, %s", LONG_REGISTER_STR(HL));

    return EIGHT_CLOCKS;
}

uint8_t AND_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = false;
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0x7;
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "AND A, %c", REGISTER_CHAR(src));

    return FOUR_CLOCKS;
}

uint8_t AND_A_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = false;
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "AND A, %s", LONG_REGISTER_STR(HL));

    return EIGHT_CLOCKS;
}

uint8_t LD_R_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t dst = (OPCODE >> 3) & 0xF;
    hardware.registers[dst] = instruction[1];
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "LD %c, 0x%X", REGISTER_CHAR(dst), instruction[1]);

    return EIGHT_CLOCKS;
}

uint8_t INC_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = (OPCODE >> 3) & 0xF;
    hardware.registers[src]++;
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "INC %c", REGISTER_CHAR(src));

    return FOUR_CLOCKS;
}

uint8_t DEC_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = (OPCODE >> 3) & 0xF;
    hardware.registers[src] = sub(hardware.registers[src], 1);

    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "DEC %c", REGISTER_CHAR(src));

    return FOUR_CLOCKS;
}

uint8_t SUB_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0xF;
    hardware.registers[A] = sub(hardware.registers[A], hardware.registers[src]);
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "SUB A, %c", REGISTER_CHAR(src));

    return FOUR_CLOCKS;
}

uint8_t LD_R_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = (OPCODE >> 3) & 0x7;
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "LD %c, (HL)", REGISTER_CHAR(src));

    hardware.registers[src] = get_memory_byte(get_long_reg(HL));

    return EIGHT_CLOCKS;
}

uint8_t ADC_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = false;
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0x7;
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "ADC A, %c", REGISTER_CHAR(src));

    return FOUR_CLOCKS;
}

uint8_t SBC_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = false;
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0x7;
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "SBC A, %c", REGISTER_CHAR(src));

    return FOUR_CLOCKS;
}

uint8_t CP_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = false;
    const uint8_t OPCODE = get_opcode(instruction);
    reg_t src = OPCODE & 0x7;

    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "CP A, %c", REGISTER_CHAR(src));

    return FOUR_CLOCKS;
}

uint8_t LD_A_DEREF_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;
    const uint8_t OPCODE = get_opcode(instruction);
    long_reg_t src = (OPCODE & 0xF0) >> 4;
    hardware.registers[A] = hardware.memory[get_long_reg(src)];
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "LD A, (%s)", LONG_REGISTER_STR(src));

    return EIGHT_CLOCKS;
}

uint8_t LD_A_DEREF_HL_INC(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = false;
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "LD A, (HL+)");

    return EIGHT_CLOCKS;
}

uint8_t LD_A_DEREF_HL_DEC(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = false;
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "LD A, (HL-)");

    return EIGHT_CLOCKS;
}

uint8_t LD_ADDR_LONG_R_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = false;
    const uint8_t OPCODE = get_opcode(instruction);
    uint8_t dst = (OPCODE & 0xF0) >> 4;
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "LD (%s), A", LONG_REGISTER_STR(dst));

    return EIGHT_CLOCKS;
}

uint8_t LD_ADDR_HL_INC_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = true;
    hardware.memory[get_long_reg(HL)] = hardware.registers[A];
    set_long_reg_u16(HL, get_long_reg(HL) + 1);
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "LD (HL+), A");

    return EIGHT_CLOCKS;
}

uint8_t LD_ADDR_HL_DEC_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = true;

    hardware.memory[get_long_reg(HL)] = hardware.registers[A];
    set_long_reg_u16(HL, get_long_reg(HL) - 1);
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "LD (HL-), A");

    return EIGHT_CLOCKS;
}

uint8_t JR_NZ_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = true;
    int8_t jump = *(int8_t *)&instruction[1]; // changes meaning to signed
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "JR NZ, %c0x%X", "+-"[jump < 0], (unsigned)abs(jump));
    if (!get_flag(Z_FLAG)) {
        hardware.pc += jump;
        return TWELVE_CLOCKS;
    }

    return EIGHT_CLOCKS;
}

uint8_t JR_NC_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = false;
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "JR NC, 0x%X", instruction[1]);
    return 0;
}

uint8_t
LD_DEREF_FF00_PLUS_IMM_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = true;
    hardware.memory[JOYP + instruction[1]] = hardware.registers[A];
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "LD (0x%X + 0x%X), A", JOYP, instruction[1]);

    return TWELVE_CLOCKS;
}

uint8_t
LD_DEREF_FF00_PLUS_C_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = true;
    hardware.memory[JOYP + hardware.registers[C]] = hardware.registers[A];

    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "LD (0x%X + C), A", JOYP);

    return EIGHT_CLOCKS;
}

uint8_t LD_LONG_R_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;
    const uint8_t OPCODE = get_opcode(instruction);
    long_reg_t long_reg = (OPCODE & 0xF0) >> 4;

    set_long_reg(long_reg, instruction[1], instruction[2]);
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "LD %s, 0x%X", LONG_REGISTER_STR(long_reg),
             two_u8s_to_u16(instruction[1], instruction[2]));

    return TWELVE_CLOCKS;
}

uint8_t INC_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;
    const uint8_t OPCODE = get_opcode(instruction);
    const long_reg_t long_reg = (OPCODE & 0xF0) >> 4;
    set_long_reg_u16(long_reg, get_long_reg(long_reg) + 1);
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "INC %s", LONG_REGISTER_STR(long_reg));

    return EIGHT_CLOCKS;
}

uint8_t DEC_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = false;
    const uint8_t OPCODE = get_opcode(instruction);
    uint8_t long_reg = (OPCODE & 0xF0) >> 4;

    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "DEC %s", LONG_REGISTER_STR(long_reg));
    return 0;
}

uint8_t PUSH_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;
    const uint8_t OPCODE = get_opcode(instruction);
    long_reg_t long_reg = ((OPCODE & 0xF0) >> 4) - 12;

    set_long_mem(hardware.sp, get_long_reg(long_reg));
    hardware.sp -= 2;
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "PUSH %s", LONG_REGISTER_STR(long_reg));
    return SIXTEEN_CLOCKS;
}

uint8_t POP_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;
    const uint8_t OPCODE = get_opcode(instruction);
    long_reg_t long_reg = ((OPCODE & 0xF0) >> 4) - 12;
    hardware.sp += 2;
    set_long_reg_u16(long_reg,
                     two_u8s_to_u16(hardware.memory[hardware.sp],
                                    hardware.memory[hardware.sp + 1]));
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "POP %s", LONG_REGISTER_STR(long_reg));
    return TWELVE_CLOCKS;
}

uint8_t CP_A_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = true;
    uint8_t res = sub(hardware.registers[A], instruction[1]);
    res < instruction[1] ? set_flag(C_FLAG)
                                           : reset_flag(C_FLAG);
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "CP A, 0x%X", instruction[1]);

    return EIGHT_CLOCKS;
}

uint8_t CALL_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;

    set_long_mem(hardware.sp, hardware.pc);
    hardware.sp -= 2;
    hardware.pc = two_u8s_to_u16(instruction[1], instruction[2]);
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "CALL 0x%X", two_u8s_to_u16(instruction[1], instruction[2]));

    return TWENTY_FOUR_CLOCKS;
}

uint8_t LD_ADDR_IMM_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;
    set_memory_byte(two_u8s_to_u16(instruction[1], instruction[2]),
                    hardware.registers[A]);
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "LD (0x%X), A", two_u8s_to_u16(instruction[1], instruction[2]));
    return SIXTEEN_CLOCKS;
}

uint8_t JR_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;

    int8_t jump = *(int8_t *)&instruction[1]; // changes meaning to signed
    hardware.pc += jump;
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "JR 0x%0.2X", instruction[1]);

    return TWELVE_CLOCKS;
}

uint8_t JR_Z_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;
    int8_t jump = *(int8_t *)&instruction[1]; // changes meaning to signed
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "JR Z, 0x%0.2X", instruction[1]);
    if (get_flag(Z_FLAG)) {
        hardware.pc += jump;
        return TWELVE_CLOCKS;
    }
    return EIGHT_CLOCKS;
}

uint8_t
LD_A_DEREF_FF00_PLUS_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    hardware.is_implemented = true;
    hardware.registers[A] = hardware.memory[JOYP + instruction[1]];
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "LD A, (0x%x + 0x%0.2X)", JOYP, instruction[1]);

    return TWELVE_CLOCKS;
}

uint8_t CP_A_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = true;
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "CP A, (HL)");

    const uint8_t val = get_memory_byte(get_long_reg(HL));

    sub(hardware.registers[A], val);
    return EIGHT_CLOCKS;
}

uint8_t RLA(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = true;
    uint8_t src = A;
    uint8_t bit7 = hardware.registers[src] & (1 << 7);
    hardware.registers[src] <<= 1;
    hardware.registers[src] |= get_flag(C_FLAG);
    bit7 ? set_flag(C_FLAG) : reset_flag(C_FLAG);
    reset_flag(N_FLAG);
    reset_flag(H_FLAG);
    reset_flag(Z_FLAG);
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE, "RLA");
    return FOUR_CLOCKS;
}

uint8_t ADC_A_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = false;
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "ADC A, 0x%X", instruction[1]);
    return 0;
}

uint8_t RET(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = true;
    hardware.sp += 2;
    hardware.pc = two_u8s_to_u16(hardware.memory[hardware.sp],
                                 hardware.memory[hardware.sp + 1]);
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE, "RET");

    return SIXTEEN_CLOCKS;
}

uint8_t RETI(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = false;
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "RETI");
    return 0;
}

uint8_t CALL_Z_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = false;
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "CALL Z, 0x%X", two_u8s_to_u16(instruction[1], instruction[2]));
    return 0;
}

uint8_t LD_ADDR_IMM_SP(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = false;
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
             "LD (0x%X), SP", two_u8s_to_u16(instruction[1], instruction[2]));
    return 0;
}

uint8_t NOP(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    (void)instruction;
    hardware.is_implemented = true;
    snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE, "NOP");
    return 0;
}
