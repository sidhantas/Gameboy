#include "hardware.h"
#include "interrupts.h"
#include "utils.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Hardware hardware;
Tracer t;

#define TRACER_SIZE 50

void initialize_hardware(void) {
    hardware.display_buffer =
        calloc(DISPLAY_WIDTH * DISPLAY_HEIGHT, sizeof(uint32_t));
    if (!(hardware.display_buffer)) {
        fprintf(stderr, "Unable to allocate memory for display buff\n");
    }
    memset(hardware.registers, 0, REGISTER_COUNT);
    hardware.is_implemented = true;
    hardware.is_double_speed = false;
    hardware.sp = 0;
    hardware.pc = 0;
    hardware.instruction_count = 0;
    hardware.interrupt_state = NOTHING;
    hardware.ime_flag = 0;
    hardware.oam_dma_started = false;
    hardware.base_sp = 0xFFFE;

    // initial state after boot
#ifdef SKIP_BOOT
    hardware.registers[A] = 0x01;
    hardware.registers[F] = 0xB0;
    hardware.registers[B] = 0x00;
    hardware.registers[C] = 0x13;
    hardware.registers[D] = 0x00;
    hardware.registers[E] = 0xD8;
    hardware.registers[H] = 0x01;
    hardware.registers[L] = 0x4D;
    hardware.sp = 0xFFFE;
    hardware.pc = 0x0100;
#endif
}

void destroy_hardware(void) {
    if (hardware.display_buffer) {
        free(hardware.display_buffer);
        hardware.display_buffer = NULL;
    }
    return;
}

uint8_t get_flag(flags_t flag) {
    const uint8_t FLAGS_REGISTER = hardware.registers[F];
    return (FLAGS_REGISTER >> (7 - flag)) & 0x1;
}

void set_flag(flags_t flag) {
    uint8_t *FLAGS_REGISTER = &hardware.registers[F];
    *FLAGS_REGISTER |= (1 << (7 - flag));
}

void reset_flag(flags_t flag) {
    uint8_t *FLAGS_REGISTER = &hardware.registers[F];
    *FLAGS_REGISTER &= ~(1 << (7 - flag));
}

void set_pc(uint16_t new_pc) { hardware.pc = new_pc; }

uint16_t get_pc(void) { return hardware.pc; }

void set_sp(uint16_t new_sp) { hardware.sp = new_sp; }
void set_base_sp(uint16_t new_base) { hardware.base_sp = new_base; }
uint16_t get_base_sp(void) { return hardware.base_sp; }

void stack_push_u16(uint16_t val) {
    uint8_t low = val & 0xFF;
    uint8_t high = val >> 8;
    set_sp(get_sp() - 2);
    set_memory_byte(get_sp(), low);
    set_memory_byte(get_sp() + 1, high);
}

void stack_push_u8(uint8_t val) {
    set_sp(get_sp() - 1);
    set_memory_byte(get_sp(), val);
}

uint16_t stack_pop_u16(void) {
    uint8_t low = get_memory_byte(get_sp());
    uint8_t high = get_memory_byte(get_sp() + 1);

    set_sp(get_sp() + 2);

    return two_u8s_to_u16(low, high);
}

uint8_t stack_pop_u8(void) {
    uint8_t val = get_memory_byte(get_sp());
    set_sp(get_sp() + 1);
    return val;
}

uint16_t get_sp(void) { return hardware.sp; }

void set_display_pixel(uint8_t x, uint8_t y, uint32_t pixel_color) {
    hardware.display_buffer[y * DISPLAY_WIDTH + x] = pixel_color;
}

uint32_t *get_display_buffer(void) { return hardware.display_buffer; }

void set_register(reg_t dst, uint8_t val) { hardware.registers[dst] = val; }

uint8_t get_register(reg_t src) { return hardware.registers[src]; }

void set_decoded_instruction(const char *str, ...) {
#ifdef ENABLE_DEBUGGER
    strncpy(hardware.previous_instruction, hardware.decoded_instruction,
            MAX_DECODED_INSTRUCTION_SIZE);
    va_list args;
    va_start(args, str);
    vsnprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE, str,
              args);
#endif
}

char *get_decoded_instruction(void) { return hardware.decoded_instruction; }
char *get_previous_decoded_instruction(void) {
    return hardware.previous_instruction;
}

void set_long_reg_u16(long_reg_t long_reg, uint16_t val) {
    uint8_t b1, b2;
    u16_to_two_u8s(val, &b1, &b2);
    set_long_reg(long_reg, b1, b2);
}

void set_long_reg(long_reg_t long_reg, uint8_t b1, uint8_t b2) {
    switch (long_reg) {
        case BC:
            hardware.registers[B] = b2;
            hardware.registers[C] = b1;
            break;
        case DE:
            hardware.registers[D] = b2;
            hardware.registers[E] = b1;
            break;
        case HL:
            hardware.registers[H] = b2;
            hardware.registers[L] = b1;
            break;
        case AF:
            hardware.registers[A] = b2;
            hardware.registers[F] = b1;
            break;
        default: exit(1); return;
    }
}

uint16_t get_long_reg(long_reg_t long_reg) {
    switch (long_reg) {
        case BC:
            return two_u8s_to_u16(hardware.registers[C], hardware.registers[B]);
        case DE:
            return two_u8s_to_u16(hardware.registers[E], hardware.registers[D]);
        case HL:
            return two_u8s_to_u16(hardware.registers[L], hardware.registers[H]);
        case AF:
            return two_u8s_to_u16(hardware.registers[F], hardware.registers[A]);
        default: exit(1); return 0;
    }
}

void set_interrupt_state(enum INTERRUPT_STATE state) {
    hardware.interrupt_state = state;
}

enum INTERRUPT_STATE get_interrupt_state(void) {
    return hardware.interrupt_state;
}

void clear_instruction(void) {
    hardware.instruction[0] = 0;
    hardware.instruction[1] = 0;
    hardware.instruction[2] = 0;
}

void append_instruction(uint8_t pos) {
    hardware.instruction[pos] = get_memory_byte(post_inc(&hardware.pc));
    return;
}

uint8_t *get_instruction(void) { return hardware.instruction; }

void inc_instruction_count(void) { hardware.instruction_count++; }

uint64_t get_instruction_count(void) { return hardware.instruction_count; }

void set_is_implemented(bool val) { hardware.is_implemented = val; }

bool get_is_implemented(void) { return hardware.is_implemented; }

uint8_t get_mode(void) { return hardware.mode; }

uint8_t get_ime_flag(void) { return hardware.ime_flag; }
void set_ime_flag(bool val) { hardware.ime_flag = val; }

bool get_oam_dma_transfer(void) { return hardware.oam_dma_started; }

void set_oam_dma_transfer(bool oam_dma_transfer_is_enabled) {
    hardware.oam_dma_started = oam_dma_transfer_is_enabled;
}

void dump_tracer(void) { tracer_dump(&t); }

void set_halted(bool halt_state) { hardware.is_halted = halt_state; }

bool is_halted(void) { return hardware.is_halted; }
