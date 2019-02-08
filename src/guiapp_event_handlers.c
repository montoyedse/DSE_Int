#include "gui/guiapp_resources.h"
#include "gui/guiapp_specifications.h"
#include "main_thread.h"


#define APP_TIMER_ID        (101)
#define APP_TIMER_PERIOD    (100)
#define C_FILTER_ORDER (int)128


uint16_t u16ADC_Data;
uint16_t u16ADC_FilteredData;
uint32_t u32ciclo_trabajo;


static void update_counter(GX_WIDGET * p_widget, GX_RESOURCE_ID id);


UINT window1_handler(GX_WINDOW *widget, GX_EVENT *event_ptr)
{
    UINT result = gx_window_event_process(widget, event_ptr);

    switch (event_ptr->gx_event_type)
    {
    default:
        gx_window_event_process(widget, event_ptr);
        break;
    }

    if (GX_EVENT_SHOW == event_ptr->gx_event_type)
    {
        gx_system_timer_start(widget, APP_TIMER_ID, APP_TIMER_PERIOD,
                              APP_TIMER_PERIOD);
    }

    else if (GX_EVENT_TIMER == event_ptr->gx_event_type)
    {
        if (APP_TIMER_ID == event_ptr->gx_event_payload.gx_event_timer_id)
        {
            update_counter(widget->gx_widget_parent, ID_SETPOINT_VALUE);
        }
    }

    return result;
}


static void update_counter(GX_WIDGET * p_widget, GX_RESOURCE_ID id)
{
    g_adc0.p_api->read(g_adc0.p_ctrl, ADC_REG_CHANNEL_0, &u16ADC_Data);

    GX_PROMPT * p_prompt = NULL;

    static GX_CHAR txt_buffer[5];

    ssp_err_t err = gx_widget_find(p_widget, id, GX_SEARCH_DEPTH_INFINITE, (GX_WIDGET**)&p_prompt);
    if (TX_SUCCESS == err)
    {
        gx_utility_ltoa((LONG) u16ADC_Data, txt_buffer, 5);

        err = gx_prompt_text_set(p_prompt, txt_buffer);
        if (err)
            while (1);
    }
}
