#include "instruction_tracer.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

void initialize_tracer(Tracer *t, uint16_t queue_size) {
    t->head = 0;
    t->tail = 0;
    t->size = 0;
    t->capacity = queue_size;
    t->queue = calloc(queue_size, sizeof(char *));

    for (int i = 0; i < t->capacity; i++) {
        t->queue[i] = calloc(300, sizeof(char));
    }
}

void tracer_dequeue(Tracer *t) {
    t->tail++;
    t->tail %= t->size;
}

void tracer_enqueue(Tracer *t, uint16_t pc, char str[100]) {
    if (t->size == t->capacity) {
        tracer_dequeue(t);
    } else {
        t->size++;
    }
    snprintf(t->queue[t->head], 300, "PC: 0x%X, %s" PRIu8, pc, str);
    t->head++;
    t->head %= t->size;
    return;
}

void tracer_dump(Tracer *t) {
    int16_t trace = t->head - 1;
    FILE *f = fopen("inst_dump.txt", "w");
    do {
        fprintf(f, "%s\n", t->queue[trace]);
        trace--;
        if (trace < 0) {
            trace = t->size - 1;
        }
    } while (trace != t->tail);
    fclose(f);
}
