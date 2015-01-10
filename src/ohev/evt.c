#include <string.h>
#include <ohev/evt.h>
#include <ohev/log.h>
#include <ohev/bkepoll.h>



evt_loop* evt_loop_init() {
    return evt_loop_init_flag(0);
}

evt_loop* evt_loop_init_flag(int flag) {
    int i;

    evt_loop *loop = (evt_loop*)fn_malloc(sizeof(evt_loop));
    if (loop == NULL) {
        goto evt_loop_init_failed;
    }

    memset(loop, 0, sizeof(evt_loop));

    loop->owner_thread = 0;
    loop->status = _LOOP_STATUS_INIT;

    loop->priority_max = 0;

    // loop->fds = vector_new(fd_info*);
    // loop->fds_mod = vector_new(fd_info*);

    // loop->bofore_evts = list_new(evt_before*);
    // loop->after_evts  = list_new(evt_after*);

    /* init backend poll */
    if (0) {
        /* only support epoll now */
        goto evt_loop_init_failed;
    } else {
        loop->poll_init = _epoll_init;
    }
    loop->poll_time_us = _LOOP_INIT_POLLUS;
    loop->poll_data    = loop->poll_init(loop);

    /* pending queue, only one queue on init  */
    // loop->pending[0] = vector_new(evt_base*);

    /* a empty event do nothing, used when remove a pending event */
    // loop->empty_ev = (evt_base*)fn_malloc(sizeof(evt_base));
    // evt_base_init(loop->empty_ev, NULL);

    return loop;

evt_loop_init_failed:
    if (loop != NULL) {
        evt_loop_destroy(loop);
    }
    return NULL;
}

int evt_loop_quit() {

}

int evt_loop_destroy(evt_loop* loop) {
    int i;
    if (loop == NULL) {
        log_warn("destroy a NULL loop");
        return 0;
    }

    return 0;
}

int evt_loop_run(evt_loop* loop) {

}