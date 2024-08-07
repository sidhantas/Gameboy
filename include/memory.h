#pragma once
#include <stdint.h>
#include <stdio.h>

#define MAX_SAVE_DATA_NAME_SIZE 40
#define MAX_TITLE_SIZE 16
#define ROM_BANK_SIZE 0x4000
#define RAM_BANK_SIZE 0x2000

typedef struct {
  char title[MAX_TITLE_SIZE + 1];
  uint8_t cartridge_type;
  uint16_t rom_banks;
  uint8_t ram_banks;
} CartridgeHeader;
typedef struct {
  void (*set_memory_byte)(uint16_t address, uint8_t byte);
  uint8_t (*get_memory_byte)(uint16_t address);
  uint32_t (*load_rom)(FILE *rom);
  void (*save_data)(FILE *save_location);
  void (*load_save_data)(FILE *save_location);
  void (*destroy_memory)(void);
} MBC;

struct RTC {
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
  uint8_t DL;
  uint8_t DH;
};

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

enum IO_REGISTERS {
  JOYP = 0xFF00,
  SB = 0xFF01,
  SC = 0xFF02,
  DIV = 0xFF04,
  TIMA = 0xFF05,
  TMA = 0xFF06,
  TAC = 0xFF07,
  IF = 0xFF0F,
  NR10 = 0xFF10,
  NR11 = 0xFF11,
  NR12 = 0xFF12,
  NR13 = 0xFF13,
  NR14 = 0xFF14,
  NR21 = 0xFF16,
  NR22 = 0xFF17,
  NR23 = 0xFF18,
  NR24 = 0xFF19,
  NR30 = 0xFF1A,
  NR31 = 0xFF1B,
  NR32 = 0xFF1C,
  NR33 = 0xFF1D,
  NR34 = 0xFF1E,
  NR41 = 0xFF20,
  NR42 = 0xFF21,
  NR43 = 0xFF22,
  NR44 = 0xFF23,
  NR50 = 0xFF24,
  NR51 = 0xFF25,
  NR52 = 0xFF26,
  DAC = 0xFF1A,
  LCDC = 0xFF40,
  STAT = 0xFF41,
  SCY = 0xFF42,
  SCX = 0xFF43,
  LCDY = 0xFF44,
  LYC = 0xFF45,
  DMA = 0xFF46,
  BGP = 0xFF47,
  OBP0 = 0xFF48,
  OBP1 = 0xFF49,
  WY = 0xFF4A,
  WX = 0xFF4B,
  DISABLE_BOOT_ROM = 0xFF50,
  IE = 0xFFFF
};

void delete_mbc(MBC **mbc);
MBC initialize_mbc0(void);
MBC initialize_mbc1(CartridgeHeader ch);
MBC initialize_mbc3(CartridgeHeader ch);

uint8_t update_stat_register(uint8_t byte);
void save_data(void);
void load_save_data(char *save_location_file_name);
void destroy_memory(void);
struct RTC *get_rtc(void);
struct RTC *get_current_rtc(void);
struct RTC *get_latched_rtc(void);
