#pragma once
#include <stdint.h>
void open_window(void);
void close_window(void);
void update_pixel_buff(uint16_t dots, uint16_t exec_count);
uint16_t draw_pixel_buff(uint16_t dots);
