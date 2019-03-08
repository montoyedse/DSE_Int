#include "sensors_thread.h"


#define SYST_CLOCK_MS   (24000)
#define KP  200.0
#define KD  40.0
#define KI  120.0


uint16_t u16adc0_data = 0;
uint32_t u32Set_Point = 0;
uint32_t ui32pwm0_dc = 0;
uint32_t ui32pwm0x1000 = 0;
uint32_t ui32actual_time = 0;
uint32_t ui32past_time = 0;
uint32_t ui32elapsed_time = 0;
uint32_t ui32vel_rpm = 0;
uint32_t u32ciclo_trabajo = 0;
uint32_t my_message[4] = {0x00, 0x00, 0x00};
uint32_t time_counter = 0;
float act_error = 0.0;
float prev_error = 0.0;
float P = 0.0;
float D = 0.0;
float I = 0.0;

/* Sensors Thread entry function */
void sensors_thread_entry(void)
{
    g_adc0.p_api->open(g_adc0.p_ctrl, g_adc0.p_cfg);
    g_adc0.p_api->scanCfg(g_adc0.p_ctrl, g_adc0.p_channel_cfg);
    g_adc0.p_api->scanStart(g_adc0.p_ctrl);

    g_timer0.p_api->open(g_timer0.p_ctrl, g_timer0.p_cfg);
    g_timer0.p_api->start(g_timer0.p_ctrl);

    g_timer9.p_api->open(g_timer9.p_ctrl, g_timer9.p_cfg);
    g_timer9.p_api->start(g_timer9.p_ctrl);
    g_timer9.p_api->dutyCycleSet(g_timer9.p_ctrl, 10000, TIMER_PWM_UNIT_PERCENT_X_1000, 0);

    g_external_irq5.p_api->open(g_external_irq5.p_ctrl, g_external_irq5.p_cfg);
    g_external_irq5.p_api->enable(g_external_irq5.p_ctrl);

    while (1)
    {
        g_adc0.p_api->read(g_adc0.p_ctrl, ADC_REG_CHANNEL_0, &u16adc0_data);
        u32Set_Point = (((uint32_t)u16adc0_data * 100) / 151) + 100;

        act_error = (float)u32Set_Point - (float)ui32vel_rpm;

        P = ((float)KP * act_error);
        I = ((float)KI * (act_error + prev_error));
        D = ((float)KD * (act_error - prev_error));

        ui32pwm0_dc = (uint32_t)(P + I + D);
        if (ui32pwm0_dc > 100000)
            ui32pwm0_dc = 100000;
        else
        {}

        u32ciclo_trabajo = ui32pwm0_dc / 1000;
        ui32pwm0x1000 = 100000 - ui32pwm0_dc;

        g_timer9.p_api->dutyCycleSet(g_timer9.p_ctrl, ui32pwm0x1000, TIMER_PWM_UNIT_PERCENT_X_1000, 0);
//        g_timer9.p_api->dutyCycleSet(g_timer9.p_ctrl, 0, TIMER_PWM_UNIT_PERCENT_X_1000, 0);

        prev_error = act_error;

        my_message[0] = ui32vel_rpm;
        my_message[1] = u32Set_Point;
        my_message[2] = u32ciclo_trabajo;

        tx_queue_send(&g_values_queue, my_message, TX_NO_WAIT);

        tx_thread_sleep(10);
    }
}


void irq_410_callback(external_irq_callback_args_t *p_args)
{
    (void) p_args;

    ui32actual_time = time_counter;
    if(ui32past_time > ui32actual_time)
    {
        ui32elapsed_time = ~(ui32past_time - ui32actual_time);
    }
    else
    {
        ui32elapsed_time = ui32actual_time - ui32past_time;
    }

    ui32past_time = ui32actual_time;
    ui32vel_rpm = 150000 / ui32elapsed_time;
    if (ui32vel_rpm > 3200)
        ui32vel_rpm = 3200;
}


void g_timer0_interrupt(timer_callback_args_t *p_args)
{
    (void) p_args;

    time_counter ++;
}
