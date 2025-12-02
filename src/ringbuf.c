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

    /* Callback conditions:
     *   fn_ptr not null and 
     *   - n > 0: data was written, OR
     *   - buffer is full: writer is blocked
     */

    if (rb->write_notify_cb && (n > 0 || ringbuf_full(rb))) {
        rb->write_notify_cb();
    }  
    return n;
}

void ringbuf_set_write_notify_fn(ringbuf_t *rb, ringbuf_notify_cb_t write_notify_cb_fn)
{
    rb->write_notify_cb = write_notify_cb_fn;

}


