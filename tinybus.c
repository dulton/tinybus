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

static int compare_message_id(void *node, void *data)
{
	message_id_t *id;
	tiny_msg_t *msg;

	return (msg->msg_id == *id) ? 0 : 1;
}

static void
post_message(void *node, void *data)
{
	slot_t *slot;
	tiny_msg_t *msg;
	
	slot = (slot_t *)node;
	msg = (tiny_msg_t *)data;
	
	// now check whether msg->msg_id is in slot->subscription
	if (list_find_custom(slot->subscription, msg, compare_message_id) != NULL)
	{
		async_queue_push(slot->msg_queue, msg);		
		atomic_inc(&msg->msg_priv->ref_count);
	}
}

static int
tiny_bus_deliver(tiny_bus_t *self, tiny_msg_t *msg)
{
	assert(self->mutex);
	
	pthread_mutex_lock(self->mutex);
	list_foreach(self->slot_list, post_message, msg);
	pthread_mutex_unlock(self->mutex);	
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
		
		tiny_bus_deliver(self, msg);
		atomic_dec(&msg->msg_priv->ref_count);
	}
	
	return NULL;
}

static void
tiny_bus_free0(tiny_bus_t *bus)
{
	assert(bus);
	
	if (bus->msg_queue)
		async_queue_destroy(bus->msg_queue);
	
	if (bus->mutex)
		free(bus->mutex);
		
	if (bus->slot_list)
		list_free(bus->slot_list);
		
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
	
	bus->slot_list = (list_t *)calloc(1, sizeof(list_t));
	if (bus->slot_list == NULL)
	{
		tiny_bus_print_err(errno, "calloc");
		tiny_bus_free0(bus);
		return NULL;		
	}
	
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

tiny_bus_result_t
tiny_bus_add_slot(tiny_bus_t *bus, slot_t *slot)
{
	int result;
	
	assert(bus);
	assert(bus->mutex);
	
	result = pthread_mutex_lock(bus->mutex);
	if (result)
		return TINY_BUS_FAILED;
		
	list_insert_tail(bus->slot_list, slot);
	
	result = pthread_mutex_unlock(bus->mutex);
	if (result)
		return TINY_BUS_FAILED;
		
	return TINY_BUS_SUCCEED;
}

tiny_bus_result_t
tiny_bus_del_slot(tiny_bus_t *bus, slot_t *slot)
{
	int result;
	
	assert(bus);
	assert(bus->mutex);
	
	result = pthread_mutex_lock(bus->mutex);
	if (result)
		return TINY_BUS_FAILED;
		
	list_remove(bus->slot_list, slot);
	
	result = pthread_mutex_unlock(bus->mutex);
	if (result)
		return TINY_BUS_FAILED;
		
	return TINY_BUS_SUCCEED;	
}

slot_t*
slot_new(char *name)
{
	slot_t *slot;
	int length, result;
	
	slot = (slot_t *)calloc(1, sizeof(slot_t));
	if (slot == NULL)
	{
		tiny_bus_print_err(errno, "calloc");
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
	
	slot->subscription = (list_t *)calloc(1, sizeof(list_t));
	if (slot->subscription == NULL)
	{
		tiny_bus_print_err(errno, "list_alloc");
		slot_free(slot);
		return NULL;		
	}
		
	length = strlen(name);
	length = (length >= sizeof(slot->slot_name)) ? (sizeof(slot->slot_name) - 1) : length;		
	strncpy(slot->slot_name, name, length);
}

void
slot_free(slot_t *slot)
{
	assert(slot);
	
	if (slot->mutex)
		free(slot->mutex);
		
	if (slot->subscription)
		list_free(slot->subscription);
		
	if (slot->msg_queue)
		async_queue_destroy(slot->msg_queue);
		
	free(slot);
}
