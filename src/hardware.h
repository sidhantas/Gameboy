#pragma once
#include <stdbool.h>
#include <stdint.h>

#define MEMORY_SIZE 0xFFFF
#define BOOT_ROM_BEGIN 0x0000
#define ROM_START 0x0100
#define JOYP 0xFF00
#define SB 0xFF01
#define SC 0xFF02
#define DIV 0xFF04
#define TIMA 0xFF05
#define TMA 0xFF06
#define TAC 0xFF07
#define IF 0xFF0F
#define LCDC 0xFF40
#define SCX 0xFF41
#define SCY 0xFF42
#define WX 0xFF4A
#define WY 0xFF4B
#define LCDY 0xFF44

#define MAX_INSTRUCTION_SIZE 3
#define REGISTER_COUNT 8
#define RESOLUTION_SCALE 3
#define TILE_MAP_WIDTH 256
#define DISPLAY_WIDTH 160
#define DISPLAY_HEIGHT 144
#define SCAN_LINES 154
#define DISPLAY_SIZE DISPLAY_WIDTH * DISPLAY_HEIGHT
#define TILE_MAP_SIZE 32 * 32

#define CLOCK_RATE 4190000
#define DISPLAY_REFRESH_RATE 60

#define MAX_DECODED_INSTRUCTION_SIZE 25
typedef enum REGISTERS { B, C, D, E, H, L, F, A } reg_t;
typedef enum LONG_REGS { BC, DE, HL, SP } long_reg_t;
typedef enum FLAGS { Z_FLAG, N_FLAG, H_FLAG, C_FLAG } flags_t;

static inline char REGISTER_CHAR(enum REGISTERS reg) {
    static const char REGS[] = {'B', 'C', 'D', 'E', 'H', 'L', 'F', 'A'};
    return REGS[reg];
}

static inline char const *LONG_REGISTER_STR(enum LONG_REGS long_reg) {
    static char const *LONG_REGS_STR[4] = {"BC", "DE", "HL", "SP"};
    return LONG_REGS_STR[long_reg];
}


typedef struct Hardware {
    uint8_t *memory;
    uint32_t *display_buffer;
    uint8_t registers[REGISTER_COUNT];
    uint16_t sp;
    uint16_t stack_start;
    uint16_t pc;
    uint8_t opcode;
    uint8_t ime_flag;
    bool is_implemented;
    bool is_double_speed;
    char vram_mode;
    char oam_mode;
    uint8_t mode;
    uint64_t instruction_count;
    uint8_t instruction[MAX_INSTRUCTION_SIZE];
    char decoded_instruction[MAX_DECODED_INSTRUCTION_SIZE];
    bool step_mode;
} Hardware;

extern Hardware hardware;

void initialize_hardware(Hardware *hardware);
uint8_t get_memory_byte(uint16_t address);
void set_memory_byte(uint16_t address, uint8_t byte);
uint8_t get_flag(flags_t flag);
void set_flag(flags_t flag);
void reset_flag(flags_t flag);
