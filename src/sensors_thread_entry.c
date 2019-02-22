#include "sensors_thread.h"


#define SYST_CLOCK_MS (24000)


uint8_t ui8prom_counter = 0;
uint16_t u16adc0_data = 0;
uint32_t u32Set_Point;
uint32_t ui32pwm0_dc = 0;
uint32_t ui32actual_time = 0;
uint32_t ui32past_time = 0;
uint32_t ui32elapsed_time = 0;
uint32_t ui32vel_rpm = 0;
uint32_t u32ciclo_trabajo;
ULONG my_message[4] = {0x00, 0x00, 0x00, 0x00};


/* Sensors Thread entry function */
void sensors_thread_entry(void)
{
    g_adc0.p_api->open(g_adc0.p_ctrl, g_adc0.p_cfg);
    g_adc0.p_api->scanCfg(g_adc0.p_ctrl, g_adc0.p_channel_cfg);
    g_adc0.p_api->scanStart(g_adc0.p_ctrl);

    g_timer9.p_api->open(g_timer9.p_ctrl, g_timer9.p_cfg);
    g_timer9.p_api->start(g_timer9.p_ctrl);
    g_timer9.p_api->dutyCycleSet(g_timer9.p_ctrl, 10000, TIMER_PWM_UNIT_PERCENT_X_1000, 0);

    g_external_irq5.p_api->open(g_external_irq5.p_ctrl, g_external_irq5.p_cfg);

    while (1)
    {
        g_adc0.p_api->read(g_adc0.p_ctrl, ADC_REG_CHANNEL_0, &u16adc0_data);
        u32Set_Point = (((uint32_t)u16adc0_data * 100) / 163) + 100;

        ui32pwm0_dc = (uint32_t)u16adc0_data * 2442;
        u32ciclo_trabajo = ui32pwm0_dc / 100000;

        g_timer9.p_api->dutyCycleSet(g_timer9.p_ctrl, (100000 - (ui32pwm0_dc / 100)), TIMER_PWM_UNIT_PERCENT_X_1000, 0);

        my_message[0] = ui32vel_rpm;
        my_message[1] = u32Set_Point;
        my_message[2] = u32ciclo_trabajo;

        tx_queue_send(&g_values_queue, my_message, TX_NO_WAIT);

        tx_thread_sleep(20);
    }
}


void irq_410_callback(external_irq_callback_args_t *p_args)
{
    (void) p_args;
    ui32actual_time = DWT->CYCCNT;

    if(ui32past_time > ui32actual_time)
    {
        ui32elapsed_time = ~(ui32past_time - ui32actual_time);
    }
    else
    {
        ui32elapsed_time = ui32actual_time - ui32past_time;
    }

    ui32elapsed_time = ui32elapsed_time / SYST_CLOCK_MS;

    ui32past_time = ui32actual_time;

    ui32vel_rpm = 150000 / ui32elapsed_time;
}
