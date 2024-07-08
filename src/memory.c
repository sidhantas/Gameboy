#include "memory.h"
#include "graphics.h"
#include "hardware.h"
#include "ppu.h"
#include "utils.h"
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static MBC mbc;
static bool dmg_mapped = false;
static uint8_t *dmg = NULL;
static uint8_t *vram = NULL;
static uint8_t *wram = NULL;
static uint8_t *oam = NULL;
static uint8_t *io_ram = NULL;
#define SAVE_DIR "saves"
static char save_location_filename[MAX_SAVE_DATA_NAME_SIZE];

static CartridgeHeader decode_cartridge_header(FILE *rom);

void initialize_memory(CartridgeHeader ch) {
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

    io_ram = calloc(0xFFFF - 0xFE9F, sizeof(uint8_t));
    if (!io_ram) {
        fprintf(stderr, "Unable to allocate memory for memory");
        exit(1);
    }

    switch (ch.cartridge_type) {
        case 0x00: mbc = initialize_mbc0(); return;
        case 0x01: mbc = initialize_mbc1(ch); return;
        case 0x02: mbc = initialize_mbc1(ch); return;
        case 0x03: mbc = initialize_mbc1(ch); return;
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13: mbc = initialize_mbc3(ch); return;
        default:
            fprintf(stderr, "Cartridge type not implemented: %d\n",
                    ch.cartridge_type);
            exit(1);
    }

#ifdef SKIP_BOOT
    privileged_set_memory_byte(JOYP, 0xCF);
    privileged_set_memory_byte(SC, 0x7E);
    privileged_set_memory_byte(DIV, 0xAB);
    privileged_set_memory_byte(TAC, 0xF8);
    privileged_set_memory_byte(IF, 0xE1);
    privileged_set_memory_byte(NR10, 0x80);
    privileged_set_memory_byte(NR11, 0xBF);
    privileged_set_memory_byte(NR12, 0xF3);
    privileged_set_memory_byte(NR13, 0xFF);
    privileged_set_memory_byte(NR14, 0xBF);
    privileged_set_memory_byte(NR21, 0x3F);
    privileged_set_memory_byte(NR22, 0x00);
    privileged_set_memory_byte(NR23, 0xFF);
    privileged_set_memory_byte(NR24, 0xBF);
    privileged_set_memory_byte(NR30, 0x7F);
    privileged_set_memory_byte(NR31, 0xFF);
    privileged_set_memory_byte(NR32, 0x9F);
    privileged_set_memory_byte(NR33, 0xFF);
    privileged_set_memory_byte(NR34, 0xBF);
    privileged_set_memory_byte(NR41, 0xFF);
    privileged_set_memory_byte(NR44, 0xBF);
    privileged_set_memory_byte(NR50, 0x77);
    privileged_set_memory_byte(NR51, 0xF3);
    privileged_set_memory_byte(NR52, 0xF1);
    privileged_set_memory_byte(LCDC, 0x91);
    privileged_set_memory_byte(STAT, 0x85);
    privileged_set_memory_byte(DMA, 0xFF);
    privileged_set_memory_byte(BGP, 0xFC);
#endif
}

void destroy_memory(void) {
    if (vram) {
        free(vram);
        vram = NULL;
    }
    if (wram) {
        free(wram);
        wram = NULL;
    }
    if (oam) {
        free(oam);
        oam = NULL;
    }
    if (io_ram) {
        free(io_ram);
        io_ram = NULL;
    }
    if (mbc.destroy_memory) {
        mbc.destroy_memory();
    }
}

static CartridgeHeader decode_cartridge_header(FILE *rom) {
    uint8_t buffer[0x50];
    fseek(rom, 0x100, SEEK_SET);
    fread(buffer, 0x50, 1, rom);
    CartridgeHeader ch;

    strncpy(ch.title, (char *)&buffer[0x34], MAX_TITLE_SIZE);
    ch.title[MAX_TITLE_SIZE] = '\0';

    ch.cartridge_type = buffer[0x47];
    ch.rom_banks = (uint16_t)(1 << (buffer[0x48] + 1));
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

void map_dmg(void) {
#ifdef SKIP_BOOT
    return;
#endif
    dmg = calloc(DMG_SIZE, sizeof(uint8_t));
    if (!dmg) {
        fprintf(stderr, "Unable to allocate memory for memory");
        exit(1);
    }
    FILE *dmg_file = fopen("dmg.bin", "r");
    if (!dmg_file) {
        fprintf(stderr, "No dmg present\n");
        exit(1);
    }
    unsigned long bytes_read =
        fread(&dmg[BOOT_ROM_BEGIN], DMG_SIZE, 1, dmg_file);
    if (bytes_read != 1) {
        fprintf(stderr, "Unable To Read DMG, %d\n", (int)bytes_read);
        exit(1);
    }
    dmg_mapped = true;
    fclose(dmg_file);
    return;
}

void unmap_dmg(void) {
    dmg_mapped = false;
    if (dmg) {
        free(dmg);
        dmg = NULL;
    }
}

void load_rom(FILE *rom) {
    CartridgeHeader ch = decode_cartridge_header(rom);
    initialize_memory(ch);
    uint32_t hash = mbc.load_rom(rom);
    snprintf(save_location_filename, MAX_SAVE_DATA_NAME_SIZE - 1,
             SAVE_DIR "/%s-%" PRIu32 ".sav", ch.title, hash);
    save_location_filename[MAX_SAVE_DATA_NAME_SIZE - 1] = '\0';
    load_save_data(save_location_filename);
    map_dmg();
}

void privileged_set_memory_byte(uint16_t address, uint8_t byte) {
    if (address >= VRAM_BASE && address < EX_RAM_BASE) {
        vram[address - VRAM_BASE] = byte;
    } else if (address >= OAM_BASE && address < PROHIBITED_BASE) {
        oam[address - OAM_BASE] = byte;
    } else if (address >= IO_RAM_BASE && address <= IE) {
        io_ram[address - IO_RAM_BASE] = byte;
    } else {
        fprintf(stderr, "Invalid privileged memory access\n");
        exit(1);
    }
}

uint8_t privileged_get_memory_byte(uint16_t address) {
    if (address >= VRAM_BASE && address < EX_RAM_BASE) {
        return vram[address - VRAM_BASE];
    } else if (address >= OAM_BASE && address < PROHIBITED_BASE) {
        return oam[address - OAM_BASE];
    } else if (address >= IO_RAM_BASE && address <= IE) {
        return io_ram[address - IO_RAM_BASE];
    } else {
        return get_memory_byte(address);
    }
}

void set_long_mem(uint16_t address, uint16_t val) {
    uint8_t b1, b2;
    u16_to_two_u8s(val, &b1, &b2);
    set_memory_byte(address, b1);
    set_memory_byte(address + 1, b2);
}

static uint8_t handle_io_read(uint16_t address) {
    switch (address) {
        case JOYP: return io_ram[address - IO_RAM_BASE] | 0xA0;
        case SB: return io_ram[address - IO_RAM_BASE];
        case SC: return io_ram[address - IO_RAM_BASE] | 0xFF;
        case DIV: return io_ram[address - IO_RAM_BASE];
        case TIMA: return io_ram[address - IO_RAM_BASE];
        case TMA: return io_ram[address - IO_RAM_BASE];
        case TAC: return io_ram[address - IO_RAM_BASE] | 0xF8;
        case IF: return io_ram[address - IO_RAM_BASE] | 0xE0;
        case NR10: return io_ram[address - IO_RAM_BASE] | 0x80;
        case 0xFF15: return 0xFF;
        case DAC: return io_ram[address - IO_RAM_BASE] | 0x7F;
        case NR32: return io_ram[address - IO_RAM_BASE] | 0x9F;
        case 0xFF1F: return 0xFF;
        case NR41: return 0xFF;
        case NR44: return io_ram[address - IO_RAM_BASE] | 0x3F;
        case NR52: return io_ram[address - IO_RAM_BASE] | 0x70;
        case LCDC: return io_ram[address - IO_RAM_BASE];
        case STAT: return io_ram[address - IO_RAM_BASE] | 0x80;
        case SCY: return io_ram[address - IO_RAM_BASE];
        case SCX: return io_ram[address - IO_RAM_BASE];
        case LCDY: return io_ram[address - IO_RAM_BASE];
        case LYC: return io_ram[address - IO_RAM_BASE];
        case BGP: return io_ram[address - IO_RAM_BASE];
        case OBP0: return io_ram[address - IO_RAM_BASE];
        case OBP1: return io_ram[address - IO_RAM_BASE];
        case WY: return io_ram[address - IO_RAM_BASE];
        case WX: return io_ram[address - IO_RAM_BASE];
        case IE: return io_ram[address - IO_RAM_BASE];
        default:

            if (address >= 0xFF10 && address <= 0xFF26) {
                return io_ram[address - IO_RAM_BASE];
            }
            if ((address >= 0xFF68 && address <= 0xFF6B) || address == 0xFF70 ||
                (0xFF40 <= address && address <= 0xFF4B) || address == 0xFF4F ||
                address == 0xFF50 || (0xFF51 <= address && address <= 0xFF55)) {
                return io_ram[address - IO_RAM_BASE];
            }
            return 0xFF;
    }
}

uint8_t get_memory_byte(uint16_t address) {
    if (dmg_mapped && address >= 0x00 && address < 0x100) {
        return dmg[address];
    }
    if (address >= ROM_BANK_00_BASE && address < ROM_BANK_NN_BASE) {
        return mbc.get_memory_byte(address);
    } else if (address >= ROM_BANK_NN_BASE && address < VRAM_BASE) {
        return mbc.get_memory_byte(address);
    } else if (address >= VRAM_BASE && address < EX_RAM_BASE) {
        if (ppu.mode == 3) {
            return 0xFF;
        }
        return vram[address - VRAM_BASE];
    } else if (address >= EX_RAM_BASE && address < WRAM_BASE) {
        return mbc.get_memory_byte(address);
    } else if (address >= WRAM_BASE && address < ECHO_RAM_BASE) {
        return wram[address - WRAM_BASE];
    } else if (address >= ECHO_RAM_BASE && address < OAM_BASE) {
        return wram[address - 0x2000 - WRAM_BASE];
    } else if (address >= OAM_BASE && address < PROHIBITED_BASE) {
        if (ppu.mode == 2 || ppu.mode == 3) {
            return 0xFF;
        }
        return oam[address - OAM_BASE];
    } else if (address >= PROHIBITED_BASE && address < IO_RAM_BASE) {
        uint8_t current_mode = get_mode();
        if (current_mode == 3 || current_mode == 2) {
            return 0xFF;
        }
        return 0x00;
    } else if (address >= IO_RAM_BASE && address < HRAM_BASE) {
        return handle_io_read(address);
    } else if (address >= HRAM_BASE) {
        return io_ram[address - IO_RAM_BASE];
    }

    fprintf(stderr, "Unhandled memory read\n");
    exit(1);
}

static void handle_io_write(uint16_t address, uint8_t byte) {
    uint16_t address_offset = address - IO_RAM_BASE;
    switch (address) {
        case JOYP:
            if (get_bit(~byte, 4)) {
                io_ram[address_offset] =
                    (byte & 0xF0) | (get_joypad_state() & 0x0F) | (0xC0);
            } else if (get_bit(~byte, 5)) {
                io_ram[address_offset] =
                    (byte & 0xF0) | (get_joypad_state() >> 4) | (0xC0);
            } else {
                io_ram[address_offset] = (byte & 0xF0) | (0xC0);
            }
            return;
        case DISABLE_BOOT_ROM:
            if (byte > 0) {
                unmap_dmg();
                io_ram[address_offset] = byte;
                return;
            }
        case DIV: io_ram[address_offset] = 0; return;
        case STAT: io_ram[address_offset] |= update_stat_register(byte); return;
        case TIMA: io_ram[address_offset] += 1; return;
        case LCDC: {
            uint8_t old_lcdc = io_ram[address_offset];
            if (get_bit(old_lcdc, 7) == 1 && get_bit(byte, 7) == 0) {
                ppu.mode = 0;
                ppu.current_scan_line = 0;
                set_memory_byte(LCDY, 0);
                ppu.line_x = 0;
                ppu.current_window_line = 0;
                ppu.window_rendered = false;
                ppu.line_dots = 0;
            }
            io_ram[address_offset] = byte;
            return;
        }
        case DMA:
            io_ram[address_offset] = byte;
            set_oam_dma_transfer(true);
            return;
        default: io_ram[address_offset] = byte; return;
    }
}

void set_memory_byte(uint16_t address, uint8_t byte) {
    if (address >= ROM_BANK_00_BASE && address < ROM_BANK_NN_BASE) {
        mbc.set_memory_byte(address, byte);
    } else if (address >= ROM_BANK_NN_BASE && address < VRAM_BASE) {
        mbc.set_memory_byte(address, byte);
    } else if (address >= VRAM_BASE && address < EX_RAM_BASE) {
        if (ppu.mode == 3) {
            return;
        }
        vram[address - VRAM_BASE] = byte;
    } else if (address >= EX_RAM_BASE && address < WRAM_BASE) {
        mbc.set_memory_byte(address, byte);
    } else if (address >= WRAM_BASE && address < ECHO_RAM_BASE) {
        wram[address - WRAM_BASE] = byte;
    } else if (address >= ECHO_RAM_BASE && address < OAM_BASE) {
        wram[address - 0x2000 - WRAM_BASE] = byte;
    } else if (address >= OAM_BASE && address < PROHIBITED_BASE) {
        if (ppu.mode == 2 || ppu.mode == 3) {
            return;
        }
        oam[address - OAM_BASE] = byte;
    } else if (address >= PROHIBITED_BASE && address < IO_RAM_BASE) {
        return;
    } else if (address >= IO_RAM_BASE) {
        handle_io_write(address, byte);
    }
}

void save_data(void) {
    struct stat st = {0};
    if (stat(SAVE_DIR, &st) == -1) {
        mkdir(SAVE_DIR, 0700);
    }
    FILE *save_location = fopen(save_location_filename, "w");
    if (mbc.save_data) {
        mbc.save_data(save_location);
    }
}

void load_save_data(char *save_location_file_name) {
    FILE *save_location = fopen(save_location_file_name, "r");
    if (!save_location) {
        return;
    }
    if (mbc.load_save_data) {
        mbc.load_save_data(save_location);
    }
}
