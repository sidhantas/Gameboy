#pragma once
#include "hardware.h"
static inline uint8_t get_opcode(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    return instruction[0] == 0xCB ? instruction[1] : instruction[0];
}

typedef enum {
    ZERO_CLOCKS = 0,
    FOUR_CLOCKS = 4,
    EIGHT_CLOCKS = 8,
    TWELVE_CLOCKS = 12,
    SIXTEEN_CLOCKS = 16,
    TWENTY_FOUR_CLOCKS = 24
} clock_cycles_t;
/* UNPREFIXED INSTRUCTIONS */
clock_cycles_t LD_RR(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t XOR_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t XOR_A_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t LD_ADDR_HL_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t ADD_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t ADD_A_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t AND_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t AND_A_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t LD_R_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t INC_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t DEC_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t SUB_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t LD_R_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t ADC_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t SBC_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t CP_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t LD_A_DEREF_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t LD_A_DEREF_HL_INC(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t LD_A_DEREF_HL_DEC(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t LD_ADDR_LONG_R_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t LD_ADDR_HL_INC_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t LD_ADDR_HL_DEC_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t JR_NZ_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t JR_NC_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t LD_IO_REGISTER_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t LD_DEREF_FF00_PLUS_C_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t LD_LONG_R_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t INC_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t DEC_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t PUSH_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t POP_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t CP_A_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t CALL_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t LD_ADDR_IMM_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t JR_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t JR_Z_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t LD_A_DEREF_FF00_PLUS_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t CP_A_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t RLA(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t ADC_A_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t RET(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t RETI(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t CALL_Z_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t LD_ADDR_IMM_SP(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t JP_NZ_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t NOP(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t UNK(uint8_t instruction[MAX_INSTRUCTION_SIZE]);

/* PREFIXED INSTRUCTIONS */
clock_cycles_t BIT_B_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
clock_cycles_t RL_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
