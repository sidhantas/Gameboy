#include <stdint.h>
#define MAX_OBJECTS 10

struct OAMRow {
    uint8_t x_start; // starting x position on the display
    uint16_t tile_row_index; // objects tile index in memory
};

typedef struct OAMSelector {
    struct OAMRow selected_objects[MAX_OBJECTS];
    uint8_t top;
} OAMSelector;

void initialize_oam_queue(OAMSelector *oam_selector);
void oam_add(OAMSelector *oam_selector, uint16_t object_no);
