#include "memory.h"
#include "utils.h"
#include <stdlib.h>

static uint8_t *rom;
static uint8_t *ram;

static uint8_t mbc0_get_memory_byte(uint16_t address);
static void mbc0_set_memory_byte(uint16_t address, uint8_t byte);
static uint32_t mbc0_load_rom(FILE *rom);
static void destroy_mbc0(void);
static void mbc0_load_save_data(FILE *save_location);
static void mbc0_save_data(FILE *save_location);

MBC initialize_mbc0(void) {
    MBC mbc0;
    mbc0.set_memory_byte = &mbc0_set_memory_byte;
    mbc0.get_memory_byte = &mbc0_get_memory_byte;
    mbc0.load_rom = &mbc0_load_rom;
    mbc0.destroy_memory = &destroy_mbc0;
    mbc0.load_save_data = &mbc0_load_save_data;
    mbc0.save_data = &mbc0_save_data;
    rom = calloc(VRAM_BASE - ROM_BANK_00_BASE, sizeof(uint8_t));
    if (!rom) {
        fprintf(stderr, "Unable to allocate memory for ROM");
        exit(1);
    }
    ram = calloc(WRAM_BASE - EX_RAM_BASE, sizeof(uint8_t));
    if (!ram) {
        fprintf(stderr, "Unable to allocate memory for EXRAM");
        exit(1);
    }

    return mbc0;
}

static void destroy_mbc0(void) {
    if (rom) {
        free(rom);
        rom = NULL;
    }

    if (ram) {
        free(ram);
        ram = NULL;
    }

}

static uint32_t mbc0_load_rom(FILE *cartridge) {
    fread(rom, 1, ROM_BANK_SIZE * 2, cartridge);
    uint32_t hash = crc32b(rom, NULL);
    return hash;
}

static void mbc0_save_data(FILE *save_location) {
    (void)save_location;
    return;
}

static void mbc0_load_save_data(FILE *save_location) {
    (void)save_location;
    return;
}

static uint8_t mbc0_get_memory_byte(uint16_t address) {
    if (address >= ROM_BANK_00_BASE && address < VRAM_BASE) {
        return rom[address];
    } else if (address >= EX_RAM_BASE && address < WRAM_BASE) {
        return ram[address - EX_RAM_BASE];
    }
    fprintf(stderr, "Invalid memory read in MBC0");
    exit(1);
}

static void mbc0_set_memory_byte(uint16_t address, uint8_t byte) {
    if (address >= ROM_BANK_00_BASE && address < VRAM_BASE) {
        return;
    } else if (address >= EX_RAM_BASE && address < WRAM_BASE) {
        ram[address - EX_RAM_BASE] = byte;
        return;
    }
    fprintf(stderr, "Invalid memory write in MBC0");
    exit(1);
    return;
}
