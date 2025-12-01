#include "ringbuf.h"

int ringbuf_read(ringbuf_t *rb, uint8_t *dst, int len)
{   
    /* Return if ringbuffer pointer is still null*/
    if (rb == NULL)
        return 0; 

    int n = 0;
    while (n < len && !ringbuf_empty(rb)) {
        dst[n++] = rb->buf[rb->tail];
        rb->tail = ringbuf_next(rb, rb->tail);
    }
    return n;
}

int ringbuf_write(ringbuf_t *rb, const uint8_t *src, int len)
{
    /* Return if ringbuffer pointer is still null*/
    if (rb == NULL)
        return 0; 

    int n = 0;
    while (n < len && !ringbuf_full(rb)) {
        rb->buf[rb->head] = src[n++];
        rb->head = ringbuf_next(rb, rb->head);
    }
    return n;
}
