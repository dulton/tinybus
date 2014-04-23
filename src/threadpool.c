#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "common.h"
#include "threadpool.h"


static void *
thread_pool_thread_proxy(void *arg)
{
	thread_pool_t *tp = (thread_pool_t*)arg;
	task_data_t *data;
	int num;
	
	while ( 1 )
	{
		data = (task_data_t *)async_queue_pop(tp->queue);
		if (data->id == task_run)
		{
			(*tp->exec)(data->param, tp->usr_data);
			free(data);
		}
		else if (data->id == task_stop)
		{
			free(data);
			break;
		}
	}

	num = atomic_dec(&tp->num_threads);
	if (num == 0)
	{
		// async queue length should be zero now
		async_queue_destroy(tp->queue);
		free(tp);
	}

#ifdef _THREAD_POOL_DEBUG_
	fprintf(stderr, 
		"Thread(%lu) has exited now, thread num is %d", pthread_self(), num);
#endif
		
	return NULL;
}

static void
thread_pool_start_threads(thread_pool_t *tp)
{
	pthread_t 		thread;
	pthread_attr_t	attr;	
	int				result;
	
	posix_check_cmd(pthread_attr_init(&attr));
	posix_check_cmd(
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED));

	while (atomic_get(&tp->max_threads) == -1 
		|| atomic_get(&tp->num_threads) < atomic_get(&tp->max_threads))
	{
		result = pthread_create(
			&thread, &attr, thread_pool_thread_proxy, tp);
		show_err2(result, "pthread_create");
		
		atomic_inc(&tp->num_threads);

		if (atomic_get(&tp->max_threads) == -1)
			break;
	}

	posix_check_cmd(pthread_attr_destroy(&attr));
}

thread_pool_t*
thread_pool_new(exec_t func, void *data, int max_threads)
{
	thread_pool_t *tp;

	tp = (thread_pool_t *)calloc(1, sizeof(thread_pool_t));
	if (tp != NULL)
	{
		tp->exec = func;
		tp->usr_data = data;
		tp->queue = async_queue_new();
		tp->num_threads = 0;
		tp->max_threads = max_threads ? max_threads : 1;
		
		thread_pool_start_threads(tp);		
	}

	return tp;
}

void
thread_pool_push(thread_pool_t *tp, void *data)
{
	task_data_t *td;
	
	assert(tp != NULL && data != NULL);

	td       = (task_data_t *)calloc(1, sizeof(task_data_t));
	td->id    = task_run;
	td->param = data;
	
	async_queue_push(tp->queue, (void *)td);	
}

void
thread_pool_free(thread_pool_t *tp)
{
	task_data_t *task;
	int i, length;

	length = atomic_get(&tp->num_threads);

	for (i = 0; i < length; i++)
	{
		task = (task_data_t *)calloc(1, sizeof(task_data_t));	
		task->id = task_stop;

		// push "stop" to notify thread exit
		async_queue_push(tp->queue, (void *)task);
	}	
}

