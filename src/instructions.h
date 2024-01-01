#pragma once
#include "hardware.h"
static inline uint8_t
get_opcode(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    return instruction[0] == 0xCB ? instruction[1] : instruction[0];
}
/* UNPREFIXED INSTRUCTIONS */
uint8_t LD_RR(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t XOR_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t LD_DEREF_HL_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t ADD_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t ADD_A_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t AND_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t AND_A_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t LD_R_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t INC_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t DEC_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t SUB_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t LD_R_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t ADC_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t SBC_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t CP_A_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t LD_A_DEREF_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t LD_A_DEREF_HL_INC(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t LD_A_DEREF_HL_DEC(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t LD_ADDR_LONG_R_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t LD_ADDR_HL_INC_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t LD_ADDR_HL_DEC_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t JR_NZ_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t JR_NC_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t LD_DEREF_FF00_PLUS_IMM_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t LD_DEREF_FF00_PLUS_C_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t LD_LONG_R_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t INC_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t DEC_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t PUSH_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t POP_LONG_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t CP_A_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t CALL_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t LD_ADDR_IMM_A(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t JR_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t JR_Z_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t LD_A_DEREF_FF00_PLUS_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t CP_A_DEREF_HL(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t RLA(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t ADC_A_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t RET(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t RETI(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t CALL_Z_IMM(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t LD_ADDR_IMM_SP(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t NOP(uint8_t instruction[MAX_INSTRUCTION_SIZE]);

/* PREFIXED INSTRUCTIONS */
uint8_t BIT_B_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
uint8_t RL_R(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
