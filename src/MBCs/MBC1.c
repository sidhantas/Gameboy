#include "memory.h"
#include <stdbool.h>
#include <stdlib.h>

#define MAX_ROM_BANKS 0x80

static uint8_t **rom_banks;
static uint8_t max_rom_banks;
static uint8_t **ram_banks;
static uint8_t max_ram_banks;

static bool ram_bank_enabled;
static bool rom_bank_enabled;
static uint8_t selected_rom_bank;
static uint8_t selected_ram_bank;
static uint8_t bank_mode_select;

static uint8_t mbc1_get_memory_byte(uint16_t address);
static void mbc1_set_memory_byte(uint16_t address, uint8_t byte);
static void mbc1_load_rom(FILE *rom);

MBC initialize_mbc1(CartridgeHeader ch) {
    MBC mbc1;
    mbc1.set_memory_byte = &mbc1_set_memory_byte;
    mbc1.get_memory_byte = &mbc1_get_memory_byte;
    mbc1.load_rom = &mbc1_load_rom;

    max_rom_banks = ch.rom_banks & 0x7F;
    max_ram_banks = ch.ram_banks & 0x03;
    fprintf(stderr, "Max rom banks %d", max_rom_banks);
    rom_banks = (uint8_t **)malloc(sizeof(uint8_t *) * max_rom_banks);
    if (!rom_banks) {
        fprintf(stderr, "Unable to allocate memory for ROM Banks");
        exit(1);
    }
    for (uint8_t i = 0; i < max_rom_banks; i++) {
        rom_banks[i] = (uint8_t *)calloc(ROM_BANK_SIZE, sizeof(uint8_t));
        if (!rom_banks[i]) {
            fprintf(stderr, "Unable to allocate memory for ROM Banks");
            exit(1);
        }
    }

    ram_banks = (uint8_t **)malloc(sizeof(uint8_t *) * max_rom_banks);
    if (!ram_banks) {
        fprintf(stderr, "Unable to allocate memory for ROM Banks");
        exit(1);
    }
    for (uint8_t i = 0; i < max_ram_banks; i++) {
        ram_banks[i] = (uint8_t *)calloc(EX_RAM_SIZE, sizeof(uint8_t));
        if (!rom_banks[i]) {
            fprintf(stderr, "Unable to allocate memory for RAM Banks");
            exit(1);
        }
    }
    ram_bank_enabled = false;
    rom_bank_enabled = false;
    selected_rom_bank = 1;
    selected_ram_bank = 0;
    bank_mode_select = 0;
    return mbc1;
}

static void load_rom_bank(uint8_t rom_bank_num, FILE *rom) {
    fread(rom_banks[rom_bank_num], 1, ROM_BANK_SIZE, rom);
}

static void mbc1_load_rom(FILE *rom) {
    // load address 0x000 - 0x4000
    // load all other rom banks with remaining data from rom
    fseek(rom, 0, SEEK_SET);
    for (uint8_t i = 0; i < max_rom_banks; i++) {
        load_rom_bank(i, rom);
    }
}

static uint8_t mbc1_get_memory_byte(uint16_t address) {
    if (address >= ROM_BANK_00_BASE && address < ROM_BANK_NN_BASE) {
        return rom_banks[0][address];
    } else if (address >= ROM_BANK_NN_BASE && address < VRAM_BASE) {
        return rom_banks[selected_rom_bank][address - ROM_BANK_NN_BASE];
    } else if (address >= EX_RAM_BASE && address < WRAM_BASE) {
        if (!ram_bank_enabled) {
            return 0xFF;
        }
        return ram_banks[selected_ram_bank][address - EX_RAM_BASE];
    } else {
        fprintf(stderr, "Unhandled memory read basic\n");
        exit(1);
    }
    return 0x00;
}

static void mbc1_set_memory_byte(uint16_t address, uint8_t byte) {
    if (address >= 0x000 && address < 0x2000) {
        ram_bank_enabled = byte == 0x0A;
    } else if (address >= 0x2000 && address < 0x4000) {
        // ROM bank is set according to byte unless it's 0
        // in which case it is 1
        selected_rom_bank = byte & 0x1F ? byte & 0x1F : 1;
    } else if (address >= 0x4000 && address < 0x6000) {
        selected_ram_bank = byte & 0x03;
    } else if (address >= 0x6000 && address < 0x8000) {
        bank_mode_select = byte & 0x01;
    } else if (address >= EX_RAM_BASE && address < WRAM_BASE) {
        if (!ram_bank_enabled) {
            return;
        }
        ram_banks[selected_ram_bank][address - EX_RAM_BASE] = byte;
    } else {
        fprintf(stderr, "Unhandled memory write basic\n");
        exit(1);
    }
    return;
}
