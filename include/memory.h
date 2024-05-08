#pragma once
#include <stdint.h>
#include <stdio.h>

typedef struct {
    void (*set_memory_byte)(uint16_t address, uint8_t byte);
    void (*privileged_set_memory_byte)(uint16_t address, uint8_t byte);
    uint8_t (*get_memory_byte)(uint16_t address);
    uint8_t (*privileged_get_memory_byte)(uint16_t address);
    void (*load_rom)(FILE *rom);
} MBC;

#define MAX_TITLE_SIZE 16
#define ROM_BANK_SIZE 0x4000
#define EX_RAM_SIZE 0x2000

enum MEMORY_MAP {
    ROM_BANK_00_BASE = 0x0000,
    ROM_BANK_NN_BASE = 0x4000,
    VRAM_BASE = 0x8000,
    EX_RAM_BASE = 0xA000,
    WRAM_BASE = 0xC000,
    ECHO_RAM_BASE = 0xE000,
    OAM_BASE = 0xFE00,
    PROHIBITED_BASE = 0xFEA0,
    IO_RAM_BASE = 0xFF00,
    HRAM_BASE = 0xFF80
};

typedef struct {
    char title[MAX_TITLE_SIZE + 1];
    uint8_t cartridge_type;
    uint16_t rom_banks;
    uint8_t ram_banks; 
} CartridgeHeader;


void delete_mbc(MBC **mbc);
void map_dmg(void);
void unmap_dmg(void);


MBC initialize_mbc0(void);
MBC initialize_mbc1(CartridgeHeader ch);
