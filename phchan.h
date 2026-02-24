#ifndef PHCHAN_H
#define PHCHAN_H

#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct phchan_t phchan_t;

int phchan_init(phchan_t *chan, size_t capacity);
int phchan_close(phchan_t *chan);
int phchan_destroy(phchan_t *chan);

int phchan_send(phchan_t *chan, void *data);
void *phchan_recv(phchan_t *chan);

int phchan_try_send(phchan_t *chan, void *data);
void *phchan_try_recv(phchan_t *chan);

#ifdef PHCHAH_IMPLEMENTATION
struct phchan_t {
	void **buf;
	size_t size;
	size_t capacity;
	size_t head;
	size_t tail;
	bool closed;
	pthread_mutex_t mutex;
	pthread_cond_t read_lock;
	pthread_cond_t write_lock;
};

#endif // PHCHAH_IMPLEMENTATION

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PHCHAN_H
