#ifndef MESHLINK_QUEUE_H
#define MESHLINK_QUEUE_H

/*
    queue.h -- Thread-safe queue
    Copyright (C) 2014, 2017 Guus Sliepen <guus@meshlink.io>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>

typedef struct meshlink_queue {
	struct meshlink_queue_item *head;
	struct meshlink_queue_item *tail;
	pthread_mutex_t mutex;
} meshlink_queue_t;

typedef struct meshlink_queue_item {
	void *data;
	struct meshlink_queue_item *next;
} meshlink_queue_item_t;

static inline void meshlink_queue_init(meshlink_queue_t *queue) {
	queue->head = NULL;
	queue->tail = NULL;
	pthread_mutex_init(&queue->mutex, NULL);
}

static inline void meshlink_queue_exit(meshlink_queue_t *queue) {
	pthread_mutex_destroy(&queue->mutex);
}

static inline __attribute__((__warn_unused_result__)) bool meshlink_queue_push(meshlink_queue_t *queue, void *data) {
	meshlink_queue_item_t *item = malloc(sizeof(*item));

	if(!item) {
		return false;
	}

	item->data = data;
	item->next = NULL;

	if(pthread_mutex_lock(&queue->mutex) != 0) {
		abort();
	}

	if(!queue->tail) {
		queue->head = queue->tail = item;
	} else {
		queue->tail = queue->tail->next = item;
	}

	pthread_mutex_unlock(&queue->mutex);
	return true;
}

static inline __attribute__((__warn_unused_result__)) void *meshlink_queue_pop(meshlink_queue_t *queue) {
	meshlink_queue_item_t *item;

	if(pthread_mutex_lock(&queue->mutex) != 0) {
		abort();
	}

	if((item = queue->head)) {
		queue->head = item->next;

		if(!queue->head) {
			queue->tail = NULL;
		}
	}

	pthread_mutex_unlock(&queue->mutex);

	void *data = item ? item->data : NULL;
	free(item);
	return data;
}

static inline __attribute__((__warn_unused_result__)) void *meshlink_queue_pop_cond(meshlink_queue_t *queue, pthread_cond_t *cond) {
	meshlink_queue_item_t *item;

	if(pthread_mutex_lock(&queue->mutex) != 0) {
		abort();
	}

	while(!queue->head) {
		pthread_cond_wait(cond, &queue->mutex);
	}

	item = queue->head;
	queue->head = item->next;

	if(!queue->head) {
		queue->tail = NULL;
	}

	pthread_mutex_unlock(&queue->mutex);

	void *data = item->data;
	free(item);
	return data;
}

#endif
