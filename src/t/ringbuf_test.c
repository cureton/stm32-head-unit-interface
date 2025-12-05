#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../ringbuf.h"

#define RB_SIZE 8   /* must be power of two */

/* -----------------------------------------------------------------------
 * Test callback mechanism
 * ----------------------------------------------------------------------- */
typedef struct {
    int called;
    void *ctx_seen;
} cb_state_t;

static void test_callback_fn(void *ctx)
{
    cb_state_t *s = (cb_state_t *)ctx;
    s->called++;
    s->ctx_seen = ctx;
}

/* Reset callback state */
static void cb_reset(cb_state_t *s)
{
    s->called = 0;
    s->ctx_seen = NULL;
}

/* -----------------------------------------------------------------------
 * Main regression tests
 * ----------------------------------------------------------------------- */
int main(void)
{
    uint8_t storage[RB_SIZE];
    ringbuf_t rb;

    /* Test 1: Init / empty / full */
    ringbuf_init(&rb, storage, RB_SIZE);
    assert(ringbuf_empty(&rb));
    assert(!ringbuf_full(&rb));
    assert(ringbuf_count(&rb) == 0);
    assert(ringbuf_free(&rb) == RB_SIZE - 1);

    /* Test 2: Single put/get */
    ringbuf_put(&rb, 0x11);
    assert(!ringbuf_empty(&rb));
    assert(ringbuf_count(&rb) == 1);

    uint8_t b;
    assert(ringbuf_get(&rb, &b) == 1);
    assert(b == 0x11);
    assert(ringbuf_empty(&rb));

    /* Test 3: Bulk write/read simple */
    uint8_t in[4]  = {1,2,3,4};
    uint8_t out[4] = {0};

    int n = ringbuf_write(&rb, in, 4);
    assert(n == 4);
    assert(ringbuf_count(&rb) == 4);

    memset(out,0,sizeof(out));
    n = ringbuf_read(&rb, out, 4);
    assert(n == 4);
    assert(memcmp(in,out,4) == 0);
    assert(ringbuf_empty(&rb));

    /* Test 4: Wrap-around behaviour */
    ringbuf_flush(&rb);
    rb.head = RB_SIZE - 2;   /* e.g. head=6, tail=6 => empty */
    rb.tail = RB_SIZE - 2;

    uint8_t wrap_in[4] = {9,8,7,6};
    uint8_t wrap_out[4];

    n = ringbuf_write(&rb, wrap_in, 4);
    assert(n == 4);          /* must wrap correctly */
    memset(wrap_out,0,sizeof(wrap_out));
    n = ringbuf_read(&rb, wrap_out, 4);
    assert(memcmp(wrap_in, wrap_out, 4) == 0);

    /* Test 5: Fill buffer to full */
    ringbuf_flush(&rb);
    uint8_t fill[RB_SIZE];
    for (int i=0; i<RB_SIZE; i++) fill[i] = i;

    /* The ring buffer can store size-1 elements */
    n = ringbuf_write(&rb, fill, RB_SIZE);
    assert(n == RB_SIZE - 1);
    assert(ringbuf_full(&rb));

    /* Any further writes must return 0 written */
    n = ringbuf_write(&rb, fill, 1);
    assert(n == 0);
    assert(ringbuf_full(&rb));

    /* Test 6: Read all back */
    uint8_t recover[RB_SIZE];
    memset(recover,0,sizeof(recover));
    n = ringbuf_read(&rb, recover, RB_SIZE);
    assert(n == RB_SIZE - 1);
    for (int i=0; i < RB_SIZE-1; i++) {
        assert(recover[i] == i);
    }
    assert(ringbuf_empty(&rb));

    /* -------------------------------------------------------------------
     * Test 7: Write callback behaviour
     * ------------------------------------------------------------------- */
    cb_state_t cb;
    cb_reset(&cb);

    rb.write_notify_cb = test_callback_fn;
    rb.write_notify_cb_ctx = &cb;

    uint8_t cbtest[] = {0xAA, 0xBB};

    /* Callback should fire once for each write() call (your design) */
    n = ringbuf_write(&rb, cbtest, 2);
    assert(n == 2);
    assert(cb.called == 1);
    assert(cb.ctx_seen == &cb);

    /* Another write triggers callback again */
    n = ringbuf_write(&rb, cbtest, 2);
    assert(cb.called == 2);

    /* Test: callback fires even when no bytes written (buffer full) */
    ringbuf_flush(&rb);
    ringbuf_write(&rb, fill, RB_SIZE-1);      /* fill it */
    cb_reset(&cb);

    n = ringbuf_write(&rb, cbtest, 2);        /* will write 0 */
    assert(n == 0);
    assert(cb.called == 1);                   /* callback must still fire */
    assert(cb.ctx_seen == &cb);

    /* Test 8: ringbuf_read does NOT invoke callback */
    cb_reset(&cb);
    ringbuf_read(&rb, recover, RB_SIZE);
    assert(cb.called == 0);

    /* Test 9: NULL pointer safety */
    assert(ringbuf_read(NULL, recover, 5) == 0);
    assert(ringbuf_write(NULL, cbtest, 2) == 0);

    printf("ALL RINGBUF TESTS PASSED.\n");
    return 0;
}
