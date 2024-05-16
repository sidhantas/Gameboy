
#include <stdint.h>
#define ADDRESS_MODE_0_BP 0x8000
#define ADDRESS_MODE_1_BP 0x9000

enum COLOR_VALUES { WHITE, LIGHT_GRAY, DARK_GRAY, BLACK, TRANSPARENT, WINDOW_OUTLINE };

uint8_t get_bg_pixel(uint8_t x, uint8_t y);
uint8_t get_win_pixel(uint8_t x, uint8_t y);
uint8_t get_obj_pixel(uint8_t x_pixel);
uint32_t get_color_from_byte(uint8_t byte);
