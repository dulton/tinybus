/*
 * tinybus.h
 * Copyright by Zhang Shiyong, 2014. shiyong.zhang.cn@outlook.com
 */

#ifndef _TINY_BUS_H_
#define _TINY_BUS_H_

#include <sys/types.h>
#include <stdint.h>

#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _tiny_bus_msg_priv
{
	uint32_t	ref_count;
	void *		data;
	size_t		size;
} tiny_msg_priv_t;

typedef struct _tiny_bus_msg
{
	uint32_t	seq;
	uint32_t	msg_id;
	
	tiny_msg_priv_t *msg_priv;
	
	// following is used when needing to transmit context for message processing
	void *		user_data;
	size_t		user_size;
} tiny_msg_t;



#ifdef __cplusplus
}
#endif

#endif

// ~ End