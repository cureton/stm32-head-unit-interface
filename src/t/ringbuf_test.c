#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../ringbuf.h"

#define RB_SIZE 8   /* must be power-of-two */

/*********************************************************************
 *  Callback tracking structure
 *********************************************************************/
typedef struct {
    int called;          /* how many times the callback was invoked */
    void *ctx_seen;      /* which context pointer was passed */
} cb_state_t;

static void test_callback_fn(void *ctx)
{
    cb_state_t *s = (cb_state_t *)ctx;
    s->called++;
    s->ctx_seen = ctx;   /* store pointer seen */
}

static void cb_reset(cb_state_t *s)
{
    s->called = 0;
    s->ctx_seen = NULL;
}

/*********************************************************************
 *  Regression Tests
 *********************************************************************/
int main(void)
{
    uint8_t storage[RB_SIZE];
    ringbuf_t rb;

    /*************************************************************
     * 1. Basic ringbuf initialisation
     *************************************************************/
    ringbuf_init(&rb, storage, RB_SIZE);
    assert(ringbuf_empty(&rb));
    assert(!ringbuf_full(&rb));
    assert(ringbuf_count(&rb) == 0);
    assert(ringbuf_free(&rb) == RB_SIZE - 1);

    /*************************************************************
     * 2. Basic put/get tests
     *************************************************************/
    ringbuf_put(&rb, 0x12);
    assert(!ringbuf_empty(&rb));

    uint8_t b;
    assert(ringbuf_get(&rb, &b) == 1);
    assert(b == 0x12);
    assert(ringbuf_empty(&rb));

    /*************************************************************
     * 3. Burst write / burst read
     *************************************************************/
    uint8_t in4[4]  = { 1,2,3,4 };
    uint8_t out4[4] = { 0 };

    int n = ringbuf_write(&rb, in4, 4);
    assert(n == 4);
    assert(ringbuf_count(&rb) == 4);

    memset(out4, 0, sizeof(out4));
    n = ringbuf_read(&rb, out4, 4);
    assert(n == 4);
    assert(memcmp(in4, out4, 4) == 0);

    /*************************************************************
     * 4. Wrap-around behaviour
     *************************************************************/
    ringbuf_flush(&rb);
    rb.head = RB_SIZE - 2;   /* e.g. head = 6, tail = 6 = empty */
    rb.tail = RB_SIZE - 2;

    uint8_t wrap_in[4]  = { 9,8,7,6 };
    uint8_t wrap_out[4] = { 0 };

    n = ringbuf_write(&rb, wrap_in, 4);
    assert(n == 4);

    n = ringbuf_read(&rb, wrap_out, 4);
    assert(memcmp(wrap_in, wrap_out, 4) == 0);

    /*************************************************************
     * 5. Fill buffer & test full condition
     *************************************************************/
    ringbuf_flush(&rb);

    uint8_t fill[RB_SIZE];
    for (int i = 0; i < RB_SIZE; i++) fill[i] = i;

    /* size-1 bytes can be written */
    n = ringbuf_write(&rb, fill, RB_SIZE);
    assert(n == RB_SIZE - 1);
    assert(ringbuf_full(&rb));

    /* further writes write 0 bytes */
    n = ringbuf_write(&rb, fill, 1);
    assert(n == 0);

    uint8_t recover[RB_SIZE] = {0};
    n = ringbuf_read(&rb, recover, RB_SIZE);
    assert(n == RB_SIZE - 1);
    for (int i = 0; i < RB_SIZE - 1; i++)
        assert(recover[i] == i);

    assert(ringbuf_empty(&rb));

    /*************************************************************
     * 6. Updated CALLBACK tests with context pointer
     *************************************************************/
    cb_state_t cb;
    cb_reset(&cb);

    ringbuf_set_write_notify_fn(&rb, test_callback_fn, &cb);

    uint8_t cbdata[2] = {0xAA, 0xBB};

    /* Every write() should call the callback ONCE */
    n = ringbuf_write(&rb, cbdata, 2);
    assert(n == 2);
    assert(cb.called == 1);
    assert(cb.ctx_seen == &cb);

    /* second write() → second callback */
    n = ringbuf_write(&rb, cbdata, 2);
    assert(cb.called == 2);

    /*************************************************************
     * 7. Callback is invoked even when buffer is full (n==0)
     *************************************************************/
    ringbuf_flush(&rb);
    /* fill completely */
    ringbuf_write(&rb, fill, RB_SIZE - 1);

    cb_reset(&cb);
    /* now buffer is full; write = 0 but callback must run */
    n = ringbuf_write(&rb, cbdata, 2);
    assert(n == 0);
    assert(cb.called == 1);
    assert(cb.ctx_seen == &cb);

    /*************************************************************
     * 8. ringbuf_read NEVER triggers callback
     *************************************************************/
    cb_reset(&cb);
    ringbuf_read(&rb, recover, RB_SIZE);
    assert(cb.called == 0);

    /*************************************************************
     * 9. NULL pointer safety
     *************************************************************/
    n = ringbuf_read(NULL, recover, 4);
    assert(n == 0);

    n = ringbuf_write(NULL, cbdata, 2);
    assert(n == 0);

    printf("ALL RINGBUF TESTS PASSED.\n");
    return 0;
}
