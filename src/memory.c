#include "memory.h"
#include "graphics.h"
#include "hardware.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

static MBC mbc;
static uint8_t rom_beginning[0x100];

static CartridgeHeader decode_cartridge_header(FILE *rom) {
    uint8_t buffer[0x50];
    fseek(rom, 0x100, SEEK_SET);
    fread(buffer, 0x50, 1, rom);
    CartridgeHeader ch;

    strncpy(ch.title, (char *)&buffer[0x34], MAX_TITLE_SIZE);
    ch.title[MAX_TITLE_SIZE] = '\0';

    ch.cartridge_type = buffer[0x47];
    ch.rom_banks = (uint16_t)(2 * (1 << buffer[0x48]));
    switch (buffer[0x49]) {
        case 0: ch.ram_banks = 0; break;
        case 0x01: ch.ram_banks = 0; break;
        case 0x02: ch.ram_banks = 1; break;
        case 0x03: ch.ram_banks = 4; break;
        case 0x04: ch.ram_banks = 16; break;
        case 0x05: ch.ram_banks = 8; break;
        default:
            fprintf(stderr, "Invalid RAM size code in cartridge header\n");
            exit(1);
    }
    fseek(rom, 0x0, SEEK_SET);
    update_window_title(ch.title);
    return ch;
}

void initialize_memory(CartridgeHeader ch) {
    switch (ch.cartridge_type) {
        case 0x00: mbc = initialize_mbc0(); return;
        case 0x01: mbc = initialize_mbc1(ch);
        default: fprintf(stderr, "Cartridge type not implemented\n"); exit(1);
    }
}

MBC *initialize_mbc(void) {
    MBC *mbc = (MBC *)malloc(sizeof(MBC));
    if (mbc == NULL) {
        fprintf(stderr, "Unable to allocate memory for MBC");
        exit(1);
    }
    return mbc;
}

void delete_mbc(MBC **mbc) { free(*mbc); }

void map_dmg(uint8_t *memory_start) {
    FILE *dmg = fopen("dmg.bin", "r");
    if (!dmg) {
        fprintf(stderr, "No dmg present\n");
        exit(1);
    }
    memcpy(rom_beginning, &memory_start[0x0], 0x100);
    unsigned long bytes_read =
        fread(&memory_start[BOOT_ROM_BEGIN], DMG_SIZE, 1, dmg);
    if (bytes_read != 1) {
        fprintf(stderr, "Unable To Read DMG, %d\n", (int)bytes_read);
        exit(1);
    }
}

void unmap_dmg(uint8_t *memory) { memcpy(&memory[0x0], rom_beginning, 0x100); }

void load_rom(FILE *rom) {
    CartridgeHeader ch = decode_cartridge_header(rom);
    initialize_memory(ch);
    mbc.load_rom(rom);
}

void privileged_set_memory_byte(uint16_t address, uint8_t byte) {
    mbc.privileged_set_memory_byte(address, byte);
}

uint8_t privileged_get_memory_byte(uint16_t address) {
    return mbc.privileged_get_memory_byte(address);
}

void set_long_mem(uint16_t address, uint16_t val) {
    uint8_t b1, b2;
    u16_to_two_u8s(val, &b1, &b2);
    set_memory_byte(address, b1);
    set_memory_byte(address + 1, b2);
}

uint8_t get_memory_byte(uint16_t address) {
    return mbc.get_memory_byte(address);
}

void set_memory_byte(uint16_t address, uint8_t byte) {
    mbc.set_memory_byte(address, byte);
}
