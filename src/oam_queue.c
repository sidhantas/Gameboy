#include "oam_queue.h"
#include "hardware.h"
#include "utils.h"
#include <string.h>

void initialize_oam_queue(OAMSelector oam_selector[MAX_OBJECTS]) {
    oam_selector->top = 0;
    memset(oam_selector->selected_objects, 0, MAX_OBJECTS * sizeof(struct OAMRow));
    return;
}

typedef struct Object {
    uint8_t y_pos;
    uint8_t x_pos;
    uint8_t tile_index;
    uint8_t attribute_flags;
} object_t;

object_t get_object(uint16_t object_index) {
    uint16_t object_address = object_index * 4 + OAM;
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

void oam_add(OAMSelector *oam_selector, uint16_t object_no) {
    if (oam_selector->top == MAX_OBJECTS) {
        return;
    }
    object_t obj = get_object(object_no);
    uint8_t obj_h = get_bit(get_memory_byte(LCDC), 2) ? 16 : 8;
    if (!(obj.y_pos <= get_memory_byte(LCDY) + 16 < obj.y_pos + obj_h)) {
        return;
    }
    oam_selector->selected_objects[oam_selector->top].x_start = obj.x_pos;
    oam_selector->selected_objects[oam_selector->top].tile_row_index = get_tile_row_address(obj, obj_h);
    oam_selector->top++;
    return;
}
