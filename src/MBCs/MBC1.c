#include "hardware.h"
#include "memory.h"
#include "utils.h"
#include <stdbool.h>
#include <stdlib.h>

#define MAX_ROM_BANKS 0x80

static uint8_t **rom_banks;
static uint8_t max_rom_banks;

static uint8_t *vram;
static uint8_t *wram;
static uint8_t *oam;
static uint8_t *io_ram;

static bool ram_bank_enabled;
static bool rom_bank_enabled;
static uint8_t selected_rom_bank;
static uint8_t selected_ram_bank;
static uint8_t bank_mode_select;

static uint8_t mbc1_get_memory_byte(uint16_t address);
static void mbc1_set_memory_byte(uint16_t address, uint8_t byte);
static void handle_IO_write(uint16_t address, uint8_t byte);
static uint8_t mbc1_privileged_get_memory_byte(uint16_t address);
static void mbc1_privileged_set_memory_byte(uint16_t address, uint8_t byte);
static void mbc1_load_rom(FILE *rom);

MBC initialize_mbc1(CartridgeHeader ch) {
    MBC mbc1;
    mbc1.set_memory_byte = &mbc1_set_memory_byte;
    mbc1.get_memory_byte = &mbc1_get_memory_byte;
    mbc1.privileged_set_memory_byte = &mbc1_privileged_set_memory_byte;
    mbc1.privileged_get_memory_byte = &mbc1_privileged_get_memory_byte;
    mbc1.load_rom = &mbc1_load_rom;

    max_rom_banks = ch.rom_banks & 0x7F;
    fprintf(stderr,"Max rom banks %d", max_rom_banks);
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

    vram = calloc(0x9FFF - 0x7FFF, sizeof(uint8_t));
    if (!vram) {
        fprintf(stderr, "Unable to allocate memory for VRAM");
        exit(1);
    }
    wram = calloc(0xDFFF - 0xBFFF, sizeof(uint8_t));
    if (!wram) {
        fprintf(stderr, "Unable to allocate memory for WRAM");
        exit(1);
    }

    oam = calloc(0xFE9F - 0xFDFF, sizeof(uint8_t));
    if (!oam) {
        fprintf(stderr, "Unable to allocate memory for memory");
        exit(1);
    }

    io_ram = calloc(0xFFFF - 0xFDFF, sizeof(uint8_t));
    if (!io_ram) {
        fprintf(stderr, "Unable to allocate memory for memory");
        exit(1);
    }
    ram_bank_enabled = false;
    rom_bank_enabled = false;
    selected_rom_bank = 1;
    selected_rom_bank = 0;
    bank_mode_select = 0;
    return mbc1;
}

static void load_rom_bank(uint8_t rom_bank_num, FILE *rom) {
    fread(rom_banks[rom_bank_num], ROM_BANK_SIZE, 1, rom);
}

static void mbc1_load_rom(FILE *rom) {
    // load address 0x000 - 0x4000
    // load all other rom banks with remaining data from rom
    fseek(rom, 0, SEEK_SET);
    for (uint8_t i = 0; i < max_rom_banks; i++) {
        load_rom_bank(i, rom);
    }
    map_dmg(rom_banks[0]);
}

static uint8_t mbc1_get_memory_byte(uint16_t address) {
    if (address >= 0x000 && address < 0x4000) {
        return rom_banks[0][address];
    } else if (address >= 0x4000 && address < 0x8000) {
        return rom_banks[selected_rom_bank][address - 0x4000];
    } else if (address >= 0x8000 && address < 0xA000) {
        return vram[address - 0x8000];
    } else if (address >= 0xC000 && address < 0xE000) {
        return wram[address - 0xC000];
    } else if (address >= 0xE000 && address < 0xFE00) {
        fprintf(stderr, "Unhandled memory read basic\n");
        exit(1);
        return 0x0;
    } else if (address >= 0xFE00 && address < 0xFEA0) {
        return oam[address - 0xFE00];
    } else if (address >= 0xFEA0 && address < 0xFF00) {
        fprintf(stderr, "Unhandled memory read basic\n");
        exit(1);
        return 0x00;
    } else if (address >= 0xFF00 && address <= 0xFFFF) {
        return io_ram[address - 0xFF00];
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
    } else if (address >= 0x8000 && address < 0xA000) {
        vram[address - 0x8000] = byte;
    }else if (address >= 0xC000 && address < 0xE000) {
        wram[address - 0xC000] = byte;
    } else if (address >= 0xE000 && address < 0xFE00) {
        return;
    } else if (address >= 0xFE00 && address < 0xFEA0) {
        oam[address - 0xFE00] = byte;
    } else if (address >= 0xFEA0 && address < 0xFF00) {
        return;
    } else if (address >= 0xFF00 && address <= 0xFFFF) {
        handle_IO_write(address, byte);
    } else {
        fprintf(stderr, "Unhandled memory write basic\n");
        exit(1);
    }
    return;
}

static void handle_IO_write(uint16_t address, uint8_t byte) {
    uint16_t address_offset = address - 0xFF00;
    switch (address) {
        case JOYP:
            if (get_bit(~byte, 4)) {
                io_ram[address_offset] =
                    (byte & 0xF0) | (get_joypad_state() & 0x0F);
            } else if (get_bit(~byte, 5)) {
                io_ram[address_offset] =
                    (byte & 0xF0) | (get_joypad_state() >> 4);
            }
            return;
        case DISABLE_BOOT_ROM:
            if (byte > 0) {
                unmap_dmg(io_ram);
                io_ram[address_offset] = byte;
                return;
            }
        case DIV: io_ram[address_offset] = 0; return;
        case DMA:
            io_ram[address_offset] = byte;
            set_oam_dma_transfer(true);
            return;
        default: io_ram[address_offset] = byte; return;
    }
}

static uint8_t mbc1_privileged_get_memory_byte(uint16_t address) {
    if (address >= 0xFE00 && address < 0xFEA0) {
        return oam[address - 0xFE00];
    } else if (address >= 0xFEA0 && address < 0xFF00) {
        return 0x00;
    } else if (address >= 0xFF00 && address <= 0xFF7F) {
        return io_ram[address - 0xFF00];
    }
    fprintf(stderr, "Unhandled memory read\n");
    exit(1);
}

static void mbc1_privileged_set_memory_byte(uint16_t address, uint8_t byte) {
    if (address >= 0xFE00 && address < 0xFEA0) {
        oam[address - 0xFE00] = byte;
    } else if (address >= 0xFEA0 && address < 0xFF00) {
        return;
    } else if (address >= 0xFF00 && address <= 0xFF7F) {
        io_ram[address - 0xFF00] = byte;
    } else {
        fprintf(stderr, "Unhandled memory write\n");
        exit(1);
    }
    return;
}
