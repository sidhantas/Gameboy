#include "cpu.h"
#include "decoder.h"
#include "graphics.h"
#include "hardware.h"
#include "interrupts.h"
#include "memory.h"
#include "oam_queue.h"
#include "ppu.h"
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/_types/_useconds_t.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

bool step_mode = false;
bool close_cpu = false;
bool boot_completed = false;
#define CYCLES_PER_FRAME 69905
struct timespec diff_timespec(const struct timespec *time1,
                              const struct timespec *time0) {
    assert(time1);
    assert(time0);
    struct timespec diff = {.tv_sec = time1->tv_sec - time0->tv_sec, //
                            .tv_nsec = time1->tv_nsec - time0->tv_nsec};
    if (diff.tv_nsec < 0) {
        diff.tv_nsec += 1000000000; // nsec/sec
        diff.tv_sec--;
    }
    return diff;
}
void *start_cpu(void *arg) {
    (void)arg;
    struct timespec start, end, diff, wait_time;
    uint32_t exec_count = 0;
    clock_gettime(CLOCK_REALTIME, &start);

    while (true) {
        clock_cycles_t clocks = 0;
        if (close_cpu) {
            break;
        }
        if (step_mode && instructions_left <= 0) {
            ppu.ready_to_render = true;
            continue;
        } else if (step_mode) {
            instructions_left -= 1;
        }

        if (!(get_memory_byte(IE) & get_memory_byte(IF))) {
            set_halted(false);
        }
        clocks += handle_interrupts();

        pthread_mutex_lock(&dots_mutex);
        if (!is_halted()) {
            clock_cycles_t (*func)(uint8_t *) = fetch_instruction();
            clocks += execute_instruction(func);
            clocks += try_oam_dma_transfer();
        } else {
            clocks += 4;
        }

        pthread_mutex_unlock(&dots_mutex);
        update_timer(clocks);
        run_ppu((uint16_t)clocks);
        exec_count += (uint32_t)clocks;
        if (exec_count >= CYCLES_PER_FRAME && !step_mode) {
            exec_count -= CYCLES_PER_FRAME;
            clock_gettime(CLOCK_REALTIME, &end);
            diff = diff_timespec(&end, &start);
            wait_time.tv_nsec = 13333337 - diff.tv_nsec;
            if (wait_time.tv_nsec > 0) {
                nanosleep(&wait_time, &wait_time);
            }
            clock_gettime(CLOCK_REALTIME, &start);
        }
    }
    return NULL;
}

void end_cpu(void) { close_cpu = true; }
void toggle_step_mode(void) { step_mode = !step_mode; }
bool get_step_mode(void) { return step_mode; }
