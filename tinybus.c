/*
 * tinybus.c
 * Copyright by Zhang Shiyong, 2014. shiyong.zhang.cn@outlook.com
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tinybus.h"

#define tiny_bus_print_err(err, name) \
do {\
	int error = (err); 							\
	if (error)	 		 		 			\
		fprintf(stderr, "file %s: line %d (%s): error '%d' during '%s'",	\
           __FILE__, __LINE__, __FUNCTION__, error, name);					\
} while (0)

static void
post_message(void *node, void *data)
{
	slot_t *slot;
	tiny_msg_t *msg;
	
	slot = (slot_t *)node;
	msg = (tiny_msg_t *)data;
	
	slot_write(slot, msg);
}

static void
tiny_bus_deliver(tiny_bus_t *self, tiny_msg_t *msg)
{
	int id;
	assert(self->mutex);
	
	id = msg->msg_id;
	if (self->slots[id] != NULL)
	{
		pthread_mutex_lock(self->mutex);
		list_foreach(self->slots[id], post_message, msg);
		pthread_mutex_unlock(self->mutex);		
	}
}

static void *
tiny_bus_thread_worker(void *context)
{
	tiny_msg_t *msg;
	tiny_bus_t *self;

	self = (tiny_bus_t *)context;	
	for (;;)
	{
		msg = (tiny_msg_t *)async_queue_pop(self->msg_queue);
		if (msg->msg_id == TINY_BUS_MSG_EXIT)
		{
			fprintf(stdout, "file %s: line %d (%s): bus exit",
				__FILE__, __LINE__, __FUNCTION__);				
			break;
		}

		atomic_dec(&msg->msg_priv->ref_count);		
		tiny_bus_deliver(self, msg);
	}
	
	return NULL;
}

static void
tiny_bus_free0(tiny_bus_t *bus)
{
	int index;
	assert(bus);
	
	if (bus->msg_queue)
		async_queue_destroy(bus->msg_queue);
	
	if (bus->mutex)
	{
		pthread_mutex_destroy(bus->mutex);
		free(bus->mutex);		
	}
		
	for (index = 0; index < BUS_MESSAGE_MAX_ID; index++)
	{
		if (bus->slots[index] != NULL)
			list_free(bus->slots[index]);
	}
		
	if (bus->thread)
		free(bus->thread);		
		
	free(bus);
}

tiny_bus_t*
tiny_bus_new(void)
{
	tiny_bus_t *bus;
	int result;
	
	bus = (tiny_bus_t *)calloc(1, sizeof(tiny_bus_t));
	if (bus == NULL)
	{
		tiny_bus_print_err(errno, "calloc");			
		return NULL;	
	}
	
	bus->msg_queue = async_queue_new();
	if (bus->msg_queue == NULL)
	{
		tiny_bus_print_err(errno, "async_queue_new");	
		tiny_bus_free0(bus);
		return NULL;
	}
		
	bus->mutex = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
	if (bus->mutex == NULL)
	{
		tiny_bus_print_err(errno, "calloc");
		tiny_bus_free0(bus);
		return NULL;
	}
	
	result = pthread_mutex_init(bus->mutex, NULL);
	if (result != 0)
	{
		tiny_bus_print_err(errno, "pthread_mutex_init");
		tiny_bus_free0(bus);
		return NULL;
	}
	
	memset(bus->slots, 0, sizeof(bus->slots));
	
	bus->thread = (pthread_t *)calloc(1, sizeof(pthread_t));
	if (bus->thread == NULL)
	{
		tiny_bus_print_err(errno, "calloc");
		tiny_bus_free0(bus);
		return NULL;		
	}
	
	result = pthread_create(bus->thread, NULL, tiny_bus_thread_worker, bus);
	if (result)
	{
		tiny_bus_print_err(result, "pthread_create");
		tiny_bus_free0(bus);
		return NULL;		
	}
	
	return bus;
}

void
tiny_bus_destroy(tiny_bus_t *bus)
{
	// .. to do
}

message_id_t
tiny_bus_alloc_msg_id(tiny_bus_t *bus)
{
	message_id_t id;
	list_t   	 *list;
	
	id = bus->msg_id_count;
	bus->msg_id_count++;			
	
	// initiate slot list for message ID
	list = (list_t *)calloc(1, sizeof(list_t));
	
	pthread_mutex_lock(bus->mutex);
	bus->slots[id] = list;
	pthread_mutex_unlock(bus->mutex);
	
	return id;
}

tiny_bus_result_t
slot_subscribe_message(tiny_bus_t *bus, slot_t *slot, message_id_t id)
{
	// this message id's slot lish must be allocated;
	assert(bus->slots[id]);
	
	pthread_mutex_lock(bus->mutex);
	list_insert_tail(bus->slots[id], slot);
	pthread_mutex_unlock(bus->mutex);
	
	return TINY_BUS_SUCCEED;
}

slot_t*
slot_new(char *name, uint16_t port)
{
	slot_t *slot;
	int length, result;
    struct sockaddr_in addr;
	socklen_t addrlen;
    char sock_resuse = 1; 
	
	slot = (slot_t *)calloc(1, sizeof(slot_t));
	if (slot == NULL)
	{
		tiny_bus_print_err(errno, "calloc");
		return NULL;
	}

	length = strlen(name);
	length = (length >= sizeof(slot->slot_name)) ? (sizeof(slot->slot_name) - 1) : length;		
	strncpy(slot->slot_name, name, length);
	
	slot->socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (slot->socket < 0)
	{
		tiny_bus_print_err(errno, "socket");
		slot_free(slot);		
		return NULL;
	}

    if (setsockopt(slot->socket, SOL_SOCKET, SO_REUSEADDR, &sock_resuse, sizeof(char)) < 0)
    {
		tiny_bus_print_err(errno, "setsockopt");
		slot_free(slot);		
		return NULL;
    }
	
	addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_aton("127.0.0.1", &addr.sin_addr);		
	if (bind(slot->socket, (struct sockaddr *)&addr, addrlen) < 0)
	{
		tiny_bus_print_err(errno, "bind");
		slot_free(slot);		
		return NULL;
	}
	
	slot->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	if (slot->mutex == NULL)
	{
		tiny_bus_print_err(errno, "malloc");
		slot_free(slot);
		return NULL;
	}
	
	result = pthread_mutex_init(slot->mutex, NULL);
	if (result)
	{
		tiny_bus_print_err(result, "pthread_mutex_init");
		slot_free(slot);
		return NULL;
	}

	slot->msg_queue = queue_new();
	if (slot->msg_queue == NULL)
	{
		tiny_bus_print_err(errno, "queue_new");
		slot_free(slot);
		return NULL;
	}
	
	return slot;
}

void
slot_free(slot_t *slot)
{
	assert(slot);
	
	if (slot->mutex)
	{
		pthread_mutex_destroy(slot->mutex);
		free(slot->mutex);	
	}
				
	if (slot->msg_queue)
		queue_free(slot->msg_queue);
		
	if (slot->socket)
	{
	#ifdef WIN32
		closesocket(slot->socket);
	#else		
		close(slot->socket);
	#endif		
	}
		
	free(slot);
}

tiny_bus_result_t
slot_write(slot_t *slot, tiny_msg_t *msg)
{
	assert(slot);
	assert(slot->mutex);
	assert(slot->msg_queue);
	assert(msg);
	
	pthread_mutex_lock(slot->mutex);
	queue_push_tail(slot->msg_queue, msg);
	pthread_mutex_unlock(slot->mutex);
	
	return TINY_BUS_SUCCEED;
}

tiny_msg_t *
slot_read(slot_t *slot)
{
	tiny_msg_t *msg;
	
	assert(slot);
	assert(slot->mutex);
	assert(slot->msg_queue);
	
	pthread_mutex_lock(slot->mutex);
	msg = queue_pop_head(slot->msg_queue);
	pthread_mutex_unlock(slot->mutex);
	
	return msg;
}