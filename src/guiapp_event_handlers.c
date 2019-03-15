/** @file guiapp_event_handlers.c
 *  @brief Controls the behavior of the LCD screen.
 *
 *  Controls the refresh rate and behavior of the LCD touch screen.
 *  Receives a message with the Speed, Duty cycle and Setpoint
 *  information, calculates the average of the received message
 *  every second and converts the data to string and shows the
 *  info in the corresponding space on the screen.
 *
 *  @author Edgar Montoya
 *  @author Jorge Alvarado
 *  @author Lloen de Jesus
 *  @bug No known bugs.
 */

/* -- Includes -- */
#include "gui/guiapp_resources.h"
#include "gui/guiapp_specifications.h"
#include "main_thread.h"
#include "sensors_thread.h"


/* -- Defines -- */
#define APP_TIMER_ID        (101)
#define APP_TIMER_PERIOD    (10)


/* -- Function prototypes -- */
static void update_counter(GX_WIDGET *p_widget);


/* -- Global variables -- */
uint32_t received_speed_prom = 0;
uint32_t received_setpoint_prom = 0;
uint32_t received_dutycycle_prom = 0;
uint32_t rcv_msg[4] = {0x00, 0x00, 0x00};
uint32_t received_speed[6];
uint32_t received_setpoint[6];
uint32_t received_dutycycle[6];
uint8_t contador_timer = 0;


/** @brief Handles events directed to the LCD touch screen.
 *
 * Creates a timer event with a period of 100 ms when the program starts
 * Each time there is a timer event it receives a message with the values
 * to show, then the values are stored in arrays, each 5 lectures an
 * average of the values is calculated and the update values function
 * is called
 *
 * @param widget pointer to the top widget in the window
 * @param event_ptr pointer to the guix event
 * @return Should not return
 */
UINT window1_handler(GX_WINDOW *widget, GX_EVENT *event_ptr)
{
    UINT result = gx_window_event_process(widget, event_ptr);

    tx_queue_receive(&g_values_queue, rcv_msg, 5);

    switch (event_ptr->gx_event_type)
    {
    case GX_EVENT_SHOW:
        gx_system_timer_start(widget, APP_TIMER_ID, APP_TIMER_PERIOD,
                              APP_TIMER_PERIOD);
    break;
    case GX_EVENT_TIMER:
        if (APP_TIMER_ID == event_ptr->gx_event_payload.gx_event_timer_id)
        {
            received_speed[contador_timer] = rcv_msg[0];
            received_setpoint[contador_timer] = rcv_msg[1];
            received_dutycycle[contador_timer] = rcv_msg[2];

            contador_timer ++;
            if (contador_timer > 4)
            {
                received_speed_prom = (received_speed[0] + received_speed[1] +
                        received_speed[2] + received_speed[3] + received_speed[4]) / 5;

                received_setpoint_prom = (received_setpoint[0] + received_setpoint[1] +
                        received_setpoint[2] + received_setpoint[3] + received_setpoint[4]) / 5;

                received_dutycycle_prom = (received_dutycycle[0] + received_dutycycle[1] +
                        received_dutycycle[2] + received_dutycycle[3] + received_dutycycle[4]) / 5;

                contador_timer = 0;

                update_counter(widget->gx_widget_parent);
            }
            else
            {}
        }
        else
        {}
    break;
    }

    return result;
}


/** @brief Updates the values on the screen.
 *
 * Finds the proper widget, converts the values to strings and updates
 * the values on the corresponding spaces
 *
 * @param p_widget pointer to the parent widget
 * @return Should not return
 */
static void update_counter(GX_WIDGET *p_widget)
{
    static GX_CHAR speed_txt[5];
    static GX_CHAR setpoint_txt[5];
    static GX_CHAR ducy_txt[4];

    GX_PROMPT *p_prompt = NULL;
    ssp_err_t err = 0;

    err = gx_widget_find(p_widget, ID_SPEED_VALUE, GX_SEARCH_DEPTH_INFINITE, (GX_WIDGET**)&p_prompt);
    if (TX_SUCCESS == err)
    {
        gx_utility_ltoa((LONG) received_speed_prom, speed_txt, 5);

        err = gx_prompt_text_set(p_prompt, speed_txt);
        if (err)
            while (1);
    }

    err = gx_widget_find(p_widget, ID_SETPOINT_VALUE, GX_SEARCH_DEPTH_INFINITE, (GX_WIDGET**)&p_prompt);
    if (TX_SUCCESS == err)
    {
        gx_utility_ltoa((LONG) received_setpoint_prom, setpoint_txt, 5);

        err = gx_prompt_text_set(p_prompt, setpoint_txt);
        if (err)
            while (1);
    }

    err = gx_widget_find(p_widget, ID_DUTY_VALUE, GX_SEARCH_DEPTH_INFINITE, (GX_WIDGET**)&p_prompt);
    if (TX_SUCCESS == err)
    {
        gx_utility_ltoa((LONG) received_dutycycle_prom, ducy_txt, 4);

        err = gx_prompt_text_set(p_prompt, ducy_txt);
        if (err)
            while (1);
    }
}
