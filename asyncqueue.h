/*
 * asyncqueue.h
 * Copyright by Zhang Shiyong, 2014. shiyong.zhang.cn@outlook.com
 */

#ifndef _ASYNC_QUEUE_H_
#define _ASYNC_QUEUE_H_

#include <stdint.h>
#include <pthread.h>
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _async_queue async_queue_t;
struct _async_queue
{
	queue_t			*queue;
	pthread_mutex_t *mutex;
	pthread_cond_t 	*cond;
};


async_queue_t *async_queue_new(void);
void async_queue_push(async_queue_t *queue, void *data);
void *async_queue_pop(async_queue_t *queue);
void async_queue_foreach(async_queue_t *queue, func_visit_custom func, void *data);
void async_queue_destroy(async_queue_t *queue);

#ifdef __cplusplus
}
#endif

#endif /* _ASYNC_QUEUE_H_ */

// ~ end