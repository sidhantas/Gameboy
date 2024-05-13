#include "memory.h"
#include "graphics.h"
#include "hardware.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

static MBC mbc;
static bool dmg_mapped = false;
static uint8_t *dmg;
static uint8_t *vram;
static uint8_t *wram;
static uint8_t *oam;
static uint8_t *io_ram;

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
        case 0x03: mbc = initialize_mbc1(ch); return;
        default:
            fprintf(stderr, "Cartridge type not implemented: %d\n",
                    ch.cartridge_type);
            exit(1);
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
    free(dmg);
}

void load_rom(FILE *rom) {
    CartridgeHeader ch = decode_cartridge_header(rom);
    initialize_memory(ch);
    mbc.load_rom(rom);
    map_dmg();
}

void privileged_set_memory_byte(uint16_t address, uint8_t byte) {
    if (address >= OAM_BASE && address < PROHIBITED_BASE) {
        oam[address - OAM_BASE] = byte;
    } else if (address >= IO_RAM_BASE && address <= IE) {
        io_ram[address - IO_RAM_BASE] = byte;
    } else {
        fprintf(stderr, "Invalid privileged memory access\n");
        exit(1);
    }
}

uint8_t privileged_get_memory_byte(uint16_t address) {
    if (address >= OAM_BASE && address < PROHIBITED_BASE) {
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
        return vram[address - VRAM_BASE];
    } else if (address >= EX_RAM_BASE && address < WRAM_BASE) {
        return mbc.get_memory_byte(address);
    } else if (address >= WRAM_BASE && address < ECHO_RAM_BASE) {
        return wram[address - WRAM_BASE];
    } else if (address >= ECHO_RAM_BASE && address < OAM_BASE) {
        return wram[address - 0x2000 - WRAM_BASE];
    } else if (address >= OAM_BASE && address < PROHIBITED_BASE) {
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
        case STAT: io_ram[address_offset] |= ((byte) & 0x78); return;
        case TIMA: io_ram[address_offset] += 1; return;
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
        vram[address - VRAM_BASE] = byte;
    } else if (address >= EX_RAM_BASE && address < WRAM_BASE) {
        mbc.set_memory_byte(address, byte);
    } else if (address >= WRAM_BASE && address < ECHO_RAM_BASE) {
        wram[address - WRAM_BASE] = byte;
    } else if (address >= ECHO_RAM_BASE && address < OAM_BASE) {
        wram[address - 0x2000 - WRAM_BASE] = byte;
    } else if (address >= OAM_BASE && address < PROHIBITED_BASE) {
        oam[address - OAM_BASE] = byte;
    } else if (address >= PROHIBITED_BASE && address < IO_RAM_BASE) {
        return;
    } else if (address >= IO_RAM_BASE) {
        handle_io_write(address, byte);
    }
}
