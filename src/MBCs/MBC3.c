#include "cpu.h"
#include "memory.h"
#include "utils.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_pid_t.h>
#include <unistd.h>

#define MAX_ROM_BANKS 0x80

#define SECONDS_PER_MINUTE 60
#define MINUTES_PER_HOUR 60
#define HOURS_PER_DAY 24

static struct RTC rtc = {0, 0, 0, 0, 0};
static struct RTC latched_rtc = {0, 0, 0, 0, 0};
static struct RTC *current_rtc = &rtc;

static uint8_t **rom_banks;
static uint8_t max_rom_banks;
static uint8_t **ram_banks;
static uint8_t max_ram_banks;

static bool ram_bank_and_rtc_enabled;
static bool rom_bank_enabled;
static uint8_t latch_register = 0xFF;
static uint8_t current_rom_bank;
static uint8_t ram_rtc_select;

static pthread_t rtc_id;
static bool close_rtc = false;
static void *start_rtc(void *arg);

static uint8_t mbc3_get_memory_byte(uint16_t address);
static void mbc3_set_memory_byte(uint16_t address, uint8_t byte);
static uint32_t mbc3_load_rom(FILE *rom);
static void mbc3_load_save_data(FILE *save_location);
static void mbc3_save_data(FILE *save_location);
static void destroy_mbc3(void);

MBC initialize_mbc3(CartridgeHeader ch) {
    MBC mbc3;
    mbc3.set_memory_byte = &mbc3_set_memory_byte;
    mbc3.get_memory_byte = &mbc3_get_memory_byte;
    mbc3.load_rom = &mbc3_load_rom;
    mbc3.save_data = &mbc3_save_data;
    mbc3.load_save_data = &mbc3_load_save_data;
    mbc3.destroy_memory = &destroy_mbc3;

    max_rom_banks = ch.rom_banks & 0xFF;
    max_ram_banks = ch.ram_banks;
    printf("Max Ram Banks: %d", max_ram_banks);
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
    ram_bank_and_rtc_enabled = false;
    rom_bank_enabled = false;

    pthread_create(&rtc_id, NULL, start_rtc, NULL);
    return mbc3;
}

static void update_rtc(uint8_t seconds) {
    if (seconds > 60) {
        fprintf(stderr, "Max update of 60 seconds");
        exit(1);
    }
    rtc.seconds += seconds;
    if (rtc.seconds >= SECONDS_PER_MINUTE) {
        rtc.seconds %= SECONDS_PER_MINUTE;
        rtc.minutes++;
    }
    if (rtc.minutes >= MINUTES_PER_HOUR) {
        rtc.minutes %= MINUTES_PER_HOUR;
        rtc.hours++;
    }
    if (rtc.hours >= HOURS_PER_DAY) {
        rtc.hours %= HOURS_PER_DAY;
        rtc.DL++;
        if (rtc.DL == 0x00) {
            // DL overflowed, update DH
            if (get_bit(rtc.DH, 0)) {
                set_bit(&rtc.DH, 7);
                reset_bit(&rtc.DH, 0);
            } else {
                set_bit(&rtc.DH, 0);
            }
        }
    }
}

static void *start_rtc(void *arg) {
    (void)arg;
    while (!close_rtc) {
        if (!get_bit(rtc.DH, 6)) {
            update_rtc(1);
        }
        sleep(1);
    }
    return NULL;
}

static void destroy_mbc3(void) {
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

    close_rtc = true;
    pthread_join(rtc_id, NULL);
}

static uint8_t get_rom_bank_01(void) { return current_rom_bank; }

static uint8_t mbc3_get_memory_byte(uint16_t address) {
    if (address >= ROM_BANK_00_BASE && address < ROM_BANK_NN_BASE) {
        return rom_banks[0][address];
    } else if (address >= ROM_BANK_NN_BASE && address < VRAM_BASE) {
        return rom_banks[get_rom_bank_01()][address - ROM_BANK_NN_BASE];
    } else if (address >= EX_RAM_BASE && address < WRAM_BASE) {
        /*
         * This memory location is also used for read/write access to the RTC
         */
        if (!ram_bank_and_rtc_enabled || max_ram_banks == 0) {
            return 0xFF;
        }
        if (ram_rtc_select <= 0x03) {
            return ram_banks[ram_rtc_select][address - EX_RAM_BASE];
        } else if (0x08 <= ram_rtc_select && ram_rtc_select <= 0x0C) {
            switch (ram_rtc_select) {
                case 0x08: return current_rtc->seconds; break;
                case 0x09: return current_rtc->minutes; break;
                case 0x0A: return current_rtc->hours; break;
                case 0x0B: return current_rtc->DL; break;
                case 0x0C: return current_rtc->DH; break;
            }
        }
        return 0xFF;
    } else {
        fprintf(stderr, "Unhandled memory read basic\n");
        exit(1);
    }
    return 0x00;
}

static void mbc3_set_memory_byte(uint16_t address, uint8_t byte) {
    if (address >= 0x000 && address < 0x2000) {
        if (byte == 0x0A) {
            ram_bank_and_rtc_enabled = true;
        } else if (byte == 0x00) {
            ram_bank_and_rtc_enabled = false;
        }
    } else if (address >= 0x2000 && address < 0x4000) {
        /*
         * ROM bank is set according to byte unless it's 0
         * in which case it is 1
         */
        current_rom_bank = byte & 0x7F ? byte & 0x7F : 1;
    } else if (address >= 0x4000 && address < 0x6000) {
        ram_rtc_select = byte;
    } else if (address >= 0x6000 && address < 0x8000) {
        // latch clock data
        if (0x08 > ram_rtc_select || ram_rtc_select > 0x0C) {
            return;
        }
        if (latch_register != 0x00) {
            latch_register = byte;
            current_rtc = &rtc;
            return;
        }
        if (byte != 0x01) {
            latch_register = byte;
            current_rtc = &rtc;
            return;
        }
        current_rtc = &latched_rtc;
        memcpy(&latched_rtc, &rtc, sizeof(struct RTC));
    } else if (address >= EX_RAM_BASE && address < WRAM_BASE) {
        if (!ram_bank_and_rtc_enabled || max_ram_banks == 0) {
            return;
        }
        if (ram_rtc_select <= 0x03) {
            ram_banks[ram_rtc_select][address - EX_RAM_BASE] = byte;
        } else if (0x08 <= ram_rtc_select && ram_rtc_select <= 0x0C) {
            switch (ram_rtc_select) {
                case 0x08:
                    current_rtc->seconds = byte % SECONDS_PER_MINUTE;
                    break;
                case 0x09:
                    current_rtc->minutes = byte % MINUTES_PER_HOUR;
                    break;
                case 0x0A: current_rtc->hours = byte % HOURS_PER_DAY; break;
                case 0x0B: current_rtc->DL = byte; break;
                case 0x0C: current_rtc->DH = byte; break;
            }
            return;
        }
    } else {
        fprintf(stderr, "Unhandled memory write basic\n");
        exit(1);
    }
    return;
}
static void load_rom_bank(uint8_t rom_bank_num, FILE *rom) {
    fread(rom_banks[rom_bank_num], 1, ROM_BANK_SIZE, rom);
}

static uint32_t mbc3_load_rom(FILE *rom) {
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

static void mbc3_save_data(FILE *save_location) {
    fwrite(&rtc, sizeof(struct RTC), 1, save_location);
    bool is_latched = current_rtc == &latched_rtc;
    fwrite(&latched_rtc, sizeof(struct RTC), 1, save_location);
    fwrite(&is_latched, sizeof(bool), 1, save_location);
    for (uint16_t i = 0; i < max_ram_banks; i++) {
        fwrite(ram_banks[i], 1, RAM_BANK_SIZE, save_location);
    }
}

static void mbc3_load_save_data(FILE *save_location) {
    if (!save_location) {
        return;
    }
    fread(&rtc, sizeof(struct RTC), 1, save_location);
    fread(&latched_rtc, sizeof(struct RTC), 1, save_location);
    bool is_latched;
    fread(&is_latched, sizeof(bool), 1, save_location);
    if (is_latched) {
        current_rtc = &latched_rtc;
    }
    for (uint16_t i = 0; i < max_ram_banks; i++) {
        fread(ram_banks[i], 1, RAM_BANK_SIZE, save_location);
    }
}

struct RTC *get_rtc(void) { return &rtc; }

struct RTC *get_current_rtc(void) { return current_rtc; }
struct RTC *get_latched_rtc(void) { return &latched_rtc; }
