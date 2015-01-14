#include <oh.h>
#include <stdio.h>
#include <check.h>
#include <unistd.h>
#include <sys/socket.h>

void evt_timer_quit_loop_cb(evt_loop *loop, evt_timer* ev) {
    evt_pool_quit((evt_pool*)ev->data);

}

void evt_timer_destroy_loop_cb(evt_loop *loop, evt_timer* ev) {
    evt_pool_destroy((evt_pool*)ev->data);

}

START_TEST(test_evtpool)
{
    evt_pool* pool = evt_pool_init(4);

    evt_timer evtm;
    evt_timer_init(&evtm, evt_timer_quit_loop_cb, SECOND(4), 0);
    evt_set_data(&evtm, pool);
    evt_timer_start(pool->loops[0], &evtm);
    evt_pool_run(pool);

    evt_timer_stop(pool->loops[0], &evtm);
    evt_timer_init(&evtm, evt_timer_destroy_loop_cb, SECOND(5), 0);
    evt_set_data(&evtm, pool);
    evt_timer_start(pool->loops[0], &evtm);
    evt_pool_run(pool);
    oh_memdbg_print_rec();
}
END_TEST



Suite *evtpool_test_suite() {
    Suite *s = suite_create("=== EvtPool Test ===");
    TCase *tc_core = tcase_create("TC core");
    tcase_set_timeout(tc_core, 1000);

    tcase_add_test(tc_core, test_evtpool);
    suite_add_tcase(s, tc_core);
    return s;
}