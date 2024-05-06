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

typedef struct {
    char title[MAX_TITLE_SIZE + 1];
    uint8_t cartridge_type;
    uint16_t rom_banks;
    uint8_t ram_banks; 
} CartridgeHeader;


MBC *initialize_mbc(void);
void delete_mbc(MBC **mbc);
void map_dmg(uint8_t *);
void unmap_dmg(uint8_t *);


MBC initialize_mbc0(void);
MBC initialize_mbc1(CartridgeHeader ch);
