#include "tinybus.h"
#include "message.h"

void
main(int argc, char **argv)
{
    tiny_bus_t   *bus;
    message_id_t msg_arr[] = 
    {
        BUS_MSG_EXIT,
        BUS_MSG_TRACE,
        BUS_MSG_TIMER
    };
    
    bus = tiny_bus_new();

    tiny_bus_init_msg_ids(bus, msg_arr, sizeof(msg_arr)/sizeof(msg_arr[0])); 

    return;
}
