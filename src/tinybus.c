/*
 * tinybus.c
 * Copyright by Zhang Shiyong, 2014. shiyong.zhang.cn@outlook.com
 */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tinybus.h"
#include "slot.h"
#include "trace.h"

static void
bus_post_message(void *node, void *data)
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
		list_foreach(self->slots[id], bus_post_message, msg);
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
		    TRACE_WARNING("file %s: line %d (%s): bus exit\r\n", 
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

tiny_bus_result_t
tiny_bus_init_msg_ids(tiny_bus_t *bus, message_id_t* ids, size_t size)
{
    size_t index, max_size;

    max_size = sizeof(bus->slots);
    if (size >= max_size)
        return TINY_BUS_FAILED;

    pthread_mutex_lock(bus->mutex);
    for (index = 0; index < size; index++)
    {
        if (ids[index] < max_size)
            bus->slots[ids[index]] = (list_t *)calloc(1, sizeof(list_t));
        else
            assert(0);
        
        assert(bus->slots[ids[index]]);
    }
    pthread_mutex_unlock(bus->mutex);
    
    return TINY_BUS_SUCCEED;
}

void
tiny_bus_free_msg_id(tiny_bus_t *bus, message_id_t id)
{
	pthread_mutex_lock(bus->mutex);

    list_free(bus->slots[id]);
    free(bus->slots[id]);
    bus->slots[id] = NULL;
    
	pthread_mutex_unlock(bus->mutex);

    // Attension: here I don't run "msg_id_count--"
    // bus->msg_id_count-- ??
}


