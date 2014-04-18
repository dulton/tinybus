/*
 * asyncqueue.c
 * Copyright by Zhang Shiyong, 2014. shiyong.zhang.cn@outlook.com
 */
#include <assert.h> 
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "asyncqueue.h"

async_queue_t *
async_queue_new( void )
{
	async_queue_t *async_queue;
	
	async_queue = (async_queue_t *)calloc(1, sizeof(async_queue_t));
	if (async_queue == NULL)
		return NULL;
	
	async_queue->queue = (queue_t *)queue_new();
	if (async_queue->queue == NULL)
	{
		free(async_queue);
		return NULL;
	}
	
	async_queue->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	if (async_queue->mutex == NULL)
	{
		async_queue_destroy(async_queue);
		return NULL;
	}
	
	if (pthread_mutex_init(async_queue->mutex, NULL) != 0)
	{
		async_queue_destroy(async_queue);
		return NULL;
	}
	
	async_queue->cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
	if (async_queue->cond == NULL)
	{
		async_queue_destroy(async_queue);
		return NULL;
	}
	
	if (pthread_cond_init(async_queue->cond, NULL) != 0)
	{
		async_queue_destroy(async_queue);
		return NULL;		
	}

	return async_queue;
}

void
async_queue_destroy(async_queue_t *queue)
{
	assert(queue);
	
	if (queue->queue)
		queue_free(queue->queue);
		
	if (queue->cond)
	{
		pthread_cond_destroy(queue->cond);
		free(queue->cond);
	}
	
	if (queue->mutex)
	{
		pthread_mutex_destroy(queue->mutex);
		free(queue->mutex);			
	}
	
	free(queue);
}

static void
async_queue_push_unlocked(async_queue_t *queue, void *data)
{
	assert(queue != NULL && data != NULL);

	queue_push_tail(queue->queue, data);
	pthread_cond_signal(queue->cond);
}

void
async_queue_push(async_queue_t *queue, void *data)
{
	assert(queue != NULL);
	assert(queue->mutex != NULL);

	pthread_mutex_lock(queue->mutex);
	async_queue_push_unlocked(queue, data);
	pthread_mutex_unlock(queue->mutex);
}

static void *
async_queue_pop_unlocked(async_queue_t *queue)
{
	int result;
	void *data;

	for (;;)
	{
		data = queue_pop_head(queue->queue);
		if (data)
			return data;

		assert(queue->cond != NULL);
		result = pthread_cond_wait(queue->cond, queue->mutex);		
		if (result != 0)
			fprintf(stderr, "file %s: line %d (%s): error '%d' during '%s'",
				__FILE__, __LINE__, __FUNCTION__, result, "pthread_cond_wait");
	}

	return NULL;
}

void *
async_queue_pop(async_queue_t *queue)
{
	void *retval;
	assert(queue != NULL);

	pthread_mutex_lock(queue->mutex);
	retval = async_queue_pop_unlocked(queue);
	pthread_mutex_unlock(queue->mutex);

	return retval;
}

void
async_queue_foreach(async_queue_t *queue, func_visit_custom func, void *data)
{
	assert(queue != NULL);
	
	pthread_mutex_lock(queue->mutex);
	queue_foreach(queue->queue, func, data);
	pthread_mutex_unlock(queue->mutex);
}