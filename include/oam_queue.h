#include "hardware.h"
#include <stdint.h>
#define MAX_OBJECTS 10

struct OAMRow {
    uint8_t x_start; // starting x position on the display
    uint16_t tile_row_index; // objects tile index in memory
};

typedef struct SpriteStore {
    struct OAMRow selected_objects[MAX_OBJECTS];
    uint8_t length;
} SpriteStore;

clock_cycles_t oam_dma_transfer(void);
void initialize_sprite_store(void);
SpriteStore *get_sprite_store(void);
void add_sprite(uint16_t object_no);
