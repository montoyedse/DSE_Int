/* generated messaging source file - do not edit */
#include "sf_message.h"
#ifndef SF_MESSAGE_CFG_QUEUE_SIZE
#define SF_MESSAGE_CFG_QUEUE_SIZE (16)
#endif
TX_QUEUE main_thread_message_queue;
static uint8_t queue_memory_main_thread_message_queue[SF_MESSAGE_CFG_QUEUE_SIZE];
sf_message_subscriber_list_t *p_subscriber_lists[] =
{ NULL };
void g_message_init(void);
void g_message_init(void)
{
    tx_queue_create (&main_thread_message_queue, (CHAR *) "Main Thread Message Queue", 1,
                     &queue_memory_main_thread_message_queue, sizeof(queue_memory_main_thread_message_queue));
}
