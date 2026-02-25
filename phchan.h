/*
 * phchan.h v1.0
 *
 * Made by Philainel
 *
 * Simple thread-safe buffered channels
 *
 */

#ifndef PHCHAN_H
#define PHCHAN_H

#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * This fields are meant to be private and unstable between releases.
 * DO NOT modify them manually until you know what you are doing.
 */
typedef struct {
	void **buf;
	size_t size;
	size_t capacity;
	size_t head;
	size_t tail;
	bool closed;
	pthread_mutex_t mutex;
	pthread_cond_t read_lock;
	pthread_cond_t write_lock;
} phchan_t;

int phchan_init(phchan_t *chan, size_t capacity);	// Initializes buffered chan with initial capacity
void phchan_close(phchan_t *chan);					// Closes chan
void phchan_destroy(phchan_t *chan);				// Destroys the chan, must be called after phchan_close() and stopping all reader threads
bool phchan_is_closed(phchan_t *chan);				// Returns true if given chan is closed

int phchan_send(phchan_t *chan, void *data);		// Sends data to given chan, blocks when internal buffer is full, returns -1 if chan is closed, otherwise 0
void *phchan_recv(phchan_t *chan);					// Reads and returns data from given chan, if no data available, blocks if chan is opened and returns NULL if chan is closed

int phchan_try_send(phchan_t *chan, void *data);	// Sends data to given chan, returns -1 if chan is closed or internal buffer is full
void *phchan_try_recv(phchan_t *chan);				// Reads and returns data from given chan, returns NULL if chan is closed or internal buffer is empty

// TODO: timed send and recv

#ifdef PHCHAN_IMPLEMENTATION

#include <stdlib.h>

int phchan_init(phchan_t *chan, size_t capacity) {
	chan->capacity = capacity;
	chan->size = 0;
	chan->head = 0;
	chan->tail = 0;
	chan->closed = false;
	chan->buf = malloc(sizeof(void *) * capacity);
	pthread_mutex_init(&chan->mutex, NULL);
	pthread_cond_init(&chan->read_lock, NULL);
	pthread_cond_init(&chan->write_lock, NULL);
	return 0;
}

void phchan_close(phchan_t *chan) {
	pthread_mutex_lock(&chan->mutex);
	chan->closed = true;
	pthread_cond_broadcast(&chan->write_lock);
	pthread_cond_broadcast(&chan->read_lock);
	pthread_mutex_unlock(&chan->mutex);
}

void phchan_destroy(phchan_t *chan) {
	if (!chan) return;
	free(chan->buf);
	pthread_mutex_destroy(&chan->mutex);
	pthread_cond_destroy(&chan->read_lock);
	pthread_cond_destroy(&chan->write_lock);
}

inline bool phchan_is_closed(phchan_t *chan) {
	return chan->closed;
}

int phchan_send(phchan_t *chan, void *data) {
	pthread_mutex_lock(&chan->mutex);
	while (chan->size == chan->capacity && !chan->closed) {
		pthread_cond_wait(&chan->write_lock, &chan->mutex);
	}
	if (chan->closed) {
		pthread_mutex_unlock(&chan->mutex);
		return -1;
	}
	chan->buf[chan->tail] = data;
	chan->tail = (chan->tail + 1) % chan->capacity;
	++chan->size;
	pthread_cond_signal(&chan->read_lock);
	pthread_mutex_unlock(&chan->mutex);
	return 0;
}

void *phchan_recv(phchan_t *chan) {
	pthread_mutex_lock(&chan->mutex);
	while (chan->size == 0 && !chan->closed) {
		pthread_cond_wait(&chan->read_lock, &chan->mutex);
	}
	if (chan->size == 0 && chan->closed) {
		pthread_mutex_unlock(&chan->mutex);
		return NULL;
	}
	void *result = chan->buf[chan->head];
	chan->head = (chan->head + 1) % chan->capacity;
	--chan->size;
	pthread_cond_signal(&chan->write_lock);
	pthread_mutex_unlock(&chan->mutex);
	return result;
}

int phchan_try_send(phchan_t *chan, void *data) {
	pthread_mutex_lock(&chan->mutex);
	if (chan->closed || chan->size == chan->capacity) {
		pthread_mutex_unlock(&chan->mutex);
		return -1;
	}
	chan->buf[chan->tail] = data;
	chan->tail = (chan->tail + 1) % chan->capacity;
	++chan->size;
	pthread_cond_signal(&chan->read_lock);
	pthread_mutex_unlock(&chan->mutex);
	return 0;

}

void *phchan_try_recv(phchan_t *chan) {
	void *result = NULL;
	pthread_mutex_lock(&chan->mutex);
	if (chan->size > 0) {
		result = chan->buf[chan->head];
		chan->head = (chan->head + 1) % chan->capacity;
		--chan->size;
		pthread_cond_signal(&chan->write_lock);
	}
	pthread_mutex_unlock(&chan->mutex);
	return result;
}

#endif // PHCHAN_IMPLEMENTATION

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PHCHAN_H
