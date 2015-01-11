#include <string.h>
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

    loop->owner_thread = 0;

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
    loop->priority_max = _LOOP_PRIORITY_INIT_MAX;
    for (i = 0; i <= loop->priority_max; i++) {
        loop->pending_size[i] = _LOOP_INIT_PENDSIZE;
        loop->pending_cnt[i]  = 0;
        loop->pending[i] = (evt_base**)ohmalloc(sizeof(evt_base*) * _LOOP_INIT_PENDSIZE);
    }

    /* a empty event do nothing, used when remove a pending event */
    loop->empty_ev = (evt_base*)ohmalloc(sizeof(evt_base));
    evt_base_init(loop->empty_ev, NULL);

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
    loop->owner_thread = thread_id();
    loop->status = _LOOP_STATUS_STARTED | _LOOP_STATUS_RUNNING;

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
        while (loop->timer_heap_cnt > 0) {
            evt_timer *evtm = loop->timer_heap[1];
            if (evtm->timestamp > get_catime())
                break;
            _evt_append_pending(loop, (evt_base*)evtm);

            /* remove from heap */
            spheap_pop(loop->timer_heap, loop->timer_heap_cnt, _EVTTIMER_CMP);

            /* if repeat > 0, start again */
            if (evtm->repeat > 0) {
                evtm->active = 0;
                evtm->timestamp = evtm->repeat;
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
}

void evt_io_start(evt_loop* loop, evt_io* ev) {
    /* alredy start, avoid start again*/
    if (ev->active == 1) {
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
    if (ev->active == 0) {
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
    if (ev->active == 1) {
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
    if (ev->active == 0) {
        ev->active = 0;
    }

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
    if (ev->active == 1) {
        return;
    }
    ev->active = 1;
    adj_between(ev->priority, 0, loop->priority_max);

    splst_add(loop->before_evtq, ev);
}

void evt_before_stop(evt_loop* loop, evt_before* ev) {
    if (ev->active == 0) {
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
    if (ev->active == 1) {
        return;
    }
    ev->active = 1;
    adj_between(ev->priority, 0, loop->priority_max);

    splst_add(loop->after_evtq, ev);
}

void evt_after_stop(evt_loop* loop, evt_after* ev) {
    if (ev->active == 0) {
        return;
    }
    ev->active = 0;
    if (ev->pendpos) {
        loop->pending[ev->priority][ev->pendpos - 1] = loop->empty_ev;
        ev->pendpos = 0;
    }

    splst_del(loop->after_evtq, ev);
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