#include "decoder.h"
#include "hardware.h"
#include "instructions.h"
#include <stdio.h>
#include <stdlib.h>

static clock_cycles_t (
    *decode_unprefixed_instruction(uint8_t instruction[MAX_INSTRUCTION_SIZE]))(
    uint8_t instruction[MAX_INSTRUCTION_SIZE]);
static clock_cycles_t (
    *decode_prefixed_instruction(uint8_t instruction[MAX_INSTRUCTION_SIZE]))(
    uint8_t instruction[MAX_INSTRUCTION_SIZE]);

clock_cycles_t (*fetch_instruction(void))(
    uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
    clock_cycles_t (*execute_func)(uint8_t *);
    clear_instruction();
    if (get_memory_byte(get_pc()) == 0xCB) {
        // Instruction has 0xCB prefix
        append_instruction(0);
        append_instruction(1);
        execute_func = decode_prefixed_instruction(get_instruction());
    } else {
        append_instruction(0);
        execute_func = decode_unprefixed_instruction(get_instruction());
    }
    return execute_func;
}

clock_cycles_t execute_instruction(
    clock_cycles_t execute_func(uint8_t instruction[MAX_INSTRUCTION_SIZE])) {
    if (execute_func) {
        inc_instruction_count();
        clock_cycles_t clocks = (*execute_func)(get_instruction());
        if (clocks >= 0) {
            return clocks;
        } else {
            set_is_implemented(false);
        }
    }
    return 0;
}

static clock_cycles_t (*decode_prefixed_instruction(
    uint8_t instruction[MAX_INSTRUCTION_SIZE]))(uint8_t *instruction) {
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
            set_decoded_instruction("UNK");
            return NULL;
    }
}
static clock_cycles_t (
    *decode_unprefixed_instruction(uint8_t instruction[MAX_INSTRUCTION_SIZE]))(
    uint8_t instruction[MAX_INSTRUCTION_SIZE]) {
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
        case 0xA8:
        case 0xA9:
        case 0xAA:
        case 0xAB:
        case 0xAC:
        case 0xAD:
        case 0xAF: {
            return &XOR_A_R;
        }
        case 0xAE: {
            return &XOR_A_DEREF_HL;
        }
        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x77: {
            return &LD_ADDR_HL_R;
        }
        case 0x80:
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
        case 0x87: {
            return &ADD_A_R;
        }
        case 0x86: {
            return &ADD_A_DEREF_HL;
        }
        case 0xA0:
        case 0xA1:
        case 0xA2:
        case 0xA3:
        case 0xA4:
        case 0xA5:
        case 0xA7: {
            return &AND_A_R;
        }
        case 0xA6: {
            return &AND_A_DEREF_HL;
        }
        case 0x06:
        case 0x16:
        case 0x26:
        case 0x0E:
        case 0x1E:
        case 0x2E:
        case 0x3E: {
            append_instruction(1);
            return &LD_R_IMM;
        }
        case 0x04:
        case 0x14:
        case 0x24:
        case 0x0C:
        case 0x1C:
        case 0x2C:
        case 0x3C: {
            return &INC_R;
        }

        case 0x05:
        case 0x15:
        case 0x25:
        case 0x0D:
        case 0x1D:
        case 0x2D:
        case 0x3D: {
            return &DEC_R;
        }
        case 0x90:
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x97: {
            return &SUB_A_R;
        }
        case 0x46:
        case 0x56:
        case 0x66:
        case 0x4E:
        case 0x5E:
        case 0x6E:
        case 0x7E: {
            return &LD_R_DEREF_HL;
        }
        case 0x88:
        case 0x89:
        case 0x8A:
        case 0x8B:
        case 0x8C:
        case 0x8D:
        case 0x8F: {
            return &ADC_A_R;
        }
        case 0x98:
        case 0x99:
        case 0x9A:
        case 0x9B:
        case 0x9C:
        case 0x9D:
        case 0x9F: {
            return &SBC_A_R;
        }
        case 0xB8:
        case 0xB9:
        case 0xBA:
        case 0xBB:
        case 0xBC:
        case 0xBD:
        case 0xBF: {
            return &CP_A_R;
        }
        case 0x0a:
        case 0x1a: {
            return &LD_A_DEREF_LONG_R;
        }
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
            append_instruction(1);
            return &JR_NZ_IMM;
        }
        case 0x30: {
            append_instruction(1);
            return &JR_NC_IMM;
        }
        case 0xE0: {
            append_instruction(1);
            return &LD_ADDR_FF00_PLUS_IMM_REGISTER_A;
        }
        case 0xE2: {
            return &LD_DEREF_FF00_PLUS_C_A;
        }
        case 0x07: {
            return &RLCA;
        }
        case 0x0F: {
            return &RRCA;
        }
        case 0x10: {
            return &STOP;
        }
        case 0x09:
        case 0x19:
        case 0x29:
        case 0x39: {
            return &ADD_HL_LONG_R;
        }
        case 0x1F: {
            return &RRA;
        }

        case 0x27: {
            return &DAA;
        }

        case 0x2f: {
            return &CPL;
        }

        case 0x34: {
            return &INC_DEREF_HL;
        }

        case 0x35: {
            return &DEC_DEREF_HL;
        }
        case 0x36: {
            append_instruction(1);
            return &LD_ADDR_HL_IMM;
        }
        case 0x37: {
            append_instruction(1);
            return &SCF;
        }
        case 0x38: {
            return &JR_C_IMM;
        }
        case 0x3f: {
            return &CCF;
        }
        case 0x76: {
            return &HALT;
        }
        case 0x8e: {
            return &ADC_A_DEREF_HL;
        }
        case 0x96: {
            return &SUB_A_DEREF_HL;
        }
        case 0x9e: {
            return &SBC_A_DEREF_HL;
        }
        case 0xb0:
        case 0xb1:
        case 0xb2:
        case 0xb3:
        case 0xb4:
        case 0xb5:
        case 0xb7: {
            return &OR_A_R;
        }
        case 0xb6: {
            return &OR_A_DEREF_HL;
        }
        case 0xc0: {
            return &RET_NZ;
        }
        case 0xc4: {
            append_instruction(1);
            append_instruction(2);
            return &CALL_NZ_IMM;
        }
        case 0xc6: {
            append_instruction(1);
            return &ADD_A_IMM;
        }
        case 0xC7:
        case 0xD7:
        case 0xE7:
        case 0xF7: {
            return &RST_x0h;
        }
        case 0xc8: {
            return &RET_Z;
        }
        case 0xd8: {
            return &RET_C;
        }
        case 0xCA: {
            append_instruction(1);
            append_instruction(2);
            return &JP_Z_IMM;
        }
        case 0xDA: {
            append_instruction(1);
            append_instruction(2);
            return &JP_C_IMM;
        }
        case 0xCF:
        case 0xDF:
        case 0xEF:
        case 0xFF: {
            return &RST_x8h;
        }
        case 0xD0: {
            return &RET_NC;
        }
        case 0xD4: {
            append_instruction(1);
            append_instruction(2);
            return &CALL_NC_IMM;
        }
        case 0xD6: {
            append_instruction(1);
            return &SUB_A_IMM;
        }
        case 0xDC: {
            append_instruction(1);
            append_instruction(2);
            return &CALL_C_IMM;
        }
        case 0xDE: {
            append_instruction(1);
            return &SBC_A_IMM;
        }
        case 0xE6: {
            append_instruction(1);
            return &AND_A_IMM;
        }
        case 0xE8: {
            append_instruction(1);
            return &ADD_SP_IMM;
        }
        case 0xE9: {
            return &JP_HL;
        }
        case 0xEE: {
            append_instruction(1);
            return &XOR_A_IMM;
        }
        case 0xF2: {
            return &LD_A_DEREF_FF00_PLUS_C;
        }
        case 0xF3: {
            return &DI;
        }
        case 0xF6: {
            append_instruction(1);
            return &OR_A_IMM;
        }
        case 0xF8: {
            append_instruction(1);
            return &LD_HL_SP_PLUS_IMM;
        }
        case 0xF9: {
            return &LD_SP_HL;
        }
        case 0xFA: {
            append_instruction(1);
            append_instruction(2);
            return &LD_A_DEREF_IMM;
        }
        case 0xFB: {
            return &EI;
        }
        case 0x01:
        case 0x11:
        case 0x21:
        case 0x31: {
            append_instruction(1);
            append_instruction(2);

            return &LD_LONG_R_IMM;
        }
        case 0x03:
        case 0x13:
        case 0x23:
        case 0x33: {
            return &INC_LONG_R;
        }
        case 0x0B:
        case 0x1B:
        case 0x2B:
        case 0x3B: {
            return &DEC_LONG_R;
        }
        case 0xc5:
        case 0xd5:
        case 0xe5:
        case 0xf5: {
            return &PUSH_LONG_R;
        }
        case 0xc1:
        case 0xd1:
        case 0xe1:
        case 0xf1: {
            return &POP_LONG_R;
        }

        case 0xFE: {
            append_instruction(1);
            return &CP_A_IMM;
        }

        case 0xCD: {
            append_instruction(1);
            append_instruction(2);

            return &CALL_IMM;
        }

        case 0xEA: {
            append_instruction(1);
            append_instruction(2);

            return &LD_ADDR_IMM_A;
        }
        case 0x18: {
            append_instruction(1);
            return &JR_IMM;
        }
        case 0x28: {
            append_instruction(1);
            return &JR_Z_IMM;
        }
        case 0xF0: {
            append_instruction(1);
            return &LD_A_DEREF_FF00_PLUS_IMM;
        }
        case 0xBE: {
            return &CP_A_DEREF_HL;
        }
        case 0x17: {
            return &RLA;
        }
        case 0xCE: {
            append_instruction(1);
            return &ADC_A_IMM;
        }
        case 0xc9: {
            return &RET;
        }
        case 0xd9: {
            return &RETI;
        }
        case 0xCC: {
            append_instruction(1);
            append_instruction(2);
            return &CALL_Z_IMM;
        }
        case 0x08: {
            append_instruction(1);
            append_instruction(2);
            return &LD_ADDR_IMM_SP;
        }
        case 0xc2: {
            append_instruction(1);
            append_instruction(2);
            return &JP_NZ_IMM;
        }
        case 0xD2: {
            append_instruction(1);
            append_instruction(2);
            return &JP_NC_IMM;
        }
        case 0xc3: {
            append_instruction(1);
            append_instruction(2);
            return &JP_IMM;
        }
        case 0x00: {
            return &NOP;
        }
        case 0xDB:
        case 0xDD:
        case 0xD3:
        case 0xE3:
        case 0xE4:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xF4:
        case 0xFC:
        case 0xFD: {

            set_decoded_instruction("INVALID");
            return &UNK;
        }
        default:
            set_decoded_instruction("UNK");
            return &UNK;
    }
}
