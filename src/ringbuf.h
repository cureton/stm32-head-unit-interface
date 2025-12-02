#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdint.h>
#include <stddef.h>

/* 
 * Power-of-two ring buffer for embedded systems.
 * User supplies storage and size.
 * Size MUST be a power of two: e.g. 32, 64, 128, 256, 512...
 */



typedef void (*ringbuf_notify_cb_t)(void);

typedef struct {
    uint8_t  *buf;
    uint16_t  size;
    volatile uint16_t head;
    volatile uint16_t tail;
    ringbuf_notify_cb_t write_notify_cb;
} ringbuf_t;




/* Initialise ring buffer using user-provided storage */
static inline void ringbuf_init(ringbuf_t *rb, uint8_t *storage, uint16_t size)
{
    rb->buf  = storage;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
    rb->write_notify_cb = NULL;
}

/* Internal helper */
static inline uint16_t ringbuf_mask(const ringbuf_t *rb)
{
    return rb->size - 1;
}

static inline uint16_t ringbuf_next(const ringbuf_t *rb, uint16_t index)
{
    return (index + 1) & ringbuf_mask(rb);
}

/* Basic status checks */
static inline int ringbuf_empty(const ringbuf_t *rb)
{
    return rb->head == rb->tail;
}

static inline int ringbuf_full(const ringbuf_t *rb)
{
    return ringbuf_next(rb, rb->head) == rb->tail;
}

static inline uint16_t ringbuf_count(const ringbuf_t *rb)
{
    return (rb->head - rb->tail) & ringbuf_mask(rb);
}

static inline uint16_t ringbuf_free(const ringbuf_t *rb)
{
    return rb->size - 1 - ringbuf_count(rb);
}

static inline void ringbuf_flush(ringbuf_t *rb)
{
    rb->tail = rb->head;
}

/* Single-byte operations (ISR safe, fully inline) */
static inline void ringbuf_put(ringbuf_t *rb, uint8_t b)
{
    uint16_t next = ringbuf_next(rb, rb->head);
    if (next == rb->tail) {
        return; /* full, drop */
    }
    rb->buf[rb->head] = b;
    rb->head = next;
}

static inline int ringbuf_get(ringbuf_t *rb, uint8_t *out)
{
    if (ringbuf_empty(rb)) {
        return 0;
    }
    *out = rb->buf[rb->tail];
    rb->tail = ringbuf_next(rb, rb->tail);
    return 1;
}

/* Bulk multi-byte ops (optional, non-inline) */
int ringbuf_read(ringbuf_t *rb, uint8_t *dst, int len);
int ringbuf_write(ringbuf_t *rb, const uint8_t *src, int len);
void ringbuf_set_write_notify_fn(ringbuf_t *rb, ringbuf_notify_cb_t write_notify_cb_fn);
#endif /* RINGBUF_H */

