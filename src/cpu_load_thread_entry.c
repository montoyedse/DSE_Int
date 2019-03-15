/* Calculating CPU load as a percentage with ThreadX.
 * This approach to calculating CPU loading uses 2 threads.
 *
 * Thread 1 - "CPU Load Counter Thread"
 * Simply increments a counter variable as fast as it can in an endless loop.
 * This thread should be the lowest in terms of priority in the system (higher number indicates lower priority).
 *
 * Thread 2 - "CPU Load Thread"
 * Ideally this thread should have the highest priority in the system. It runs periodically but only for a short
 * time and so should not impact performance.
 *
 * This thread first measures the value Thread 1 is able to count to over a set period of RTOS ticks
 * (CPU_LOAD_SAMPLING_TIME_TICKS) when the only threads enabled in the system are Thread 1 and Thread 2.
 *
 * This thread then resumes other threads in the system which would normally be running at start-up. Populate
 * g_threadsToResume[] with the addresses of the TCBs of the threads to resume.
 *
 * Thread 2 then takes the current Thread 1 count, suspends for CPU_LOAD_SAMPLING_TIME_TICKS ticks, takes the
 * count again when it resumes and then calculates the difference in count when the system is loaded and when it is
 * unloaded. From this the CPU loading can be calculated for that sample period.
 *
 * When running on a debugger in e2studio the chart view can be used to see a realtime view of the CPU load as the
 * system runs.
 *
 * Note:
 * It is important that only Thread 1 "CPU Load Counter Thread" and Thread 2 "CPU Load Thread" are set to auto-start
 * in the Synergy configurator. All other threads should have this setting disabled. Threads which would normally have
 * this setting enabled should be added to the g_threadsToResume[] array.
 *
 * To use this approach in a Synergy project:
 *
 * 1.  Add a new thread with the following properties modified:
 *     Symbol:      cpu_load_counter_thread_entry
 *     Name:        CPU Load Counter Thread
 *     Priority:    30 (or another value ensuring it is the lowest priority thread (higher number = lower priority)
 *     Auto start:  enabled
 *
 * 2.  Add a new thread with the following properties modified:
 *     Symbol:      cpu_load_thread_entry
 *     Name:        CPU Load Thread
 *     Priority:    1 (or another value ensuring it is the highest priority thread (lower number = highest priority)
 *                  Priority 0 is reserved for ThreadX internal use.
 *     Auto start:  enabled
 *
 * 3.  Add the contents of the supplied files "cpu_load_counter_thread_entry.c" and "cpu_load_thread_entry.c" to the
 *     generated equivalents.
 *
 * 4.  Add any other threads to the system as required ensuring their "Auto start" settings is set to "disabled".
 *
 * 5.  Add any threads which would normally auto-start to the g_threadsToResume[] array.
 */

#include "cpu_load_thread.h"

#define CPU_LOAD_SAMPLING_TIME_TICKS    5

extern uint32_t g_cpuLoadCounter;
extern TX_THREAD main_thread;
extern TX_THREAD sensors_thread;

/* Current CPU load stored here */
volatile uint32_t g_cpuLoad = 0;

TX_THREAD * g_threadsToResume[] =
{
     &main_thread,
     &sensors_thread,
};

void cpu_load_thread_entry(void);

/* CPU Load Thread entry function */
void cpu_load_thread_entry(void)
{
    /* TODO: add your own code here */
    uint32_t firstCount     = 0;
    uint32_t secondCount    = 0;
    uint32_t base_count     = 0;
    uint32_t i;
    uint32_t temp;

    /* Suspend to allow time for the CPU counter thread to run and initialise. */
    tx_thread_sleep(10);

    /* At this point the only active threads in the system should be this one and the CPU load counter thread.
     * All other threads should be configured with auto-start disabled.
     */

    /* Calculate the base count when only the lowest priority thread is running. */
    firstCount = g_cpuLoadCounter;
    tx_thread_sleep(CPU_LOAD_SAMPLING_TIME_TICKS);
    base_count = g_cpuLoadCounter - firstCount;

    /* Resume all threads as necessary */
    for(i=0; i<(sizeof(g_threadsToResume) / sizeof(TX_THREAD *)); i++)
    {
        tx_thread_resume(g_threadsToResume[i]);
    }

    while (1)
    {
        firstCount = g_cpuLoadCounter;
        tx_thread_sleep(CPU_LOAD_SAMPLING_TIME_TICKS);
        secondCount = g_cpuLoadCounter;

        if(secondCount >= firstCount)
        {
            temp = secondCount - firstCount;
        }
        else
        {
            /* Counter has overflowed */
            temp = secondCount + (0xFFFFFFFF - firstCount);
        }

        temp *= 100;
        temp = temp / base_count;
        g_cpuLoad = 100 - temp;
    }
}
