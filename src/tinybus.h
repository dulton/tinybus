/*
 * tinybus.h
 * Copyright by Zhang Shiyong, 2014. shiyong.zhang.cn@outlook.com
 */

#ifndef _TINY_BUS_H_
#define _TINY_BUS_H_

#include <stdint.h>
#include <pthread.h>
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

typedef uint32_t message_id_t;
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


typedef struct _tiny_bus
{
	/*
	 * bus thread
	 */
	pthread_t       *thread;	
	
	/*
	 * contain tiny_msg_t object
	 */
	async_queue_t	*msg_queue;

	/*
	 * Identify current registered message ID count
	 */
    uint32_t        msg_id_count;
	
	/*
	 * contain slot_t objects, here I use array to associate message ID
	 * and slot list, a message ID is a index of array actually, and it
	 * can be assocaited with a slot list, slot subscribed message ID
	 * will be pushed into the slot list
	 */
	pthread_mutex_t	*mutex;	
	list_t          *slots[BUS_MESSAGE_MAX_ID];
} tiny_bus_t;

typedef enum
{	
	TINY_BUS_FAILED	= -1,
	TINY_BUS_SUCCEED
} tiny_bus_result_t;

tiny_bus_t *
tiny_bus_new(void); // allocate a bus object

void
tiny_bus_destroy(tiny_bus_t *bus);  // destroy a bus object

tiny_bus_result_t
tiny_bus_init_msg_ids(tiny_bus_t *bus, message_id_t* ids, size_t size);

void
tiny_bus_free_msg_id(tiny_bus_t *bus, message_id_t id);


#ifdef __cplusplus
}
#endif

#endif

// ~ End
