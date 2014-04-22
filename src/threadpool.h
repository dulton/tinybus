#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include "atomic.h"
#include "asyncqueue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*exec_t) (void *, void *usr_data);

typedef enum _task_id
{
	task_stop = 0,
	task_run
} task_id_t;

typedef struct _task_data
{
	task_id_t	id;
	void *		param;
} task_data_t;

typedef struct _thread_pool
{
	exec_t 			exec;
	void 			*usr_data;
	async_queue_t	*queue;
	atomic_t		num_threads;
	atomic_t		max_threads;
} thread_pool_t;


thread_pool_t* thread_pool_new(exec_t func, void *data, int max_threads);
void thread_pool_push(thread_pool_t *tp, void *data);
void thread_pool_free(thread_pool_t *tp);


#ifdef __cplusplus
}
#endif

#endif
