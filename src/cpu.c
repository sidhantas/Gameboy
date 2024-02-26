#include "cpu.h"
#include "decoder.h"
#include "instructions.h"
#include "ppu.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

bool close_cpu = false;
bool step_mode = false;

void *start_cpu(void *arg) {
    (void)arg;
    struct timeval start, end, diff;
    uint16_t exec_count = 0;
    gettimeofday(&start, NULL);
    while (true) {
        if (close_cpu) {
            break;
        }
        if (step_mode) {
            continue;
        }
        clock_cycles_t (*func)(uint8_t *) = fetch_instruction();
        clock_cycles_t clocks = execute_instruction(func);
        exec_count++;
        pthread_mutex_lock(&dots_lock);
        dots += clocks * 4;
        pthread_mutex_unlock(&dots_lock);
        if (exec_count >= 10) {
            exec_count -= 10;
            gettimeofday(&end, NULL);
            timersub(&end, &start, &diff);
            usleep(10);
            gettimeofday(&start, NULL);
        }
    }
    return NULL;
}

void end_cpu(void) { close_cpu = true; }
