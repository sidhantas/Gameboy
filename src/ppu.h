#include <stdbool.h>
#include <stdint.h>
typedef struct PPU {
    uint16_t line_dots;
    uint8_t mode;
    bool ready_to_render;
} PPU;

extern PPU ppu;

void initialize_ppu(PPU *ppu);
void *refresh_loop(void *arg);
void ppu_update(uint8_t clocks);
void end_ppu(void);
