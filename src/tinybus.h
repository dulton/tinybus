/*
 * tinybus.h
 * Copyright by Zhang Shiyong, 2014. shiyong.zhang.cn@outlook.com
 */

#ifndef _TINY_BUS_H_
#define _TINY_BUS_H_

#include <stdint.h>
#include <pthread.h>

#ifdef WIN32
#include <winsock2.h>
typedef int		socklen_t;
typedef SOCKET	socket_t;
#else
#include <sys/types.h>
#include <sys/socket.h>
typedef int		socket_t;
#endif

#include "atomic.h"
#include "asyncqueue.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TINY_BUS_MSG_EXIT		0xFFFFFFFF

#define BUS_MESSAGE_BASE_ID     0
#define BUS_MESSAGE_MAX_ID		256	// base index is 0

typedef struct _tiny_bus_msg_priv
{
	atomic_t	ref_count;
	void *		data;
	size_t		size;
} tiny_msg_priv_t;

typedef unsigned long message_id_t;
typedef struct _tiny_bus_msg
{
	uint32_t		seq;
	message_id_t	msg_id;
	
	tiny_msg_priv_t *msg_priv;
	
	/*
	 * following is used when needing to transmit context for message processing
	 */
	void *		user_data;
	size_t		user_size;
} tiny_msg_t;

typedef enum _slot_status
{
	slot_not_ready		= 0,
	slot_read_ready		= 0x0001,
	slot_write_ready	= 0x0002,
	slot_readwrite_read	= 0x0003
} slot_status_t;

/*
 *  Each subscriber should own a slot, which is used to communicate with bus
 *  a) A slot should own a message ID queue to identify its subscription
 *  b) A slot should own a message queue to contain message
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
	
    /*
	 * Contain message posted by bus, will be processed by app module.
	 * Message will be push into queue, and send to socket packet to notify
	 * receiver.
	 */
	pthread_mutex_t	*mutex;
	queue_t			*msg_queue;	
	socket_t		so;			// used to send/receive message notify
	
} slot_t;

typedef struct _tiny_bus
{
	/*
	 * bus thread
	 */
	pthread_t		*thread;	
	
	/*
	 * contain tiny_msg_t object
	 */
	async_queue_t	*msg_queue;

	/*
	 * Identify current registered message ID count
	 */
    uint32_t		msg_id_count;
	
	/*
	 * contain slot_t objects, here I use array to associate message ID and slot
	 * list, a message ID is a index of array actually, and it can be assocaited
	 * with a slot list, slot subscribed message ID will be pushed into the slot
	 * list
	 */
	pthread_mutex_t	*mutex;	
	list_t *		slots[BUS_MESSAGE_MAX_ID];
} tiny_bus_t;

typedef struct _dustbin
{
	slot_t 	*slot;
} dustbit_t;

typedef enum
{	
	TINY_BUS_FAILED	= -1,
	TINY_BUS_SUCCEED
} tiny_bus_result_t;

tiny_bus_t *
tiny_bus_new(void); // allocate a bus object

void
tiny_bus_destroy(tiny_bus_t *bus);  // destroy a bus object

slot_t *
slot_new(char *name, uint16_t port);

void
slot_free(slot_t *slot);

message_id_t
tiny_bus_alloc_msg_id(tiny_bus_t *bus);

tiny_bus_result_t
tiny_bus_init_msg_ids(tiny_bus_t *bus, message_id_t* ids, size_t size);

tiny_bus_result_t
slot_subscribe_message(tiny_bus_t *bus, slot_t *slot, message_id_t id);

void
slot_unsubscribe_message(tiny_bus_t *bus, slot_t *slot, message_id_t id);

void
tiny_bus_free_msg_id(tiny_bus_t *bus, message_id_t id);

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

/*
 * module read message from slot's msg queue
 */
tiny_msg_t *
slot_read(slot_t *slot);

#ifdef __cplusplus
}
#endif

#endif

// ~ End
