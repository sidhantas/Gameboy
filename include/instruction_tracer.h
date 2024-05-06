#include <stdint.h>

typedef struct Tracer {
    char **queue;
    uint16_t head;
    uint16_t tail;
    uint16_t size;
    uint16_t capacity;
} Tracer;
void initialize_tracer(Tracer *t, uint16_t queue_size);
void tracer_dequeue(Tracer *t);
void tracer_enqueue(Tracer *t, uint16_t pc, char str[100]);
void initialize_tracer(Tracer *t, uint16_t queue_size);
void tracer_dump(Tracer *t);
