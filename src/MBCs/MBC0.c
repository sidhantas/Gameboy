#include "hardware.h"
#include "memory.h"
#include "utils.h"
#include <stdlib.h>

static uint8_t *memory;
static uint8_t mbc0_get_memory_byte(uint16_t address);
static void mbc0_set_memory_byte(uint16_t address, uint8_t byte);
static void handle_IO_write(uint16_t address, uint8_t byte);
static uint8_t mbc0_privileged_get_memory_byte(uint16_t address);
static void mbc0_privileged_set_memory_byte(uint16_t address, uint8_t byte);
static void mbc0_load_rom(FILE *rom);

MBC initialize_mbc0(void) {
    MBC mbc0;
    mbc0.set_memory_byte = &mbc0_set_memory_byte;
    mbc0.get_memory_byte = &mbc0_get_memory_byte;
    mbc0.privileged_set_memory_byte = &mbc0_privileged_set_memory_byte;
    mbc0.privileged_get_memory_byte = &mbc0_privileged_get_memory_byte;
    mbc0.load_rom = &mbc0_load_rom;
    memory = calloc(MEMORY_SIZE, sizeof(uint8_t));

    return mbc0;
}

static void mbc0_load_rom(FILE *rom) {
    uint16_t sectors_read = 0;
    unsigned long read_size;
    do {
        read_size = fread(&memory[sectors_read * SECTOR_SIZE], SECTOR_SIZE,
                          1, rom);
        sectors_read++;
    } while (read_size > 0);
    map_dmg(memory);
}

static uint8_t mbc0_get_memory_byte(uint16_t address) {
    return memory[address];
}
static uint8_t mbc0_privileged_get_memory_byte(uint16_t address) {
    return memory[address];
}

static void mbc0_privileged_set_memory_byte(uint16_t address, uint8_t byte) {
    memory[address] = byte;
}
static void mbc0_set_memory_byte(uint16_t address, uint8_t byte) {
    if (address <= 0x7FFF || (address >= 0xE000 && address <= 0xFDFF)) {
        return;
    } else if (address >= 0xFF00 && address <= 0xFF7F) {
        handle_IO_write(address, byte);
    } else {
        memory[address] = byte;
    }
    return;
}


static void handle_IO_write(uint16_t address, uint8_t byte) {
    switch (address) {
        case JOYP:
            if (get_bit(~byte, 4)) {
                memory[address] =
                    (byte & 0xF0) | (get_joypad_state() & 0x0F);
            } else if (get_bit(~byte, 5)) {
                memory[address] =
                    (byte & 0xF0) | (get_joypad_state() >> 4);
            }
            return;
        case DISABLE_BOOT_ROM:
            if (byte > 0) {
                unmap_dmg(memory);
                memory[address] = byte;
                return;
            }
        case DIV: memory[address] = 0; return;
        case DMA:
            memory[address] = byte;
            set_oam_dma_transfer(true);
            return;
        default: memory[address] = byte; return;
    }
}

