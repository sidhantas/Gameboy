#include "cpu.h"
#include "memory.h"
#include "utils.h"
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_ROM_BANKS 0x80

static uint8_t **rom_banks;
static uint8_t max_rom_banks;
static uint8_t **ram_banks;
static uint8_t max_ram_banks;
static uint8_t bank_1_mask;

static bool ram_bank_enabled;
static bool rom_bank_enabled;

static bool is_large_cartridge = false;

static uint8_t bank_register_1;
static uint8_t bank_register_2;
static uint8_t bank_mode_select;

static uint8_t mbc1_get_memory_byte(uint16_t address);
static void mbc1_set_memory_byte(uint16_t address, uint8_t byte);
static uint32_t mbc1_load_rom(FILE *rom);
static void mbc1_load_save_data(FILE *save_location);
static void mbc1_save_data(FILE *save_location);
static void destroy_mbc_1(void);

MBC initialize_mbc1(CartridgeHeader ch) {
    MBC mbc1;
    mbc1.set_memory_byte = &mbc1_set_memory_byte;
    mbc1.get_memory_byte = &mbc1_get_memory_byte;
    mbc1.load_rom = &mbc1_load_rom;
    mbc1.save_data = &mbc1_save_data;
    mbc1.load_save_data = &mbc1_load_save_data;
    mbc1.destroy_memory = &destroy_mbc_1;

    max_rom_banks = ch.rom_banks & 0xFF;
    max_ram_banks = ch.ram_banks;
    bank_1_mask = ~((max_rom_banks - 1) ^ 0xFF);
    if (max_rom_banks >= 64) {
        is_large_cartridge = true;
        max_ram_banks = 1;
    }
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

    ram_banks = (uint8_t **)malloc(sizeof(uint8_t *) * max_ram_banks);
    if (!ram_banks) {
        fprintf(stderr, "Unable to allocate memory for RAM Banks\n");
        exit(1);
    }
    for (uint8_t i = 0; i < max_ram_banks; i++) {
        ram_banks[i] = (uint8_t *)calloc(RAM_BANK_SIZE, sizeof(uint8_t));
        if (!ram_banks[i]) {
            fprintf(stderr, "Unable to allocate memory for RAM Banks\n");
            exit(1);
        }
    }
    ram_bank_enabled = false;
    rom_bank_enabled = false;
    bank_register_1 = 1;
    bank_register_2 = 0;
    bank_mode_select = 0;
    return mbc1;
}

static void destroy_mbc_1(void) {
    for (uint8_t i = 0; i < max_rom_banks; i++) {
        if (rom_banks[i]) {
            free(rom_banks[i]);
            rom_banks[i] = NULL;
        }
    }
    free(rom_banks);
    rom_banks = NULL;

    for (uint8_t i = 0; i < max_ram_banks; i++) {
        if (ram_banks[i]) {
            free(ram_banks[i]);
            ram_banks[i] = NULL;
        }
    }
    free(ram_banks);
    ram_banks = NULL;
}

static uint8_t get_rom_bank_x0(void) {
    if (bank_mode_select != 1 || !is_large_cartridge) {
        return 0;
    }
    // Using extended banking
    return (uint8_t)(bank_register_2 << 5);
}

static uint8_t get_rom_bank_01(void) {
    uint8_t selected_rom_bank = bank_register_1;
    if (is_large_cartridge) {
        selected_rom_bank |= (bank_register_2 << 5);
    }
    return selected_rom_bank;
}

uint8_t get_ram_bank(void) {
    if (bank_mode_select == 0 || is_large_cartridge || max_ram_banks == 1) {
        return 0;
    }
    return bank_register_2;
}

static uint8_t mbc1_get_memory_byte(uint16_t address) {
    if (address >= ROM_BANK_00_BASE && address < ROM_BANK_NN_BASE) {
        return rom_banks[get_rom_bank_x0()][address];
    } else if (address >= ROM_BANK_NN_BASE && address < VRAM_BASE) {
        return rom_banks[get_rom_bank_01()][address - ROM_BANK_NN_BASE];
    } else if (address >= EX_RAM_BASE && address < WRAM_BASE) {
        if (!ram_bank_enabled || max_ram_banks == 0) {
            return 0xFF;
        }
        return ram_banks[get_ram_bank()][address - EX_RAM_BASE];
    } else {
        fprintf(stderr, "Unhandled memory read basic\n");
        exit(1);
    }
    return 0x00;
}

static void mbc1_set_memory_byte(uint16_t address, uint8_t byte) {
    if (address >= 0x000 && address < 0x2000) {
        ram_bank_enabled = (byte & 0x0F) == 0x0A;
    } else if (address >= 0x2000 && address < 0x4000) {
        /*
         * ROM bank is set according to byte unless it's 0
         * in which case it is 1
         */
        bank_register_1 = byte & 0x1F ? byte & 0x1F : 1;
        bank_register_1 &= bank_1_mask;
    } else if (address >= 0x4000 && address < 0x6000) {
        bank_register_2 = byte & 0x03;
    } else if (address >= 0x6000 && address < 0x8000) {
        if (max_ram_banks <= 1 || max_rom_banks <= 32) {
            bank_mode_select = 0;
        }
        bank_mode_select = get_bit(byte, 0);
    } else if (address >= EX_RAM_BASE && address < WRAM_BASE) {
        if (!ram_bank_enabled || max_ram_banks == 0) {
            return;
        }
        ram_banks[get_ram_bank()][address - EX_RAM_BASE] = byte;
    } else {
        fprintf(stderr, "Unhandled memory write basic\n");
        exit(1);
    }
    return;
}
static void load_rom_bank(uint8_t rom_bank_num, FILE *rom) {
    fread(rom_banks[rom_bank_num], 1, ROM_BANK_SIZE, rom);
}

static uint32_t mbc1_load_rom(FILE *rom) {
    // load address 0x000 - 0x4000
    // load all other rom banks with remaining data from rom
    uint32_t hash = 0;
    fseek(rom, 0, SEEK_SET);
    for (uint8_t i = 0; i < max_rom_banks; i++) {
        load_rom_bank(i, rom);
        hash = crc32b(rom_banks[i], hash ? &hash : NULL);
    }
    return hash;
}

static void mbc1_save_data(FILE *save_location) {
    for (uint16_t i = 0; i < max_ram_banks; i++) {
        fwrite(ram_banks[i], 1, RAM_BANK_SIZE, save_location);
    }
    
}

static void mbc1_load_save_data(FILE *save_location) {
    if (!save_location) {
        return;
    }
    for (uint16_t i = 0; i < max_ram_banks; i++) {
        fread(ram_banks[i], 1, RAM_BANK_SIZE, save_location);
    }
}

uint8_t get_banking_mode(void) { return bank_mode_select; }
