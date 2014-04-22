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

#include "tinybus.h"
#include "threadpool.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _slot_status
{
	slot_not_ready		= 0,
	slot_read_ready		= 0x0001,
	slot_write_ready	= 0x0002,
	slot_readwrite_read	= 0x0003
} slot_status_t;

/*
 *  Each mudole should own a slot, which is used to communicate with bus
 */
typedef struct _slot
{
	/*
	 * for slot debug
	 */
	char	        slot_name[32];
	
	/*
	 * Identify whether slot is ready to recv message
	 */ 
	slot_status_t	slot_ready;	

    thread_pool_t   *msg_pool;	
} slot_t;


slot_t *
slot_new(char *name, exec_t func, void *usr_data, int max_threads);

void
slot_free(slot_t *slot);

tiny_bus_result_t
slot_subscribe_message(tiny_bus_t *bus, slot_t *slot, message_id_t id);

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
