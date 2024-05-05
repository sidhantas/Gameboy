#include "cpu.h"
#include "decoder.h"
#include "hardware.h"
#include "interrupts.h"
#include "oam_queue.h"
#include "ppu.h"
#include <inttypes.h>
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
pthread_mutex_t cpu_mutex = PTHREAD_MUTEX_INITIALIZER;
void *start_cpu(void *arg) {
    (void)arg;
    struct timeval start, end, diff;
    uint16_t exec_count = 0;
    gettimeofday(&start, NULL);
    // FILE *f = fopen("inst_dump.txt", "w");

    while (true) {
        clock_cycles_t clocks = 0;
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

        //        fprintf(f,
        //                "A:%0.2X F:%0.2X B:%0.2X C:%0.2X D:%0.2X E:%0.2X
        //                H:%0.2X " "L:%0.2X SP:%0.4X PC:%0.4X
        //                PCMEM:%0.2X,%0.2X,%0.2X,%0.2X\n", get_register(A),
        //                get_register(F), get_register(B), get_register(C),
        //                get_register(D), get_register(E), get_register(H),
        //                get_register(L), get_sp(), get_pc(),
        //                privileged_get_memory_byte(get_pc()),
        //                privileged_get_memory_byte(get_pc() + 1),
        //                privileged_get_memory_byte(get_pc() + 2),
        //                privileged_get_memory_byte(get_pc() + 3));

        // uint16_t old_pc = get_pc();

        if (is_halted()) {
            if (get_memory_byte(IE) & get_memory_byte(IF)) {
                set_halted(false);
            }
        }
        clocks += handle_interrupts();
        if (!is_halted()) {
            clock_cycles_t (*func)(uint8_t *) = fetch_instruction();
            clocks += execute_instruction(func);
            clocks += oam_dma_transfer();
        } else {
            clocks += 4;
        }
        update_timer(clocks);
        //        snprintf(trace_str, 255,
        //                 "Instruction: 0x%0.2x 0x%0.2x 0x%0.2x %-15s A:
        //                 0x%0.2x, B: " "0x%0.2x, C: " "0x%0.2x, D: 0x%0.2x, E:
        //                 " "0x%0.2x, H: 0x%0.2x, L: 0x%0.2x, C Flag: %" PRIu8
        //                 "", get_instruction()[0], get_instruction()[1],
        //                 get_instruction()[2], get_decoded_instruction(),
        //                 get_register(A), get_register(B), get_register(C),
        //                 get_register(D), get_register(E), get_register(H),
        //                 get_register(L), get_flag(C_FLAG));
        //        tracer_enqueue(&t, old_pc, trace_str);
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
            // suseconds_t remaining_time = expected_time - diff.tv_usec;
            usleep(10);
            gettimeofday(&start, NULL);
        }
    }
    return NULL;
}

void end_cpu(void) { close_cpu = true; }
void toggle_step_mode(void) { step_mode = !step_mode; }
