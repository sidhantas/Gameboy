#include "cpu.h"
#include "decoder.h"
#include "hardware.h"
#include "ppu.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

bool step_mode = false;
bool close_cpu = false;
bool boot_completed = false;
#define CPU_CATCH_UP 100
void *start_cpu(void *arg) {
    (void)arg;
    struct timeval start, end, diff;
    uint16_t exec_count = 0;
    suseconds_t expected_time = CPU_CATCH_UP * 1000000 / CLOCK_RATE;
    gettimeofday(&start, NULL);
    set_memory_byte(JOYP, 0x0F);
    while (true) {
        if (get_is_implemented() == false) {
            step_mode = true;
        }
        if (close_cpu) {
            break;
        }
        if (step_mode && instructions_left <= 0) {
            continue;
        } else if (step_mode) {
            instructions_left -= 1;
        }

        uint16_t old_pc = get_pc();
        clock_cycles_t (*func)(uint8_t *) = fetch_instruction();
        clock_cycles_t clocks = execute_instruction(func);
        tracer_enqueue(&t, old_pc, get_decoded_instruction());
        pthread_mutex_lock(&dots_mutex);
        ppu.available_dots += clocks * 4;
        pthread_mutex_unlock(&dots_mutex);
        exec_count += clocks;
        if (exec_count >= CPU_CATCH_UP) {
            exec_count -= CPU_CATCH_UP;
            gettimeofday(&end, NULL);
            if (end.tv_usec < start.tv_usec) {
                diff.tv_usec = 1000000 + end.tv_usec - start.tv_usec;
            } else {
                diff.tv_usec = end.tv_usec - start.tv_usec;
            }
            //suseconds_t remaining_time = expected_time - diff.tv_usec;
            usleep(15);
            gettimeofday(&start, NULL);
        }
        if (!boot_completed && get_memory_byte(0xFF50)) {
            boot_completed = true;
            unmap_dmg();
        }
        //if (get_pc() == 0x369) {
        //    step_mode = true;
        //}
    }
    return NULL;
}

void end_cpu(void) { close_cpu = true; }
