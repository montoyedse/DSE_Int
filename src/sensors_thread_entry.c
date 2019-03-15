/** @file sensors_thread_entry.c
 *  @brief Controls the motor with a PID.
 *
 *  This thread gets the Setpoint with a variable resistor and an ADC,
 *  it gets also the speed of the motor, calculates the PWM output with
 *  a PID algorithm and sends a message to another thread with the
 *  Setpoint (RPM), the Speed (RPM) and the Duty cycle (%).
 *
 *  @author Edgar Montoya
 *  @author Jorge Alvarado
 *  @author Lloen de Jesus
 *  @bug No known bugs.
 */

/* -- Includes -- */
#include "sensors_thread.h"


/* -- Defines -- */
#define PERIOD_INT_1    (15)
#define KP              (350)
#define KI              (175)
#define KD              (175)
#define SATURATION      (100000)


/* -- Global variables -- */
uint32_t ui32vel_rpm = 0;
uint32_t ui32old_vel_rpm = 0;
uint32_t ui32past_time = 0;
uint32_t u32ciclo_trabajo = 0;
uint32_t u32Set_Point = 0;
uint32_t time_counter = 0;
int32_t prev_error = 0;


/** @brief Sensors Thread entry function.
 *
 *  This is the entrypoint for the Sensors Thread entry function.
 *  Here the peripherals will be initialized and in the loop a message
 *  will be sent with the variables needed to display.
 *
 * @return Should not return
 */
void sensors_thread_entry(void)
{
    /*
     * Message with enough space to send 3 variables to other threads
     */
    uint32_t my_message[4] = {0x00, 0x00, 0x00};

    /*
     * The ADC0 is opened, configured at 12 bits continuous sampling,
     * and started
     */
    g_adc0.p_api->open(g_adc0.p_ctrl, g_adc0.p_cfg);
    g_adc0.p_api->scanCfg(g_adc0.p_ctrl, g_adc0.p_channel_cfg);
    g_adc0.p_api->scanStart(g_adc0.p_ctrl);

    /*
     * The Timer 0 is opened and started, it will interrupt every 100
     * microseconds
     */
    g_timer0.p_api->open(g_timer0.p_ctrl, g_timer0.p_cfg);
    g_timer0.p_api->start(g_timer0.p_ctrl);

    /*
     * The Timer 1 is opened and started, it will interrupt every 15
     * milliseconds
     */
    g_timer1.p_api->open(g_timer1.p_ctrl, g_timer1.p_cfg);
    g_timer1.p_api->start(g_timer1.p_ctrl);
    g_timer1.p_api->periodSet(g_timer1.p_ctrl, PERIOD_INT_1, TIMER_UNIT_PERIOD_MSEC);

    /*
     * The Timer 9 is opened and started, it will work as a PWM with a
     * frequency of 1 kHz and take the duty cycle value in percent
     * multiplied by 1000
     */
    g_timer9.p_api->open(g_timer9.p_ctrl, g_timer9.p_cfg);
    g_timer9.p_api->start(g_timer9.p_ctrl);
    g_timer9.p_api->dutyCycleSet(g_timer9.p_ctrl, 10000, TIMER_PWM_UNIT_PERCENT_X_1000, 0);

    /*
     * An external interrupt is configured to take place in pin 410 at
     * falling edge
     */
    g_external_irq5.p_api->open(g_external_irq5.p_ctrl, g_external_irq5.p_cfg);
    g_external_irq5.p_api->enable(g_external_irq5.p_ctrl);

    while (1)
    {
        /*
         * The variables are stored in the message array
         */
        my_message[0] = ui32vel_rpm;
        my_message[1] = u32Set_Point;
        my_message[2] = u32ciclo_trabajo;

        /*
         * The message is sent without waiting
         */
        tx_queue_send(&g_values_queue, my_message, TX_NO_WAIT);

        /*
         * The thread is put to sleep 50 ms
         */
        tx_thread_sleep(5);
    }
}


/** @brief External interrupt on pin410 handler function.
 *
 *  Reads the actual time and compares it with the previous lecture of
 *  time, calculates the speed in RPM with the elapsed time (ac_t - prv_t)
 *
 * @param p_args pointer to the parameters of the interrupt
 * @return Should not return
 */
void irq_410_callback(external_irq_callback_args_t *p_args)
{
    uint32_t ui32actual_time = 0;
    uint32_t ui32elapsed_time = 0;

    (void) p_args;

    ui32old_vel_rpm = ui32vel_rpm;

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
        ui32vel_rpm = ui32old_vel_rpm;
    else
    {}
}


/** @brief Interrupt callback for Timer 0.
 *
 *  Increments a variable each 100 µs
 *
 * @param p_args pointer to the parameters of the interrupt
 * @return Should not return
 */
void g_timer0_interrupt(timer_callback_args_t *p_args)
{
    (void) p_args;

    time_counter ++;
}


/** @brief Interrupt callback for Timer 1.
 *
 *  Calculates the PWM output with a PID algorithm each 15 ms
 *
 * @param p_args pointer to the parameters of the interrupt
 * @return Should not return
 */
void g_timer1_interrupt(timer_callback_args_t *p_args)
{
    uint32_t ui32pwm0x1000 = 0;
    int32_t ui32pwm0_dc = 0;
    int32_t P = 0;
    int32_t D = 0;
    int32_t I = 0;
    int32_t act_error = 0;
    uint16_t u16adc0_data = 0;

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
    if (I >= 0)
    {
        if (I >= SATURATION)
            I = SATURATION;
        else
        {}
    }
    else
    {
        if (I <= -SATURATION)
            I = -SATURATION;
        else
        {}
    }

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
