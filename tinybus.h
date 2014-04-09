/*
 * tinybus.h
 * Copyright by Zhang Shiyong, 2014. shiyong.zhang.cn@outlook.com
 */

#ifndef _TINY_BUS_H_
#define _TINY_BUS_H_

#include <sys/types.h>
#include <stdint.h>
#include "atomic.h"
#include "asyncqueue.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TINY_BUS_MSG_EXIT		0xFFFFFFFF
#define TINY_BUS_USER_MSG_BASE	0x00000000

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
	 * Messages can be posted to slot, it'multiple message IDs list(subscription);
	 * when posting message, bus will check whether message id is in queue, true
	 * then insert into slot's member: msg_queue.
	 */
	pthread_mutex_t	*mutex;
	list_t			*subscription;	
	
	/*
	 * for slot debug
	 */
	unsigned char	slot_name[32];
	
	/*
	 * Identify whether slot is ready to recv message
	 */ 
	slot_status_t	slot_ready;
	
    /*
	 * contain message posted by bus, will be processed by app module
	 */
	async_queue_t	*msg_queue;
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
	 * contain slot_t object
	 */
	pthread_mutex_t	*mutex;
	list_t			*slot_list;
} tiny_bus_t;

typedef struct _publisher
{
	slot_t			*slot;
	//...
} publisher_t;

typedef struct _subscriber
{
	slot_t			*slot;
	
	//...
} subscriber_t;

typedef struct _dustbin
{
	subscriber_t 	*subscriber;
} dustbit_t;

typedef enum
{	
	TINY_BUS_FAILED	= -1,
	TINY_BUS_SUCCEED
} tiny_bus_result_t;

tiny_bus_t* tiny_bus_new(void);	// allocate a bus object
void tiny_bus_destroy(tiny_bus_t *bus);	// destroy a bus object

tiny_bus_result_t	tiny_bus_add_slot(tiny_bus_t *bus, slot_t *slot);
tiny_bus_result_t	tiny_bus_del_slot(tiny_bus_t *bus, slot_t *slot);

slot_t* slot_new(char *name);
void slot_free(slot_t *slot);

#ifdef __cplusplus
}
#endif

#endif

// ~ End