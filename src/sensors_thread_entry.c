#include "sensors_thread.h"


#define PERIOD_INT_1    (15)
#define KP              (350)
#define KI              (60)
#define KD              (30)
#define SATURATION      (100000)

uint16_t counter_prom = 0;
uint16_t u16adc0_data = 0;
uint32_t u32Set_Point = 0;
int32_t ui32pwm0_dc = 0;
uint32_t ui32pwm0x1000 = 0;
uint32_t ui32actual_time = 0;
uint32_t sample_time = 0;
uint32_t ui32past_time = 0;
uint32_t ui32elapsed_time = 0;
uint32_t ui32vel_rpm = 0;
uint32_t ui32vel_rpm_prom = 0;
uint32_t ui32vel_rpm_arr[35];
uint32_t u32ciclo_trabajo = 0;
uint32_t my_message[4] = {0x00, 0x00, 0x00};
uint32_t time_counter = 0;
int32_t act_error = 0;
int32_t prev_error = 0;
int32_t P = 0;
int32_t D = 0;
int32_t I = 0;
//volatile bool completed = false;


/* Sensors Thread entry function */
void sensors_thread_entry(void)
{
//    uint8_t contador = 0;

    g_adc0.p_api->open(g_adc0.p_ctrl, g_adc0.p_cfg);
    g_adc0.p_api->scanCfg(g_adc0.p_ctrl, g_adc0.p_channel_cfg);
    g_adc0.p_api->scanStart(g_adc0.p_ctrl);

    g_timer0.p_api->open(g_timer0.p_ctrl, g_timer0.p_cfg);
    g_timer0.p_api->start(g_timer0.p_ctrl);

    g_timer1.p_api->open(g_timer1.p_ctrl, g_timer1.p_cfg);
    g_timer1.p_api->start(g_timer1.p_ctrl);
    g_timer1.p_api->periodSet(g_timer1.p_ctrl, PERIOD_INT_1, TIMER_UNIT_PERIOD_MSEC);

    g_timer9.p_api->open(g_timer9.p_ctrl, g_timer9.p_cfg);
    g_timer9.p_api->start(g_timer9.p_ctrl);
    g_timer9.p_api->dutyCycleSet(g_timer9.p_ctrl, 10000, TIMER_PWM_UNIT_PERCENT_X_1000, 0);

    g_external_irq5.p_api->open(g_external_irq5.p_ctrl, g_external_irq5.p_cfg);
    g_external_irq5.p_api->enable(g_external_irq5.p_ctrl);

    while (1)
    {
//        if (completed)
//        {

//            for (contador = 0; contador < counter_prom; contador ++)
//            {
//                ui32vel_rpm_prom = ui32vel_rpm_prom + ui32vel_rpm_arr[contador];
//            }
//
//            ui32vel_rpm_prom = ui32vel_rpm_prom / counter_prom;

//            act_error = (float)u32Set_Point - (float)ui32vel_rpm_prom;

            my_message[0] = ui32vel_rpm;
            my_message[1] = u32Set_Point;
            my_message[2] = u32ciclo_trabajo;

//            ui32vel_rpm = 0;

//            ui32vel_rpm_prom = 0;

            tx_queue_send(&g_values_queue, my_message, TX_NO_WAIT);

//            counter_prom = 0;
//            completed = false;
//        }
//        else
//        {}

        tx_thread_sleep(5);
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


void g_timer1_interrupt(timer_callback_args_t *p_args)
{
    (void) p_args;

    g_adc0.p_api->read(g_adc0.p_ctrl, ADC_REG_CHANNEL_0, &u16adc0_data);
    u32Set_Point = (((uint32_t)u16adc0_data * 100) / 273) + 1000;

    act_error = (int32_t)u32Set_Point - (int32_t)ui32vel_rpm;

    P = KP * act_error;
    if (P >= 0)
    {
        if (P >= SATURATION)
            P = SATURATION;
        else
        {}
    }
    else
    {
        if (P <= -SATURATION)
            P = -SATURATION;
        else
        {}
    }

    I = KI * (act_error + prev_error);

    D = KD * (act_error - prev_error);
    if (D >= 0)
    {
        if (D > SATURATION)
            D = SATURATION;
        else
        {}
    }
    else
    {
        if (D <= -SATURATION)
            D = 0;
        else
            D = 100000 - D;
    }

    ui32pwm0_dc = (P + I + D);

    if (ui32pwm0_dc >= 0)
    {
        if (ui32pwm0_dc >= SATURATION)
                ui32pwm0_dc = SATURATION;
        else
        {}
    }
    else
    {
        ui32pwm0_dc = 0;
    }

    u32ciclo_trabajo = (uint32_t)ui32pwm0_dc / 1000;
    ui32pwm0x1000 = 100000 - (uint32_t)ui32pwm0_dc;

    g_timer9.p_api->dutyCycleSet(g_timer9.p_ctrl, ui32pwm0x1000, TIMER_PWM_UNIT_PERCENT_X_1000, 0);

    prev_error = act_error;
}
