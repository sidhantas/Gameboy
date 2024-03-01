#include <stdint.h>

typedef struct Tracer {
    char **queue;
    int16_t head;
    int16_t tail;
    int16_t size;
    int16_t capacity;
} Tracer;
void initialize_tracer(Tracer *t, uint16_t queue_size);
void tracer_dequeue(Tracer *t);
void tracer_enqueue(Tracer *t, uint16_t pc, char str[25]);
void initialize_tracer(Tracer *t, uint16_t queue_size);
void tracer_dump(Tracer *t);
