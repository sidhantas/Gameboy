#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct PPU {
    uint16_t available_dots;
    uint64_t consumed_dots;
    uint16_t line_dots;
    uint8_t line_x;
    uint8_t current_scan_line;
    uint8_t mode;
    bool ready_to_render;
} PPU;

extern PPU ppu;
extern pthread_mutex_t display_buffer_mutex;
extern pthread_mutex_t dots_mutex;

void initialize_ppu(void);
void run_ppu(uint16_t dots);
void *start_ppu(void *arg);
void render_loop(void);
uint8_t get_x_pixel(void);
uint8_t get_y_pixel(void);
void end_ppu(void);
