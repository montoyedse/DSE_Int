#include "CPU_load_counter_thread.h"

/* This thread has the lowest priority in the system and should only run when no other thread is ready to run. */

void cpu_load_counter_thread_entry(void);

/* Global counter */
volatile uint32_t g_cpuLoadCounter;

/* New Thread entry function */
void cpu_load_counter_thread_entry(void)
{
    /* TODO: add your own code here */
    g_cpuLoadCounter = 0;

    while (1)
    {
        g_cpuLoadCounter++;
    }
}
