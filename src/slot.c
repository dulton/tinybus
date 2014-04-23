#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include "common.h"
#include "trace.h"
#include "slot.h"

slot_t*
slot_new(char *name, 
    uint32_t msg_delta, exec_t func, void *usr_data, int max_threads)
{
	slot_t *slot;
	int length;
	
	slot = (slot_t *)calloc(1, sizeof(slot_t));
	if (slot == NULL)
	{
		show_err2(errno, "calloc");
		return NULL;
	}

    slot->msg_pool = thread_pool_new(func, usr_data, max_threads);
    if (slot->msg_pool == NULL)
    {
        show_err2(errno, "thread_pool_new");
        free(slot);
        
        return NULL;
    }

	length = strlen(name);
	length = (length < sizeof(slot->slot_name)) ? length : (sizeof(slot->slot_name) - 1);
	strncpy(slot->slot_name, name, length);

    slot->msg_delta = msg_delta;
	return slot;
}

void
slot_free(slot_t *slot)
{
	assert(slot);
	assert(slot->msg_pool);
    
    thread_pool_free(slot->msg_pool);
		
	free(slot);
}

static inline uint32_t
slot_entry_index(slot_t *slot, message_id_t id)
{
    assert(slot);
    assert(id >= slot->msg_delta);
    
    return (id - slot->msg_delta);
}

tiny_bus_result_t
slot_subscribe_message(
    tiny_bus_t *bus, slot_t *slot, message_id_t id, msg_func_t handler)
{
	// this message id's slot lish must be allocated;
	if (bus->slots[id] == NULL)
    {
        show_err("Failed to subscribe message. You must allocat message id firstly.\r\n");
        return TINY_BUS_FAILED;
    }
	
	pthread_mutex_lock(bus->mutex);
	list_insert_tail(bus->slots[id], slot);
	pthread_mutex_unlock(bus->mutex);

    slot->msg_funcs[slot_entry_index(slot, id)] = handler;
	
	return TINY_BUS_SUCCEED;
}

void
slot_unsubscribe_message(tiny_bus_t *bus, slot_t *slot, message_id_t id)
{
    assert(bus->slots[id]);

    pthread_mutex_lock(bus->mutex);
    list_remove(bus->slots[id], slot);
    pthread_mutex_unlock(bus->mutex);

    slot->msg_funcs[slot_entry_index(slot, id)] = NULL;

    return;
}


/*
 * bus send message into slot's msg queue
 */
tiny_bus_result_t
slot_write(slot_t *slot, tiny_msg_t *msg)
{
	assert(slot);
	assert(slot->msg_pool);
	assert(msg);

    thread_pool_push(slot->msg_pool, msg);
	
	return TINY_BUS_SUCCEED;
}

void
slot_publish(tiny_bus_t *bus, tiny_msg_t *msg)
{
    assert(bus);

    async_queue_push(bus->msg_queue, msg);
}


