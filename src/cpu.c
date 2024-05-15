#include "cpu.h"
#include "decoder.h"
#include "hardware.h"
#include "interrupts.h"
#include "memory.h"
#include "oam_queue.h"
#include "ppu.h"
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
#define CYCLES_PER_FRAME 35000
void *start_cpu(void *arg) {
    (void)arg;
    struct timespec start, end, diff;
    uint32_t exec_count = 0;
    clock_gettime(CLOCK_REALTIME, &start);
    // FILE *f = fopen("inst_dump.txt", "w");

    while (true) {
        clock_cycles_t clocks = 0;
        if (get_pc() == 0x48) {
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

        // fprintf(f,
        //         "A:%0.2X F:%0.2X B:%0.2X C:%0.2X D:%0.2X E:%0.2X H:%0.2X "
        //         "L:%0.2X SP:%0.4X PC:%0.4X PCMEM:%0.2X,%0.2X,%0.2X,%0.2X\n",
        //         get_register(A), get_register(F), get_register(B),
        //         get_register(C), get_register(D), get_register(E),
        //         get_register(H), get_register(L), get_sp(), get_pc(),
        //         get_memory_byte(get_pc()), get_memory_byte(get_pc() + 1),
        //         get_memory_byte(get_pc() + 2), get_memory_byte(get_pc() +
        //         3));
        clocks += handle_interrupts();

        pthread_mutex_lock(&dots_mutex);
        if (!is_halted()) {
            clock_cycles_t (*func)(uint8_t *) = fetch_instruction();
            clocks += execute_instruction(func);
            clocks += try_oam_dma_transfer();
        } else {
            clocks += 4;
        }

        ppu.available_dots += clocks;
        pthread_mutex_unlock(&dots_mutex);
        update_timer(clocks);
        run_ppu((uint16_t)clocks);
        //         snprintf(trace_str, 255,
        //                  "Instruction: 0x%0.2x 0x%0.2x 0x%0.2x %-15s A:
        //                  0x%0.2x, B: " "0x%0.2x, C: " "0x%0.2x, D: 0x%0.2x,
        //                  E: " "0x%0.2x, H: 0x%0.2x, L: 0x%0.2x, C Flag: %"
        //                  PRIu8
        //                  "", get_instruction()[0], get_instruction()[1],
        //                  get_instruction()[2], get_decoded_instruction(),
        //                  get_register(A), get_register(B), get_register(C),
        //                  get_register(D), get_register(E), get_register(H),
        //                  get_register(L), get_flag(C_FLAG));
        //         tracer_enqueue(&t, old_pc, trace_str);
        exec_count += (uint32_t)clocks;
                if (exec_count >= CYCLES_PER_FRAME && !step_mode) {
                    exec_count -= CYCLES_PER_FRAME;
                    clock_gettime(CLOCK_REALTIME, &end);
                    diff.tv_nsec = end.tv_nsec - start.tv_nsec;
                    if (diff.tv_nsec < 0) {
                        diff.tv_nsec += 1000000000;
                    }
        
                    long remaining_time = 8888888L - diff.tv_nsec;
                    usleep((useconds_t)remaining_time / 2500);
                    clock_gettime(CLOCK_REALTIME, &start);
                }
    }
    return NULL;
}

void end_cpu(void) { close_cpu = true; }
void toggle_step_mode(void) { step_mode = !step_mode; }
