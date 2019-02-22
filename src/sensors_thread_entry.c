#include "sensors_thread.h"


uint16_t u16adc0_data = 0;
uint32_t ui32pwm0_dc = 0;


/* Sensors Thread entry function */
void sensors_thread_entry(void)
{
    g_adc0.p_api->open(g_adc0.p_ctrl, g_adc0.p_cfg);
    g_adc0.p_api->scanCfg(g_adc0.p_ctrl, g_adc0.p_channel_cfg);
    g_adc0.p_api->scanStart(g_adc0.p_ctrl);

    g_timer9.p_api->open(g_timer9.p_ctrl, g_timer9.p_cfg);
    g_timer9.p_api->start(g_timer9.p_ctrl);
    g_timer9.p_api->dutyCycleSet(g_timer9.p_ctrl, 10000, TIMER_PWM_UNIT_PERCENT_X_1000, 0);

    while (1)
    {
        g_adc0.p_api->read(g_adc0.p_ctrl, ADC_REG_CHANNEL_0, &u16adc0_data);

        ui32pwm0_dc = (uint32_t)u16adc0_data * 24;

        g_timer9.p_api->dutyCycleSet(g_timer9.p_ctrl, (100000 - ui32pwm0_dc), TIMER_PWM_UNIT_PERCENT_X_1000, 0);

        tx_thread_sleep (100);
    }
}
