#include "oam_queue.h"
#include "utils.h"
#include <string.h>


SpriteStore sprite_store;

void initialize_sprite_store(void) {
    sprite_store.length = 0;
    memset(sprite_store.selected_objects, 0,
           MAX_OBJECTS * sizeof(struct OAMRow));
    return;
}

SpriteStore *get_sprite_store(void) {
    return &sprite_store;
}

clock_cycles_t oam_dma_transfer(void) {
    if (!get_oam_dma_transfer()) {
        return 0;
    }
    uint16_t start = get_memory_byte(DMA) << 8;
    for (uint16_t i = 0; i < OAM_SIZE; i++) {
        privileged_set_memory_byte(OAM_START + i, get_memory_byte(start + i));
    }

    set_oam_dma_transfer(false);
    return 160;
}

typedef struct Object {
    uint8_t y_pos;
    uint8_t x_pos;
    uint8_t tile_index;
    uint8_t attribute_flags;
} object_t;

object_t get_object(uint16_t object_index) {
    uint16_t object_address = object_index * 4 + OAM_START;
    object_t obj = {
        get_memory_byte(object_address),
        get_memory_byte(object_address + 1),
        get_memory_byte(object_address + 2),
        get_memory_byte(object_address + 3),
    };
    return obj;
}

uint16_t get_tile_row_address(object_t obj, uint8_t row) {
    uint16_t tile_address = 0x8000 + obj.tile_index & 0xFE;
    uint16_t row_address = tile_address + row * 2;
    return row_address;
}

void add_sprite(uint16_t object_no) {
    if (sprite_store.length == MAX_OBJECTS) {
        return;
    }
    object_t obj = get_object(object_no);
    uint8_t obj_h = get_bit(get_memory_byte(LCDC), 2) ? 16 : 8;
    if (!(obj.y_pos <= get_memory_byte(LCDY) + 16 < obj.y_pos + obj_h)) {
        return;
    }
    sprite_store.selected_objects[sprite_store.length].x_start = obj.x_pos;
    sprite_store.selected_objects[sprite_store.length].tile_row_index =
        get_tile_row_address(obj, obj_h);
    sprite_store.length++;
    return;
}
