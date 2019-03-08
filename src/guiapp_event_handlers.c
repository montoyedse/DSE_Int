#include "gui/guiapp_resources.h"
#include "gui/guiapp_specifications.h"
#include "main_thread.h"
#include "sensors_thread.h"


#define APP_TIMER_ID        (101)
#define APP_TIMER_PERIOD    (10)


uint32_t my_rcv_message[4] = {0x00, 0x00, 0x00};
uint8_t contador_timer = 0;
uint32_t received_speed[6];
uint32_t received_setpoint[6];
uint32_t received_dutycycle[6];
uint32_t received_speed_prom = 0;
uint32_t received_setpoint_prom = 0;
uint32_t received_dutycycle_prom = 0;


static void update_counter(GX_WIDGET *p_widget);


UINT window1_handler(GX_WINDOW *widget, GX_EVENT *event_ptr)
{
    UINT result = gx_window_event_process(widget, event_ptr);

    tx_queue_receive(&g_values_queue, my_rcv_message, 5);

    switch (event_ptr->gx_event_type)
    {
    case GX_EVENT_SHOW:
        gx_system_timer_start(widget, APP_TIMER_ID, APP_TIMER_PERIOD,
                              APP_TIMER_PERIOD);
    break;
    case GX_EVENT_TIMER:
        if (APP_TIMER_ID == event_ptr->gx_event_payload.gx_event_timer_id)
        {
            received_speed[contador_timer] = my_rcv_message[0];
            received_setpoint[contador_timer] = my_rcv_message[1];
            received_dutycycle[contador_timer] = my_rcv_message[2];
            contador_timer ++;
            if (contador_timer > 4)
            {
                received_speed_prom = (received_speed[0] + received_speed[1] +
                        received_speed[2] + received_speed[3] + received_speed[4]) / 5;

                received_setpoint_prom = (received_setpoint[0] + received_setpoint[1] +
                        received_setpoint[2] + received_setpoint[3] + received_setpoint[4]) / 5;

                received_dutycycle_prom = (received_dutycycle[0] + received_dutycycle[1] +
                        received_dutycycle[2] + received_dutycycle[3] + received_dutycycle[4]) / 5;
                update_counter(widget->gx_widget_parent);
                contador_timer = 0;
            }
        }
    break;
    }

    return result;
}


static void update_counter(GX_WIDGET *p_widget)
{
    static GX_CHAR speed_buffer[5];
    static GX_CHAR txt_buffer[5];
    static GX_CHAR dut_buffer[4];

    GX_PROMPT *p_prompt = NULL;
    ssp_err_t err = 0;

    err = gx_widget_find(p_widget, ID_SPEED_VALUE, GX_SEARCH_DEPTH_INFINITE, (GX_WIDGET**)&p_prompt);
    if (TX_SUCCESS == err)
    {
        gx_utility_ltoa((LONG) received_speed_prom, speed_buffer, 5);

        err = gx_prompt_text_set(p_prompt, speed_buffer);
        if (err)
            while (1);
    }

    err = gx_widget_find(p_widget, ID_SETPOINT_VALUE, GX_SEARCH_DEPTH_INFINITE, (GX_WIDGET**)&p_prompt);
    if (TX_SUCCESS == err)
    {
        gx_utility_ltoa((LONG) received_setpoint_prom, txt_buffer, 5);

        err = gx_prompt_text_set(p_prompt, txt_buffer);
        if (err)
            while (1);
    }

    err = gx_widget_find(p_widget, ID_DUTY_VALUE, GX_SEARCH_DEPTH_INFINITE, (GX_WIDGET**)&p_prompt);
    if (TX_SUCCESS == err)
    {
        gx_utility_ltoa((LONG) received_dutycycle_prom, dut_buffer, 4);

        err = gx_prompt_text_set(p_prompt, dut_buffer);
        if (err)
            while (1);
    }
}
