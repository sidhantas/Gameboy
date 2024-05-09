#pragma once
#include "instruction_tracer.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define MEMORY_SIZE 0xFFFF
#define OAM_START 0xFE00
#define OAM_SIZE 0xA0
#define BOOT_ROM_BEGIN 0x0000
#define ROM_START 0x0100

#define MAX_INSTRUCTION_SIZE 3
#define REGISTER_COUNT 8
#define RESOLUTION_SCALE 4
#define TILE_MAP_WIDTH 256
#define DISPLAY_WIDTH 160
#define DISPLAY_HEIGHT 144
#define SCAN_LINES 154
#define DISPLAY_SIZE DISPLAY_WIDTH *DISPLAY_HEIGHT
#define TILE_MAP_SIZE 32 * 32

#define CLOCK_RATE 4194304
#define DISPLAY_REFRESH_RATE 60

#define MAX_DECODED_INSTRUCTION_SIZE 25
typedef enum REGISTERS { B, C, D, E, H, L, F, A } reg_t;
typedef enum LONG_REGS { BC, DE, HL, AF } long_reg_t;
typedef enum FLAGS { Z_FLAG, N_FLAG, H_FLAG, C_FLAG } flags_t;

static inline char REGISTER_CHAR(enum REGISTERS reg) {
  static const char REGS[] = {'B', 'C', 'D', 'E', 'H', 'L', 'F', 'A'};
  return REGS[reg];
}

static inline char const *LONG_REGISTER_STR(enum LONG_REGS long_reg) {
  static char const *LONG_REGS_STR[4] = {"BC", "DE", "HL", "AF"};
  return LONG_REGS_STR[long_reg];
}

typedef enum {
  INVALID_CLOCKS = -1,
  ZERO_CLOCKS = 0,
  FOUR_CLOCKS = 4,
  EIGHT_CLOCKS = 8,
  TWELVE_CLOCKS = 12,
  SIXTEEN_CLOCKS = 16,
  TWENTY_CLOCKS = 20,
  TWENTY_FOUR_CLOCKS = 24,
  THIRTY_TWO_CLOCKS = 32
} clock_cycles_t;

enum INTERRUPT_STATE {
  NOTHING = 0,
  DISABLE = 1,
  ENABLE = 2,
};

typedef struct Hardware {
  uint32_t *display_buffer;
  uint8_t registers[REGISTER_COUNT];
  uint16_t sp;
  uint16_t base_sp;
  uint16_t stack_start;
  uint16_t pc;
  uint8_t opcode;
  enum INTERRUPT_STATE interrupt_state;
  uint8_t ime_flag;
  bool is_implemented;
  bool is_double_speed;
  uint8_t mode;
  uint64_t instruction_count;
  uint8_t instruction[MAX_INSTRUCTION_SIZE];
  char decoded_instruction[MAX_DECODED_INSTRUCTION_SIZE];
  char previous_instruction[MAX_DECODED_INSTRUCTION_SIZE];
  bool step_mode;
  bool oam_dma_started;
  bool is_halted;
} Hardware;

typedef struct Joypad {
  // high nibble == action
  // low nibble == directional
  uint8_t inputs;
} Joypad;

typedef enum JoypadButtons {
  RIGHT,
  LEFT,
  UP,
  DOWN,
  A_BUTTON,
  B_BUTTON,
  SELECT,
  START
} joypad_t;

extern Tracer t;

void initialize_hardware(void);
void initialize_io(void);
void load_rom(FILE *);
uint8_t privileged_get_memory_byte(uint16_t address);
uint8_t get_memory_byte(uint16_t address);
void privileged_set_memory_byte(uint16_t address, uint8_t byte);
void set_memory_byte(uint16_t address, uint8_t byte);
uint8_t get_flag(flags_t flag);
void set_flag(flags_t flag);
void reset_flag(flags_t flag);
void set_display_pixel(uint_fast8_t x, uint_fast8_t y,
                       uint_fast32_t pixel_color);
uint32_t *get_display_buffer(void);
void set_register(reg_t dst, uint8_t val);
uint8_t get_register(reg_t src);
void set_decoded_instruction(const char *str, ...);
char *get_previous_decoded_instruction(void);
void set_long_reg(long_reg_t long_reg, uint8_t b1, uint8_t b2);
void set_long_mem(uint16_t address, uint16_t val);
void set_long_reg_u16(long_reg_t long_reg, uint16_t val);
uint16_t get_long_reg(long_reg_t long_reg);
void set_pc(uint16_t new_pc);
uint16_t get_pc(void);
void set_sp(uint16_t new_sp);
void set_base_sp(uint16_t new_base);
uint16_t get_base_sp(void);
void dec_sp(void);
uint16_t get_sp(void);
void set_interrupt_state(enum INTERRUPT_STATE val);
enum INTERRUPT_STATE get_interrupt_state(void);
void append_instruction(uint8_t pos);
uint8_t *get_instruction(void);
void inc_instruction_count(void);
void set_is_implemented(bool val);
bool get_is_implemented(void);
char *get_decoded_instruction(void);
uint8_t get_mode(void);
uint8_t get_ime_flag(void);
void set_ime_flag(bool val);
uint64_t get_instruction_count(void);
void clear_instruction(void);
void dump_tracer(void);
void stack_push_u16(uint16_t val);
uint16_t stack_pop_u16(void);
uint8_t stack_pop_u8(void);
void set_joypad_state(joypad_t button);
void reset_joypad_state(joypad_t button);
uint8_t get_joypad_state(void);
bool get_oam_dma_transfer(void);
void set_oam_dma_transfer(bool oam_dma_transfer_is_enabled);

// TIMER
void update_timer(clock_cycles_t clocks);

// HALT instruction
void set_halted(bool halt_state);
bool is_halted(void);
