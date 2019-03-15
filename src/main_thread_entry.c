/** @file main_thread_entry.c
 *  @brief Initializes peripherals to control the LCD screen.
 *
 *  Initializes the peripherals needed to control the LCD touch screen,
 *  initializes the LCD touch screen and sends the first message needed
 *  to control it from other threads.
 *
 *  @author Edgar Montoya
 *  @author Jorge Alvarado
 *  @author Lloen de Jesus
 *  @bug No known bugs.
 */

/* -- Includes -- */
#include "main_thread.h"
#include "bsp_api.h"
#include "gx_api.h"
#include "gui/guiapp_specifications.h"
#include "gui/guiapp_resources.h"

#if defined(BSP_BOARD_S7G2_SK)
#include "hardware/lcd.h"
#endif


/* -- Function prototypes -- */
void main_thread_entry(void);

#if defined(BSP_BOARD_S7G2_SK)
void g_lcd_spi_callback(spi_callback_args_t * p_args);
#endif


/* -- Global variables -- */
GX_WINDOW_ROOT * p_window_root;
GX_CONST GX_CHAR *sw_version = "20";
GX_CONST GX_CHAR *hw_version = "1";
GX_CONST GX_CHAR *programmer1 = "De Jesus, Lloen";
GX_CONST GX_CHAR *programmer2 = "Alvarado, Jorge";
GX_CONST GX_CHAR *programmer3 = "Montoya, Edgar";
GX_CONST GX_CHAR *nombre_proyecto = "Control Vel. Motor DC";


/** @brief Primary logic for handling events generated by the various sub-systems.
 *
 *
 *
 * @return Should not return
 */
void main_thread_entry(void)
{
	ssp_err_t        err;
    sf_message_header_t * p_message = NULL;

	UINT      status = TX_SUCCESS;

    /* Initializes GUIX. */
    status = gx_system_initialize();
    if(TX_SUCCESS != status)
    {
        while(1);
    }

    /* Initializes GUIX drivers. */
    err = g_sf_el_gx.p_api->open (g_sf_el_gx.p_ctrl, g_sf_el_gx.p_cfg);
    if(SSP_SUCCESS != err)
    {
        while(1);
    }

    gx_studio_display_configure ( DISPLAY_1,
                                  g_sf_el_gx.p_api->setup,
                                  LANGUAGE_ENGLISH,
                                  DISPLAY_1_THEME_1,
                                  &p_window_root );

    err = g_sf_el_gx.p_api->canvasInit(g_sf_el_gx.p_ctrl, p_window_root);
    if(SSP_SUCCESS != err)
    {
        while(1);
    }

    status = gx_studio_named_widget_create("window1", (GX_WIDGET *) p_window_root, GX_NULL);
    if(TX_SUCCESS != status)
    {
        while(1);
    }

    /* Shows the root window to make it and patients screen visible. */
    status = gx_widget_show(p_window_root);
    if(TX_SUCCESS != status)
    {
        while(1);
    }

    /* Shows the variables created by the user */
    status = gx_prompt_text_set(&window1.window1_Control_vel, nombre_proyecto);
    if(TX_SUCCESS != status)
    {
        while(1);
    }

    status = gx_prompt_text_set(&window1.window1_sw_ver_value, sw_version);
    if(TX_SUCCESS != status)
    {
        while(1);
    }

    status = gx_prompt_text_set(&window1.window1_hw_ver_value, hw_version);
    if(TX_SUCCESS != status)
    {
        while(1);
    }

    status = gx_prompt_text_set(&window1.window1_programmer1, programmer1);
    if(TX_SUCCESS != status)
    {
        while(1);
    }

    status = gx_prompt_text_set(&window1.window1_programmer2, programmer2);
    if(TX_SUCCESS != status)
    {
        while(1);
    }

    status = gx_prompt_text_set(&window1.window1_programmer3, programmer3);
    if(TX_SUCCESS != status)
    {
        while(1);
    }

    /* Lets GUIX run. */
    status = gx_system_start();
    if(TX_SUCCESS != status)
    {
        while(1);
    }


#if defined(BSP_BOARD_S7G2_SK)
    /** Open the SPI driver to initialize the LCD (SK-S7G2) **/
    err = g_spi_lcdc.p_api->open(g_spi_lcdc.p_ctrl, (spi_cfg_t *)g_spi_lcdc.p_cfg);
    if (err)
    {
        while(1);
    }
    /** Setup the ILI9341V (SK-S7G2) **/
    ILI9341V_Init();
#endif

    /* Controls the GPIO pin for LCD ON (DK-S7G2, PE-HMI1) */
#if defined(BSP_BOARD_S7G2_DK)
    err = g_ioport.p_api->pinWrite(IOPORT_PORT_07_PIN_10, IOPORT_LEVEL_HIGH);
    if (err)
    {
        while(1);
    }
#elif defined(BSP_BOARD_S7G2_PE_HMI1)
    err = g_ioport.p_api->pinWrite(IOPORT_PORT_10_PIN_03, IOPORT_LEVEL_HIGH);
    if (err)
    {
        while(1);
    }
#endif

    /* Opens PWM driver and controls the TFT panel back light (DK-S7G2, PE-HMI1) */
#if defined(BSP_BOARD_S7G2_DK) || defined(BSP_BOARD_S7G2_PE_HMI1)
    err = g_pwm_backlight.p_api->open(g_pwm_backlight.p_ctrl, g_pwm_backlight.p_cfg);
    if (err)
    {
        while(1);
    }
#endif

    err = g_sf_message0.p_api->pend(g_sf_message0.p_ctrl, &main_thread_message_queue, (sf_message_header_t **) &p_message, TX_WAIT_FOREVER);
    if (err)
    {
        while(1);
    }

	while(1)
	{

	}
}


/** @brief LCD SPI Callback function.
 *
 * Puts a semaphore when the transfer has been completed
 *
 * @param p_args pointer to the parameters of the callback function
 * @return Should not return
 */
void g_lcd_spi_callback (spi_callback_args_t * p_args)
{
    if(SPI_EVENT_TRANSFER_COMPLETE == p_args->event)
        tx_semaphore_ceiling_put(&g_main_semaphore_lcdc, 1);
}
