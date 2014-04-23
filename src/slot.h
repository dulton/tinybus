#ifndef _SLOT_H_
#define _SLOT_H_

#ifdef WIN32
	#include <winsock2.h>
	typedef int		socklen_t;
	typedef SOCKET	socket_t;
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	typedef int		socket_t;
#endif

#include <stdint.h>
#include "tinybus.h"
#include "threadpool.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SLOT_MAX_MSG_NUM    256

typedef enum _slot_status
{
	SLOT_NOT_READY		= 0,
	SLOT_READ_READY		= 0X0001,
	SLOT_WRITE_READY	= 0X0002,
	SLOT_READWRITE_READ	= 0X0003
} slot_status_t;

typedef enum _msg_result_t
{
    MSG_SUCCEED = 0,
    MSG_FAILED  = 0X0001
} msg_result_t;

typedef struct _slot slot_t;
typedef msg_result_t (*msg_func_t)(slot_t *slot, tiny_msg_t *msg);

/*
 *  Each mudole should own a slot, which is used to communicate with bus
 */
struct _slot
{
	/*
	 * for slot debug
	 */
	char	        slot_name[32];
	
	/*
	 * Identify whether slot is ready to recv message
	 */ 
	slot_status_t	slot_ready;	

    /*
     * store message handler function in array msg_funcs, and array's index
     * is "message id" actually.
     * Attension: here slot's message ID is different from bus's message ID,
     * so we need a map to transform bus's message ID to slot's message ID,
     * it should be "index = msg_id - msg_delta"
     */
    uint32_t        msg_delta;
    msg_func_t      msg_funcs[SLOT_MAX_MSG_NUM];

    /*
     * contain message queue, using thread pool invoking message function to
     * process message
     */
    thread_pool_t   *msg_pool;
    
};


slot_t *
slot_new(char *name, uint32_t msg_delta, exec_t func, void *usr_data, int max_threads);

void
slot_free(slot_t *slot);

tiny_bus_result_t
slot_subscribe_message(tiny_bus_t *bus, slot_t *slot, message_id_t id, msg_func_t handler);

void
slot_unsubscribe_message(tiny_bus_t *bus, slot_t *slot, message_id_t id);

/*
 * module publish message into bus
 */
void
slot_publish(tiny_bus_t *bus, tiny_msg_t *msg);    

/*
 * bus send message into slot's msg queue
 */
tiny_bus_result_t
slot_write(slot_t *slot, tiny_msg_t *msg);

#ifdef __cplusplus
}
#endif

#endif
