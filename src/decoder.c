#include "decoder.h"
#include "hardware.h"
#include "instructions.h"
#include "utils.h"
#include <inttypes.h>
#include <stdio.h>

static bool is_prefix_byte(uint8_t prefix_byte);
static void *
decode_unprefixed_instruction(uint8_t instruction[MAX_INSTRUCTION_SIZE]);
static void *
decode_prefixed_instruction(const uint8_t instruction[MAX_INSTRUCTION_SIZE]);

void fetch_instruction() {
    void (*execute_func)();
    if (get_memory_byte(hardware.pc) == 0xCB) {
        // Instruction has 0xCB instruction
        hardware.instruction[0] = get_memory_byte(post_inc(&hardware.pc));
        hardware.instruction[1] = get_memory_byte(post_inc(&hardware.pc));
        execute_func = decode_prefixed_instruction(hardware.instruction);
    } else {
        hardware.instruction[0] = get_memory_byte(post_inc(&hardware.pc));
        hardware.instruction[1] = 0;
        hardware.instruction[2] = 0;
        execute_func = decode_unprefixed_instruction(hardware.instruction);
    }
    if (execute_func) {
        hardware.is_implemented = false;
        (*execute_func)(hardware.instruction);
    }
}

static void *
decode_prefixed_instruction(const uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = instruction[1];
    switch (OPCODE) {
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x17: {
            return &RL_R;
        }
        case 0x7c: {
            return &BIT_B_R;
        }
        default:
            snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
                     "UNK");
            return NULL;
    }
}
static void *
decode_unprefixed_instruction(uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    const uint8_t OPCODE = instruction[0];
    switch (OPCODE) {
            /* clang-format off */
        case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x47:
        case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4f:
        case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x57:
        case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5f:
        case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x67:
        case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6f:
        case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7f:
            /* clang-format on */
            { return &LD_RR; }
            /* clang-format off */
        case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAE: case 0xAF:
            /*clang-format on */
            {
                return &XOR_A_R;
            }
            /* clang-format off */
        case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:
            /* clang-format on */
            { return &LD_DEREF_HL_R; }
            /* clang-format off */
        case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
            /* clang-format on */
            { return &ADD_A_R; }
            /* clang-format off */
        case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA6: case 0xA7:
            /* clang-format on */
            { return &AND_A_R; }
            /* clang-format off */
        case 0x06: case 0x16: case 0x26: case 0x0E: case 0x1E: case 0x2E: case 0x3E:
            /* clang-format on */
            {
                instruction[1] = get_memory_byte(post_inc(&hardware.pc));
                return &LD_R_IMM;
            }
            /* clang-format off */
        case 0x04: case 0x14: case 0x24: case 0x34: case 0x0C: case 0x1C: case 0x2C: case 0x3C:
            /* clang-format on */
            { return &INC_R; }

            /* clang-format off */
        case 0x05: case 0x15: case 0x25: case 0x0D: case 0x1D: case 0x2D: case 0x3D:
            /* clang-format on */
            { return &DEC_R; }

            /* clang-format off */
        case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x97:
            /* clang-format on */
            { return &SUB_A_R; }
            /* clang-format off */
        case 0x46: case 0x56: case 0x66: case 0x4E: case 0x5E: case 0x6E: case 0x7E:
            /* clang-format on */
            { return &LD_R_DEREF_HL; }
            /* clang-format off */
        case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8F:
            /* clang-format on */
            { return &ADC_A_R; }
            /* clang-format off */
        case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9F:
            /* clang-format on */
            { return &SBC_A_R; }
            /* clang-format off */
        case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBF:
            /* clang-format on */
            { return &CP_A_R; }
            /* clang-format off */
        case 0x0a: case 0x1a:
            /* clang-format on */
            { return &LD_A_DEREF_LONG_R; }
        case 0x2a: {
            return &LD_A_DEREF_HL_INC;
        }
        case 0x3a: {
            return &LD_A_DEREF_HL_DEC;
        }
        case 0x02:
        case 0x12: {
            return &LD_ADDR_LONG_R_A;
        }
        case 0x22: {

            return &LD_ADDR_HL_INC_A;
        }
        case 0x32: {
            return &LD_ADDR_HL_DEC_A;
        }
        case 0x20: {
            instruction[1] = get_memory_byte(post_inc(&hardware.pc));
            return &JR_NZ_IMM;
        }
        case 0x30: {
            instruction[1] = get_memory_byte(post_inc(&hardware.pc));
            return &JR_NC_IMM;
        }
        case 0xE0: {
            instruction[1] = get_memory_byte(post_inc(&hardware.pc));
            return &LD_DEREF_FF00_PLUS_IMM_A;
        }
        case 0xE2: {
            return &LD_DEREF_FF00_PLUS_C_A;
        }
            /* clang-format off */
        case 0x01: case 0x11: case 0x21: case 0x31:
            /* clang-format on */
            {
                instruction[1] = get_memory_byte(post_inc(&hardware.pc));
                instruction[2] = get_memory_byte(post_inc(&hardware.pc));

                return &LD_LONG_R_IMM;
            }
            /* clang-format off */
        case 0x03: case 0x13: case 0x23: case 0x33:
            /* clang-format on */
            { return &INC_LONG_R; }
            /* clang-format off */
        case 0x0B: case 0x1B: case 0x2B: case 0x3B:
            /* clang-format on */
            { return &DEC_LONG_R; }
            /* clang-format off */
        case 0xc5: case 0xd5: case 0xe5: case 0xf5:
            /* clang-format on */
            { return &PUSH_LONG_R; }
            /* clang-format off */
        case 0xc1: case 0xd1: case 0xe1: case 0xf1:
            /* clang-format on */
            { return &POP_LONG_R; }

        case 0xFE: {
            instruction[1] = get_memory_byte(post_inc(&hardware.pc));
            return &CP_A_IMM;
        }

        case 0xCD: {
            instruction[1] = get_memory_byte(post_inc(&hardware.pc));
            instruction[2] = get_memory_byte(post_inc(&hardware.pc));

            return &CALL_IMM;
        }

        case 0xEA: {
            instruction[1] = get_memory_byte(post_inc(&hardware.pc));
            instruction[2] = get_memory_byte(post_inc(&hardware.pc));

            return &LD_ADDR_IMM_A;
        }
        case 0x18: {
            instruction[1] = get_memory_byte(post_inc(&hardware.pc));
            return &JR_IMM;
        }
        case 0x28: {
            instruction[1] = get_memory_byte(post_inc(&hardware.pc));
            return &JR_Z_IMM;
        }
        case 0xF0: {
            instruction[1] = get_memory_byte(post_inc(&hardware.pc));
            return &LD_A_DEREF_FF00_PLUS_IMM;
        }
        case 0xBE: {
            return &CP_A_DEREF_HL;
        }
        case 0x17: {
            return &RLA;
        }
        case 0xCE: {
            instruction[1] = get_memory_byte(post_inc(&hardware.pc));
            return &ADC_A_IMM;
        }
        case 0xc9: {
            return &RET;
        }
        case 0xd9: {
            return &RETI;
        }
        case 0xCC: {
            instruction[1] = get_memory_byte(post_inc(&hardware.pc));
            instruction[2] = get_memory_byte(post_inc(&hardware.pc));
            return &CALL_Z_IMM;
        }
        case 0x08: {
            instruction[1] = get_memory_byte(post_inc(&hardware.pc));
            instruction[2] = get_memory_byte(post_inc(&hardware.pc));
            return &LD_ADDR_IMM_SP;
        }
        case 0x00: {
            return &NOP;
        }
            /* clang-format off */
        case 0xDB: case 0xDD: case 0xE3: case 0xE4: case 0xEB: case 0xEC: case 0xED:
        case 0xF4: case 0xFC: case 0xFD:
            /* clang-format on */
            {

                snprintf(hardware.decoded_instruction,
                         MAX_DECODED_INSTRUCTION_SIZE, "INVALID");
                return &NOP;
            }
        default:
            snprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE,
                     "UNK");
            return &NOP;
    }
}
