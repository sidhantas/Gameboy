#include "hardware.h"
#include "utils.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Hardware hardware;
Tracer t;

#define TRACER_SIZE 50

uint8_t rom_beginning[0x100];

void initialize_hardware(void) {
    hardware.memory = calloc(MEMORY_SIZE, sizeof(uint8_t));
    if (!(hardware.memory)) {
        fprintf(stderr, "Unable to allocate memory for memory\n");
    }
    hardware.display_buffer = calloc(256 * 256, sizeof(uint32_t));
    if (!(hardware.display_buffer)) {
        fprintf(stderr, "Unable to allocate memory for display buff\n");
    }
    memset(hardware.registers, 0, REGISTER_COUNT);
    hardware.is_implemented = true;
    hardware.is_double_speed = false;
    hardware.sp = 0;
    hardware.pc = 0;
    hardware.instruction_count = 0;
    hardware.step_mode = false;
    hardware.ime_flag = 0;
    hardware.memory[JOYP] = 0xFF;
    initialize_tracer(&t, 1000);
}

void map_dmg(FILE *rom) {
    if (!rom) {
        fprintf(stderr, "No dmg present\n");
        exit(1);
    }
    memcpy(rom_beginning, hardware.memory, 0x100);
    uint16_t bytes_read =
        fread(&(hardware.memory[BOOT_ROM_BEGIN]), DMG_SIZE, 1, rom);
    if (bytes_read != 1) {
        fprintf(stderr, "Unable To Read DMG, %d\n", bytes_read);
        exit(1);
    }
}

void unmap_dmg(void) {
    memcpy(hardware.memory, rom_beginning, 0x100);
}

void load_rom(FILE *rom) {
    uint16_t sectors_read = 0;
    int16_t read_size;
    do {
        read_size =
            fread(&hardware.memory[sectors_read * SECTOR_SIZE],
                  SECTOR_SIZE, 1, rom);
        sectors_read++;
    } while (read_size > 0);
}

uint8_t get_memory_byte(uint16_t address) { return hardware.memory[address]; }

void set_memory_byte(uint16_t address, uint8_t byte) {
    if ((address >= 0x100 && address <= 0x7FFF) || (address >= 0xE000 && address <= 0xFDFF) ||
        address == 0xFF00) {
        return;
    }
    hardware.memory[address] = byte;
}

uint8_t get_flag(flags_t flag) {
    const uint8_t FLAGS_REGISTER = hardware.registers[F];
    return (FLAGS_REGISTER >> (7 - flag)) & 1;
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

uint16_t get_sp(void) { return hardware.sp; }

void set_display_pixel(uint_fast8_t x, uint_fast8_t y,
                       uint_fast32_t pixel_color) {
    hardware.display_buffer[y * DISPLAY_WIDTH + x] = pixel_color;
}

uint32_t *get_display_buffer(void) { return hardware.display_buffer; }

void set_register(reg_t dst, uint8_t val) { hardware.registers[dst] = val; }

uint8_t get_register(reg_t src) { return hardware.registers[src]; }

void set_decoded_instruction(const char *str, ...) {

    strncpy(hardware.previous_instruction, hardware.decoded_instruction,
            MAX_DECODED_INSTRUCTION_SIZE);
    va_list args;
    va_start(args, str);
    vsnprintf(hardware.decoded_instruction, MAX_DECODED_INSTRUCTION_SIZE, str,
              args);
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

void set_long_mem(uint16_t address, uint16_t val) {
    uint8_t b1, b2;
    u16_to_two_u8s(val, &b1, &b2);
    hardware.memory[address] = b1;
    hardware.memory[address + 1] = b2;
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
        case SP: hardware.sp = two_u8s_to_u16(b1, b2); break;
        default: return;
    }
}

uint16_t get_long_reg(long_reg_t long_reg) {
    switch (long_reg) {
        case SP: return hardware.sp;
        case BC:
            return two_u8s_to_u16(hardware.registers[C], hardware.registers[B]);
        case DE:
            return two_u8s_to_u16(hardware.registers[E], hardware.registers[D]);
        case HL:
            return two_u8s_to_u16(hardware.registers[L], hardware.registers[H]);
        default: return 0;
    }
}

void set_interrupts(bool val) { hardware.ime_flag = val; }

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

void dump_tracer(void) { tracer_dump(&t); }
