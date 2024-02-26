#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct PPU {
    uint16_t line_dots;
    uint8_t mode;
    bool ready_to_render;
} PPU;

extern PPU ppu;
extern pthread_mutex_t dots_lock;
extern pthread_mutex_t display_buffer_lock;
extern uint64_t dots;
extern uint64_t consumed_dots;

void initialize_ppu(PPU *ppu);
void *start_ppu(void *arg);
void end_ppu(void);
