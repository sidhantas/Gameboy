#include "hardware.h"
#include <stdint.h>
#define MAX_OBJECTS 10

struct ObjectRowData {
    uint8_t x_start; // starting x position on the display
    uint8_t y;
    uint16_t tile_start; // objects tile index in memory
    bool x_flipped;
    bool y_flipped;
    bool priority;
    uint8_t DMG_palette;
};

typedef struct SpriteStore {
    struct ObjectRowData selected_objects[MAX_OBJECTS];
    uint8_t length;
} SpriteStore;

clock_cycles_t try_oam_dma_transfer(void);
void initialize_sprite_store(void);
SpriteStore *get_sprite_store(void);
void add_sprite(uint16_t object_no);
