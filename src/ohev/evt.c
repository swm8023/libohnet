#include <string.h>
#include <sys/eventfd.h>
#include <ohev/evt.h>
#include <ohev/log.h>
#include <ohev/bkepoll.h>



evt_loop* evt_loop_init() {
    return evt_loop_init_flag(0);
}

evt_loop* evt_loop_init_flag(int flag) {
    int i;

    evt_loop *loop = (evt_loop*)ohmalloc(sizeof(evt_loop));
    if (loop == NULL) {
        goto evt_loop_init_failed;
    }

    memset(loop, 0, sizeof(evt_loop));

    /* LOOP STATUS => init */
    loop->status = _LOOP_STATUS_INIT;

    loop->owner_tid = 0;

    /* init fds and fd change queue*/
    loop->fds_size = _LOOP_INIT_FDSSIZE;
    loop->fds = (fd_info*)ohmalloc(sizeof(fd_info) * _LOOP_INIT_FDSSIZE);
    memset(loop->fds, 0, sizeof(fd_info) * _LOOP_INIT_FDSSIZE);

    loop->fds_change_cnt = 0;
    loop->fds_change_size = _LOOP_INIT_FDSSIZE;
    loop->fds_change = (int*)ohmalloc(sizeof(int) * _LOOP_INIT_FDSSIZE);
    memset(loop->fds_change, 0, sizeof(int) * _LOOP_INIT_FDSSIZE);

    /* before and after event */
    loop->before_evtq = NULL;
    loop->after_evtq  = NULL;

    /* timer event */
    loop->timer_heap_cnt = 0;
    loop->timer_heap_size = _LOOP_INIT_EVTSIZE;
    loop->timer_heap = (evt_timer**)ohmalloc(sizeof(evt_timer*) * _LOOP_INIT_EVTSIZE);

    /* event fd, used to wake up poll_wait to do sth. */
#if (defined(EFD_CLOEXEC) && defined(EFD_NONBLOCK))
    loop->evtfd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
#else
    loop->evtfd = eventfd(0, 0);
    if (loop->evtfd >= 0) {
        fd_cloexec(loop->evtfd);
        fd_nonblock(loop->evtfd);
    }
#endif

    if (loop->evtfd < 0) {
        goto evt_loop_init_failed;
    }
    loop->evtfd_ev = (evt_io*)ohmalloc(sizeof(evt_io));
    evt_io_init(loop->evtfd_ev, _evt_do_wakeup, loop->evtfd, EVTIO_READ);
    evt_io_start(loop, loop->evtfd_ev);

    /* async event */
    loop->async_evtq = NULL;
    loop->async_mutex = mutex_init(NULL);

    /* init backend poll */
    if (0) {
        /* only support epoll now */
        goto evt_loop_init_failed;
    } else {
        loop->poll_init = _epoll_init;
    }
    loop->poll_time_us = _LOOP_INIT_POLLUS;
    loop->poll_data    = loop->poll_init(loop);

    /* quit lock*/
    loop->quit_cond = cond_init(NULL);

    /* pending queue, only one queue on init  */
    loop->priority_max = _LOOP_PRIORITY_INIT_MAX;
    for (i = 0; i <= loop->priority_max; i++) {
        loop->pending_size[i] = _LOOP_INIT_PENDSIZE;
        loop->pending_cnt[i]  = 0;
        loop->pending[i] = (evt_base**)ohmalloc(sizeof(evt_base*) * _LOOP_INIT_PENDSIZE);
    }

    /* a empty event do nothing, used when remove a pending event */
    loop->empty_ev = (evt_base*)ohmalloc(sizeof(evt_base));
    evt_base_init(loop->empty_ev, NULL);

    log_inner("event loop(%d) init.", thread_tid());
    return loop;

evt_loop_init_failed:
    log_error("event loop(%d) init failed!", thread_tid());
    if (loop != NULL) {
        evt_loop_destroy(loop);
    }
    return NULL;
}

int evt_loop_quit(evt_loop* loop) {
    loop->status |= _LOOP_STATUS_QUITING;

    /* wake up loop when not quit in loop thread */
    if (loop->owner_tid != thread_tid()) {
        evt_loop_wakeup(loop);
    }

    return 0;
}

int evt_loop_destroy(evt_loop* loop) {
    int i;
    if (loop == NULL) {
        log_warn("destroy a NULL loop");
        return 0;
    }
    /* if running, stop first */
    if (loop->status & _LOOP_STATUS_RUNNING) {
        loop->status |= _LOOP_STATUS_WAITDESTROY;
        evt_loop_quit(loop);
        return 0;
        /* not stop in loop thread, wait for loop quit */
        // if (loop->owner_tid != thread_tid()) {
        //     cond_lock(loop->quit_cond);
        //     while (loop->status & _LOOP_STATUS_RUNNING) {
        //         cond_wait(loop->quit_cond);
        //     }
        //     cond_unlock(loop->quit_cond);
        // /* stop in loop thread, set wait destroy flag and destroy after loop quit */
        // } else {
        //     loop->status |= _LOOP_STATUS_WAITDESTROY;
        //     return 0;
        // }
    }

    /* free memory */
    ohfree(loop->fds);
    ohfree(loop->fds_change);
    ohfree(loop->timer_heap);
    for (i = 0; i <= loop->priority_max; i++) {
        ohfree(loop->pending[i]);
    }

    ohfree(loop->evtfd_ev);
    close(loop->evtfd);

    /* clean all async event */
    evt_async *async_head = loop->async_evtq;
    while (async_head) {
        /* async_head may be free in evt_async->clean, so let it be next element now */
        evt_async *async_now = async_head;
        async_head = async_head->next;
        if (async_now->clean) {
            async_now->clean(async_now);
        }
    }
    mutex_destroy(loop->async_mutex);

    if (loop->poll_destroy) {
        loop->poll_destroy(loop);
    }

    cond_destroy(loop->quit_cond);
    ohfree(loop->empty_ev);

    ohfree(loop);

    log_inner("event loop(%d) destroy.", thread_tid());
    return 0;
}

int evt_loop_run(evt_loop* loop) {
    loop->owner_tid = thread_tid();
    loop->status = _LOOP_STATUS_STARTED | _LOOP_STATUS_RUNNING;

    log_inner("event loop(%d) running.", thread_tid());
    
    /* do while util loop->status be set quiting */
    while (!(loop->status & _LOOP_STATUS_QUITING)) {
        evt_base* eb;

        /* time point before poll_dispatch */
        update_catime();

        /* queue and execute before events */
        for_splst_each((evt_base*)loop->before_evtq, eb) {
            _evt_append_pending(loop, eb);
        }
        _evt_execute_pending(loop);

        /* maybe a quit flag is set here */
        if (_LOOP_STATUS_QUITING & loop->status) {
            break;
        }

        /* update fd changes */
        _evt_update_fdchanges(loop);

        /* update time before cal the poll time */
        update_catime();
        loop->poll_time_us = _LOOP_INIT_POLLUS;
        if (loop->timer_heap_cnt > 0 && (loop->poll_time_us < 0 ||
            loop->timer_heap[1]->timestamp - get_catime() < loop->poll_time_us)) {
            loop->poll_time_us = loop->timer_heap[1]->timestamp - get_catime();
            if (loop->poll_time_us < 0) {
                loop->poll_time_us = 0;
            }
        }

        /* do poll_dispatch */
        loop->poll_dispatch(loop);

        /* time point after poll_dispatch */
        update_catime();

        /* maybe waken up by evt_loop_quit, then quit while */
        if (_LOOP_STATUS_QUITING & loop->status) {
            break;
        }

        /* append timeout timers to pending queue */
        while (loop->timer_heap_cnt > 0) {
            evt_timer *evtm = loop->timer_heap[1];
            if (evtm->timestamp > get_catime())
                break;
            _evt_append_pending(loop, (evt_base*)evtm);

            /* remove event from timer heap */
            spheap_pop(loop->timer_heap, loop->timer_heap_cnt, _EVTTIMER_CMP);

            /* if repeat > 0, start again, calculate timestamp carefully */
            if (evtm->repeat > 0) {
                evtm->active = 0;
                evtm->timestamp = evtm->timestamp - get_catime() + evtm->repeat;
                evt_timer_start(loop, evtm);
            }
        }

        /* queue after events */
        for_splst_each((evt_base*)loop->after_evtq, eb) {
            _evt_append_pending(loop, eb);
        }
        /* execute timer,io,after events */
        _evt_execute_pending(loop);
    }

    log_inner("event loop(%d) quit.", thread_tid());
    /* quited */
    loop->status &= ~_LOOP_STATUS_RUNNING;
    loop->status &= ~_LOOP_STATUS_QUITING;
    loop->status &= ~_LOOP_STATUS_SUSPEND;
    loop->status |= _LOOP_STATUS_STOP;

    /* if destroy in loop thread */
    if (loop->status & _LOOP_STATUS_WAITDESTROY) {
        evt_loop_destroy(loop);
    /* if destroy in other thread */
    }
    //  else {
    //     cond_broadcast(loop->quit_cond);
    // }
}

void evt_io_start(evt_loop* loop, evt_io* ev) {
    if (loop->owner_tid && loop->owner_tid != thread_tid()) {
        log_warn("can't start an io event with a running loop in another thread!");
        return;
    }
    if (ev->active == 1) {
        //log_warn("start an active io event!");
        return;
    }
    ev->active = 1;
    adj_between(ev->priority, 0, loop->priority_max);

    int oldsize = loop->fds_size;
    check_and_expand_array(loop->fds, loop->fds_size, ev->fd + 1,
        expand_multi_two, init_array_zero);

    if (loop->fds_size != oldsize) {
        log_inner("fds size increase to %d.", loop->fds_size);
    }

    /* append event to list */
    fd_info *fdi = loop->fds + ev->fd;
    splst_add(fdi->evq, (evt_base*)ev);

    /* append fd to change queue */
    _evt_fd_change(loop, ev->fd);
}

void evt_io_stop(evt_loop* loop, evt_io* ev) {
    if (loop->owner_tid && loop->owner_tid != thread_tid()) {
        log_warn("can't stop an io event with a running loop in another thread!");
        return;
    }
    if (ev->active == 0) {
        //log_warn("stop an inactive io event!");
        return;
    }
    ev->active = 0;

    /* if ev is in pending queue, use a empty ev to replace it */
    if (ev->pendpos) {
        loop->pending[ev->priority][ev->pendpos - 1] = loop->empty_ev;
        ev->pendpos = 0;
    }

    /* remove from fd's event list */
    splst_del(loop->fds[ev->fd].evq, (evt_base*)ev);

    /* append fd to change queue */
    _evt_fd_change(loop, ev->fd);

}

void evt_timer_start(evt_loop* loop, evt_timer* ev) {
    if (loop->owner_tid && loop->owner_tid != thread_tid()) {
        log_warn("can't start a timer event with a running loop in another thread!");
        return;
    }
    if (ev->active == 1) {
        log_warn("start an active timer event!");
        return;
    }
    ev->active = 1;
    adj_between(ev->priority, 0, loop->priority_max);

    /* heap root is 1, so need + 2, cnt == heap_size*/
    check_and_expand_array(loop->timer_heap, loop->timer_heap_size,
        loop->timer_heap_cnt + 2, expand_multi_two, init_array_none);

    /* adjust to absolute time */
    ev->timestamp += get_catime();

    spheap_push(loop->timer_heap, loop->timer_heap_cnt, ev, _EVTTIMER_CMP);
}

void evt_timer_stop(evt_loop* loop, evt_timer* ev) {
    if (loop->owner_tid && loop->owner_tid != thread_tid()) {
        log_warn("can't stop a timer event with a running loop in another thread!");
        return;
    }
    if (ev->active == 0) {
        log_warn("stop an inactive timer event!");
        return;
    }
    ev->active = 0;

    if (ev->pendpos) {
        loop->pending[ev->priority][ev->pendpos - 1] = loop->empty_ev;
        ev->pendpos = 0;
    }

    /* also need delete this timer event from heap */
    if (ev->heap_pos > 0) {
        spheap_delete(loop->timer_heap, ev->heap_pos, loop->timer_heap_cnt, _EVTTIMER_CMP);
    }
}

void evt_before_start(evt_loop* loop, evt_before* ev) {
    if (loop->owner_tid && loop->owner_tid != thread_tid()) {
        log_warn("can't start a before event with a running loop in another thread!");
        return;
    }
    if (ev->active == 1) {
        log_warn("start an active before event!");
        return;
    }
    ev->active = 1;
    adj_between(ev->priority, 0, loop->priority_max);

    splst_add(loop->before_evtq, ev);
}

void evt_before_stop(evt_loop* loop, evt_before* ev) {
    if (loop->owner_tid && loop->owner_tid != thread_tid()) {
        log_warn("can't stop a before event with a running loop in another thread!");
        return;
    }
    if (ev->active == 0) {
        log_warn("stop an inactive before event!");
        return;
    }
    ev->active = 0;
    if (ev->pendpos) {
        loop->pending[ev->priority][ev->pendpos - 1] = loop->empty_ev;
        ev->pendpos = 0;
    }

    splst_del(loop->before_evtq, ev);
}

void evt_after_start(evt_loop* loop, evt_after* ev) {
    if (loop->owner_tid && loop->owner_tid != thread_tid()) {
        log_warn("can't start a after event with a running loop in another thread!");
        return;
    }
    if (ev->active == 1) {
        log_warn("start an active after event!");
        return;
    }
    ev->active = 1;
    adj_between(ev->priority, 0, loop->priority_max);

    splst_add(loop->after_evtq, ev);
}

void evt_after_stop(evt_loop* loop, evt_after* ev) {
    if (loop->owner_tid && loop->owner_tid != thread_tid()) {
        log_warn("can't stop a after event with a running loop in another thread!");
        return;
    }
    if (ev->active == 0) {
        log_warn("stop an inactive after event!");
        return;
    }
    ev->active = 0;
    if (ev->pendpos) {
        loop->pending[ev->priority][ev->pendpos - 1] = loop->empty_ev;
        ev->pendpos = 0;
    }

    splst_del(loop->after_evtq, ev);
}

int evt_loop_wakeup(evt_loop* loop) {
    /* just write data to evtfd to wake up epoll_dispatch */
    uint64_t val = 1;
    ssize_t wsize = write(loop->evtfd, &val, sizeof(uint64_t));
    if (wsize != sizeof(eventfd_t)) {
        log_error("eventfd_write error(%d)", wsize);
        return -1;
    }
    return 0;
}

void _evt_do_wakeup(evt_loop* loop, evt_io* ev) {
    if (loop->status & _LOOP_STATUS_QUITING) {
        return;
    }
    uint64_t val = 0;
    ssize_t rsize = read(loop->evtfd, &val, sizeof(uint64_t));
    log_inner("event loop(%d) wake up by %lld caller.", thread_tid(), val);
    if (rsize != sizeof(eventfd_t)) {
        log_error("eventfd_read error(%d)", rsize);
        return;
    }

    /* empty async event list */
    mutex_lock(loop->async_mutex);
    evt_async *async_head = loop->async_evtq;
    loop->async_evtq = NULL;
    mutex_unlock(loop->async_mutex);

    while (async_head) {
        /* async_head may be free in evt_async->clean, so let it be next element now */
        evt_async *async_now = async_head;
        async_head = async_head->next;

        /* call event callback and clean callback */
        if (async_now->cb) {
            async_now->cb(loop, async_now);
        }
        if (async_now->clean) {
            async_now->clean(async_now);
        }
    }
}

void evt_async_start(evt_loop* loop, evt_async* ev) {
    if (ev->active == 1) {
        log_warn("start an active async event!");
        return;
    }
    ev->active = 1;
    /* just inited or run in loop thread, add with no lock */
    if (loop->owner_tid == 0 || loop->owner_tid == thread_tid()) {
        splst_add(loop->async_evtq, ev);
    /* else add with lock */
    } else {
        mutex_lock(loop->async_mutex);
        splst_add(loop->async_evtq, ev);
        mutex_unlock(loop->async_mutex);
    }
}

void evt_async_stop(evt_loop* loop, evt_async* ev) {
    if (ev->active == 0) {
        log_warn("stop an inactive async event!");
        return;
    }
    ev->active = 0;
    /* just inited or run in loop thread, remove with no lock */
    if (loop->owner_tid == 0 || loop->owner_tid == thread_tid()) {
        splst_del(loop->async_evtq, ev);
    /* else remove with lock */
    } else {
        mutex_lock(loop->async_mutex);
        splst_del(loop->async_evtq, ev);
        mutex_unlock(loop->async_mutex);
    }
}


void _evt_append_pending(evt_loop* loop, evt_base* evt) {
    int pri = evt->priority;

    /* event's priority can't larger than priority_max */
    assert(pri <= loop->priority_max);

    /* only append event only if the event not in pending queue */
    if (evt->pendpos == 0) {
        int oldsize = loop->pending_size[pri];
        check_and_expand_array(loop->pending[pri], loop->pending_size[pri],
            loop->pending_cnt[pri] + 1, expand_multi_two, init_array_none);
        loop->pending[pri][loop->pending_cnt[pri]] = evt;
        evt->pendpos = ++loop->pending_cnt[pri];

        if (oldsize != loop->pending_size[pri]) {
            log_inner("pending[%d] size increase to %d.", pri, loop->pending_size[pri]);
        }
    }
}

void _evt_execute_pending(evt_loop* loop) {
    int i, j;

    /* execute high priority event first */
    for (i = 0; i <= loop->priority_max; i++) {
        for (j = 0; j < loop->pending_cnt[i]; j++) {
            evt_base *eb = loop->pending[i][j];
            eb->pendpos = 0;
            if (eb->cb) {
                eb->cb(loop, eb);
            }
        }
        /* clear the pending queue */
        loop->pending_cnt[i] = 0;
    }
}

void _evt_fd_change(evt_loop* loop, int fd) {
    fd_info *fdi = loop->fds + fd;

    /* ensure only append fd to fds_change queue once */
    if (!(fdi->flag & _FDS_FLAG_CHANGED)) {
        int oldsize = loop->fds_change_size;
        check_and_expand_array(loop->fds_change, loop->fds_change_size,
            loop->fds_change_cnt + 1, expand_multi_two, init_array_none);
        loop->fds_change[loop->fds_change_cnt] = fd;
        loop->fds_change_cnt++;
        /* set changed flag */
        fdi->flag |= _FDS_FLAG_CHANGED;

        if (oldsize != loop->fds_change_size) {
            log_inner("fds_change size increase to %d.", loop->fds_change_size);
        }
    }
}

void _evt_update_fdchanges(evt_loop* loop) {
    int i;
    evt_base *eb;

    for (i = 0; i < loop->fds_change_cnt; i++) {
        int fd = loop->fds_change[i];
        fd_info *fdi = loop->fds + fd;
        /* clear changed flag */
        fdi->flag &= ~_FDS_FLAG_CHANGED;

        uint8_t oev = fdi->event;
        uint8_t nev = 0;
        for_splst_each(fdi->evq, eb) {
            nev |= ((evt_io*)eb)->event;
        }
        /* update only if focus event really changed */
        if (oev != nev) {
            loop->poll_update(loop, fd, oev, fdi->event = nev);
        }
    }
    /* clear the queue */
    loop->fds_change_cnt = 0;
}

