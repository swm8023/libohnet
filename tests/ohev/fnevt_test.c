#include <oh.h>
#include <stdio.h>
#include <check.h>
#include <unistd.h>
#include <sys/socket.h>


void evt_io_rcb(evt_loop *loop, evt_io* ev) {
    //printf("rcb\n");
    evt_io_stop(loop, ev);
}

void evt_io_wcb(evt_loop *loop, evt_io* ev) {
    //printf("wcb\n");
    evt_io_stop(loop, ev);
}

void evt_before_cb(evt_loop *loop, evt_before* ev) {
    // printf("before\n");
    //evt_before_stop(loop, ev);
}

void evt_after_cb(evt_loop *loop, evt_after* ev) {
    // printf("after\n");
    //evt_after_stop(loop, ev);
}

void evt_timer_cb1(evt_loop *loop, evt_timer* ev) {
    log_debug("timer_cb1");
    //evt_after_stop(loop, ev);
}
void evt_timer_cb2(evt_loop *loop, evt_timer* ev) {
    log_debug("timer_cb2");
    evt_timer* ev2 = (evt_timer*)ev->data;
    if (ev2->active)
        evt_timer_stop(loop, ev2);
    //evt_after_stop(loop, ev);
}
void evt_timer_cb3(evt_loop *loop, evt_timer* ev) {
    log_debug("timer_cb3");
    //evt_after_stop(loop, ev);
    evt_timer* ev2 = (evt_timer*)ev->data;
    evt_timer_stop(loop, ev2);
    evt_loop_destroy(loop);

}

void evt_async_clean_cb(evt_async *ev) {
    log_debug("evt_async_clean_cb");
    ohfree(ev);
}

void evt_async_cb(evt_loop *loop, evt_async *ev) {
    log_debug("evt_async_cb");
}

THREAD_FUNC_START(thread_func1, arg) {
    evt_loop* loop = (evt_loop*)arg;

    ohusleep(SECOND(4.9));
    evt_async *ev = ohmalloc(sizeof(evt_async));
    evt_async_init(ev, evt_async_cb, evt_async_clean_cb);
    evt_async_start(loop, ev);
    evt_loop_wakeup(loop);

    ohusleep(SECOND(1.7));
    evt_async *ev2 = ohmalloc(sizeof(evt_async));
    evt_async_init(ev2, evt_async_cb, evt_async_clean_cb);
    evt_async_start(loop, ev2);
    evt_loop_wakeup(loop);
}
THREAD_FUNC_END

START_TEST(test_evt_single)
{
    int i;
    evt_loop* loop = evt_loop_init();
    evt_io evr[1000], evw[10000];
    evt_before evb[1000];
    evt_after eva[1000];
    for (i = 0; i < 1000; i++) {
        evt_before_init(&evb[i], evt_before_cb);
        evt_before_start(loop, &evb[i]);
        evt_after_init(&eva[i], evt_after_cb);
        evt_after_start(loop, &eva[i]);
    }
    for (i = 0; i < 100; i++) {
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        evt_io_init(&evr[i], evt_io_rcb, fd, EVTIO_READ);
        evt_io_start(loop, &evr[i]);
        evt_io_init(&evw[i], evt_io_wcb, fd, EVTIO_WRITE);
        evt_io_start(loop, &evw[i]);
    }

    evt_timer evtm[3];
    evt_timer_init(&evtm[0], evt_timer_cb1, SECOND(2), SECOND(1));
    evt_timer_init(&evtm[1], evt_timer_cb2, SECOND(5), SECOND(3));
    evt_timer_init(&evtm[2], evt_timer_cb3, SECOND(15), SECOND(1));
    evt_set_data(&evtm[1], &evtm[0]);
    evt_set_data(&evtm[2], &evtm[1]);
    evt_timer_start(loop, &evtm[0]);
    evt_timer_start(loop, &evtm[1]);
    evt_timer_start(loop, &evtm[2]);

    thread_t thid;
    thread_start(thid, thread_func1, loop);

    evt_loop_run(loop);

    oh_memdbg_print_rec();
}
END_TEST



Suite *evt_test_suite() {
    Suite *s = suite_create("=== Evt Test ===");
    TCase *tc_core = tcase_create("TC core");
    tcase_set_timeout(tc_core, 1000);

    tcase_add_test(tc_core, test_evt_single);
    suite_add_tcase(s, tc_core);
    return s;
}