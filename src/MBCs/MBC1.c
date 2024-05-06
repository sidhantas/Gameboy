#include "hardware.h"
#include "memory.h"
#include <stdlib.h>

#define MAX_ROM_BANKS 0x80

static uint8_t *memory;
static uint8_t **rom_banks;
static bool ram_bank_enabled;
static bool rom_bank_enabled;
static uint8_t rom_bank;
static uint8_t ram_bank;
static uint8_t bank_mode_select;

static uint8_t mbc1_get_memory_byte(uint16_t address);
static void mbc1_set_memory_byte(uint16_t address, uint8_t byte);
static uint8_t *mbc1_get_raw_mem_pointer(uint16_t address);
static void handle_IO_write(uint16_t address, uint8_t byte);
static uint8_t mbc1_privileged_get_memory_byte(uint16_t address);
static void mbc1_privileged_set_memory_byte(uint16_t address, uint8_t byte);

MBC *initialize_mbc1(CartridgeHeader ch) {
    MBC *mbc1 = initialize_mbc();
    mbc1->set_memory_byte = &mbc1_set_memory_byte;
    mbc1->get_memory_byte = &mbc1_get_memory_byte;
    mbc1->get_raw_mem_pointer = &mbc1_get_raw_mem_pointer;
    mbc1->privileged_set_memory_byte = &mbc1_privileged_set_memory_byte;
    mbc1->privileged_get_memory_byte = &mbc1_privileged_get_memory_byte;

    memory = calloc(MEMORY_SIZE, sizeof(uint8_t));
    ram_bank_enabled = false;
    rom_bank_enabled = false;
    rom_bank = 0;
    rom_bank = 0;
    bank_mode_select = 0;
    return mbc1;
}

static uint8_t mbc1_get_memory_byte(uint16_t address) {
    if (address >= 0x000 && address < 0x4000) {
        return memory[address];
    }
    return 0x00;
}

static void mbc1_set_memory_byte(uint16_t address, uint8_t byte) {
    if (address >= 0x000 && address < 0x2000) {
        ram_bank_enabled = byte == 0x0A;
    } else if (address >= 0x2000 && address < 0x4000) {
        rom_bank = byte & 0x1F;
    } else if (address >= 0x4000 && address < 0x6000) {
        ram_bank = byte & 0x03;
    } else if (address >= 0x6000 && address < 0x8000) {
        bank_mode_select = byte & 0x01;
    }
}

static uint8_t *mbc1_get_raw_mem_pointer(uint16_t address) {

}

static void handle_IO_write(uint16_t address, uint8_t byte) {}

static uint8_t mbc1_privileged_get_memory_byte(uint16_t address) {}

static void mbc1_privileged_set_memory_byte(uint16_t address, uint8_t byte) {}
