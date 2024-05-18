#include "hardware.h"
#include "memory.h"
#include "oam_queue.h"
#include "utils.h"
#include <string.h>

SpriteStore sprite_store;

void initialize_sprite_store(void) {
    sprite_store.length = 0;
    memset(sprite_store.selected_objects, 0,
           MAX_OBJECTS * sizeof(struct ObjectRowData));
    return;
}

SpriteStore *get_sprite_store(void) { return &sprite_store; }

clock_cycles_t try_oam_dma_transfer(void) {
    static uint8_t current_oam_byte = 0;
    if (current_oam_byte == OAM_SIZE) {
        current_oam_byte %= OAM_SIZE;
        set_oam_dma_transfer(false);
    }
    if (!get_oam_dma_transfer()) {
        return 0;
    }
    uint16_t start = (uint16_t)(get_memory_byte(DMA) << 8);
    privileged_set_memory_byte(OAM_START + current_oam_byte, get_memory_byte(start + current_oam_byte));
    current_oam_byte++;
    return FOUR_CLOCKS;
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

uint16_t get_tile_row_address(uint8_t tile_index) {
    uint16_t tile_address = 0x8000 + (uint16_t)(tile_index << 4);
    return tile_address;
}

void add_sprite(uint16_t object_no) {
    if (sprite_store.length == MAX_OBJECTS) {
        return;
    }
    object_t obj = get_object(object_no);
    uint8_t obj_h = get_bit(get_memory_byte(LCDC), 2) ? 16 : 8;
    uint16_t current_draw_height = get_memory_byte(LCDY) + 16;

    if (obj.y_pos > current_draw_height ||
        current_draw_height >= obj.y_pos + obj_h) {
        return;
    }
    uint8_t tile_index;
    bool y_flipped = get_bit(obj.attribute_flags, 6);

    if (obj_h == 16) {
        if (!y_flipped) {
            tile_index = (current_draw_height - obj.y_pos >= 8)
                             ? obj.tile_index | 0x01
                             : obj.tile_index & 0xFE;
        } else {
            tile_index = (current_draw_height - obj.y_pos >= 8)
                             ? obj.tile_index & 0xFE
                             : obj.tile_index | 0x01;
        }
    } else {
        tile_index = obj.tile_index;
    }
    sprite_store.selected_objects[sprite_store.length].x_start = obj.x_pos;
    sprite_store.selected_objects[sprite_store.length].tile_start =
        get_tile_row_address(tile_index);
    sprite_store.selected_objects[sprite_store.length].y =
        get_memory_byte(LCDY) - obj.y_pos % obj_h;
    sprite_store.selected_objects[sprite_store.length].x_flipped =
        get_bit(obj.attribute_flags, 5);
    sprite_store.selected_objects[sprite_store.length].y_flipped = y_flipped;
    sprite_store.selected_objects[sprite_store.length].DMG_palette =
        get_bit(obj.attribute_flags, 4);
    sprite_store.selected_objects[sprite_store.length].priority =
        get_bit(obj.attribute_flags, 7);
    sprite_store.length++;
    return;
}
