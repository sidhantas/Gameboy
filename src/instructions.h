#pragma once
#include "hardware.h"
static inline uint8_t
get_opcode(const uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    return instruction[0] == 0xCB ? instruction[1] : instruction[0];
}
/* UNPREFIXED INSTRUCTIONS */
void LD_RR(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void XOR_A_R(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void LD_DEREF_HL_R(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void ADD_A_R(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void AND_A_R(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void LD_R_IMM(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void INC_R(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void DEC_R(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void SUB_A_R(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void LD_R_DEREF_HL(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void ADC_A_R(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void SBC_A_R(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void CP_A_R(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void LD_A_DEREF_LONG_R(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void LD_A_DEREF_HL_INC(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void LD_A_DEREF_HL_DEC(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void LD_ADDR_LONG_R_A(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void LD_ADDR_HL_INC_A(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void LD_ADDR_HL_DEC_A(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void JR_NZ_IMM(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void JR_NC_IMM(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void LD_DEREF_FF00_PLUS_IMM_A(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void LD_DEREF_FF00_PLUS_C_A(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void LD_LONG_R_IMM(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void INC_LONG_R(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void DEC_LONG_R(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void PUSH_LONG_R(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void POP_LONG_R(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void CP_A_IMM(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void CALL_IMM(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void LD_ADDR_IMM_A(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void JR_IMM(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void JR_Z_IMM(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void LD_A_DEREF_FF00_PLUS_IMM(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void CP_A_DEREF_HL(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void RLA(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void ADC_A_IMM(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void RET(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void RETI(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void CALL_Z_IMM(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void LD_ADDR_IMM_SP(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void NOP(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);

/* PREFIXED INSTRUCTIONS */
void BIT_B_R(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
void RL_R(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);
