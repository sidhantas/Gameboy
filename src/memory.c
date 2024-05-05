#include "memory.h"
#include "graphics.h"
#include "hardware.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t *memory;
} MBC0;

static MBC0 mbc0;
static uint8_t rom_beginning[0x100];

void initialize_memory(uint8_t cartridge_type) {
    switch (cartridge_type) {
        case 0x00:
            mbc0.memory = calloc(MEMORY_SIZE, sizeof(uint8_t));
            if (!(mbc0.memory)) {
                fprintf(stderr, "Unable to allocate memory for memory\n");
            }
            mbc0.memory[JOYP] = 0x3F;
            return;
        default: fprintf(stderr, "Cartridge type not implemented\n"); exit(1);
    }
    return;
}

void map_dmg(FILE *rom) {
    if (!rom) {
        fprintf(stderr, "No dmg present\n");
        exit(1);
    }
    memcpy(rom_beginning, mbc0.memory, 0x100);
    unsigned long bytes_read =
        fread(&(mbc0.memory[BOOT_ROM_BEGIN]), DMG_SIZE, 1, rom);
    if (bytes_read != 1) {
        fprintf(stderr, "Unable To Read DMG, %d\n", (int)bytes_read);
        exit(1);
    }
}

void unmap_dmg(void) { memcpy(mbc0.memory, rom_beginning, 0x100); }

void load_rom(FILE *rom) {
    uint16_t sectors_read = 0;
    unsigned long read_size;
    do {
        read_size = fread(&mbc0.memory[sectors_read * SECTOR_SIZE], SECTOR_SIZE,
                          1, rom);
        sectors_read++;
    } while (read_size > 0);
    update_window_title((char *)&mbc0.memory[0x134]);
}

void privileged_set_memory_byte(uint16_t address, uint8_t byte) {
    mbc0.memory[address] = byte;
}

uint8_t privileged_get_memory_byte(uint16_t address) {
    return mbc0.memory[address];
}

void set_long_mem(uint16_t address, uint16_t val) {
    uint8_t b1, b2;
    u16_to_two_u8s(val, &b1, &b2);
    mbc0.memory[address] = b1;
    mbc0.memory[address + 1] = b2;
}

uint8_t get_memory_byte(uint16_t address) { return mbc0.memory[address]; }

static void handle_IO_write(uint16_t address, uint8_t byte) {
    switch (address) {
        case JOYP:
            if (get_bit(~byte, 4)) {
                mbc0.memory[address] =
                    (byte & 0xF0) | (get_joypad_state() & 0x0F);
            } else if (get_bit(~byte, 5)) {
                mbc0.memory[address] =
                    (byte & 0xF0) | (get_joypad_state() >> 4);
            }
            return;
        case DISABLE_BOOT_ROM:
            if (byte > 0) {
                unmap_dmg();
                mbc0.memory[address] = byte;
                return;
            }
        case DIV: mbc0.memory[address] = 0; return;
        case DMA:
            mbc0.memory[address] = byte;
            set_oam_dma_transfer(true);
            return;
        default: mbc0.memory[address] = byte; return;
    }
}

void set_memory_byte(uint16_t address, uint8_t byte) {
    if (address <= 0x7FFF || (address >= 0xE000 && address <= 0xFDFF)) {
        return;
    } else if (address >= 0xFF00 && address <= 0xFF7F) {
        handle_IO_write(address, byte);
    } else {
        mbc0.memory[address] = byte;
    }
}
