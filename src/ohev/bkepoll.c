#include <string.h>
#include <errno.h>
#include <ohev/bkepoll.h>
#include <ohev/log.h>


void *_epoll_init(evt_loop* loop) {
    _epoll_data *epd = (_epoll_data*)ohmalloc(sizeof(_epoll_data));
    memset(epd, 0, sizeof(_epoll_data));

    /* set epoll fd close on exec */
#ifdef EPOLL_CLOEXEC
    epd->fd = epoll_create1(EPOLL_CLOEXEC);
#else
    epd->fd = epoll_create(1);
    if (epd->fd >= 0) {
        fd_cloexec(epd->fd);
    }
#endif

    if (epd->fd < 0) {
       goto epoll_init_failed;
    }
    epd->nevent = _EPOLL_INIT_NEVENT;
    epd->events = (epoll_event*)ohmalloc(sizeof(epoll_event) * _EPOLL_INIT_NEVENT);
    memset(epd->events, 0, sizeof(epoll_event) * _EPOLL_INIT_NEVENT);

    if (epd->events == NULL) {
        goto epoll_init_failed;
    }

    /* init function pointer of loop */
    loop->poll_destroy  = _epoll_destroy;
    loop->poll_dispatch = _epoll_dispatch;
    loop->poll_update   = _epoll_update;

    log_inner("epoll(%d) init complete!", epd->fd);
    return epd;

epoll_init_failed:
    log_error("epoll init failed");
    if (epd->fd >= 0) {
        close (epd->fd);
    }
    if (epd->events) {
        fn_free(epd->events);
    }
    fn_free(epd);
    return NULL;
}

void _epoll_destroy(evt_loop* loop) {
    _epoll_data *epd = (_epoll_data*)loop->poll_data;

    log_inner("epoll(%d) destroyd!", epd->fd);
    if (epd->fd >= 0) {
        close (epd->fd);
    }
    if (epd->events) {
        fn_free(epd->events);
    }
    fn_free(epd);

}

int _epoll_update(evt_loop* loop, int fd, uint8_t oev, uint8_t nev) {
    /* don't delete here */
    if (nev == 0) {
        return;
    }

    _epoll_data *epd = (_epoll_data*)loop->poll_data;
    epoll_event evt;
    evt.data.fd = fd;
    evt.events = (((nev & _EVT_READ)  ? EPOLLIN : 0)
                | ((nev & _EVT_WRITE) ? EPOLLOUT : 0));

    /* nev != 0 && oev != 0  => EPOLL_CTL_MOD
     * nev == 0 && oev != 0  => EPOLL_CTL_DEL
     * nev != 0 && oev == 0  => EPOLL_CTL_ADD
     */
    if (epoll_ctl(epd->fd, nev ? (oev ? EPOLL_CTL_MOD : EPOLL_CTL_ADD) : EPOLL_CTL_DEL, fd, &evt)) {
        log_inner("epoll(%d) %s fd(%d)", epd->fd, nev ? (oev ? "mod" : "add") : "del", fd);
        /* del a not registered fd                  => success
         * mod a not registered fd but add success  => success
         * add a registered fd but mod success      => success
         * other case                               => failed
         */
        if ((errno == ENOENT && nev == 0) ||
            (errno == ENOENT && !epoll_ctl(epd->fd, EPOLL_CTL_ADD, fd, &evt)) ||
            (errno == EEXIST && !epoll_ctl(epd->fd, EPOLL_CTL_MOD, fd, &evt))) {
            return 0;
        } else {
            log_warn("epoll(%d) update on fd:%d failed.", epd->fd, fd);
            return -1;
        }
    }
    return 0;
}

int _epoll_dispatch(evt_loop* loop) {
    int i, j;
    _epoll_data *epd = (_epoll_data*)loop->poll_data;

    /* be careful here! 
     * wait_ms == 0 => no wait
     * wait_ms  < 0 => block unit event happen
     * wait_ms  > 0 => block specified time
     * epoll's precision is ms, let it more than poll_time_us to avoid return many times in 1ms
     */
    int wait_ms = 0;
    if (loop->poll_time_us < 0) {
        wait_ms = -1;
    } else if (loop->poll_time_us > 0) {
        wait_ms = loop->poll_time_us / 1000 + 1;
    }

    int evtcnt = epoll_wait(epd->fd, epd->events, epd->nevent, wait_ms);
    log_inner("epoll(%d) return %d events.", epd->fd, evtcnt);

    if (evtcnt < 0) {
        /* return error when epoll_wait return  error and wasn't caused by interruption */
        return errno == EINTR ? 0 : -1;
    }

    /* append event to pending queue */
    for (i = 0; i < evtcnt; i++) {
        epoll_event *evt = epd->events + i;
        int fd = evt->data.fd;
        fd_info *fdi = loop->fds + fd;

        uint8_t rev = (((evt->events & (EPOLLIN  | EPOLLERR | EPOLLHUP)) ? _EVT_READ  : 0) |
                       ((evt->events & (EPOLLOUT | EPOLLERR | EPOLLHUP)) ? _EVT_WRITE : 0));
        fdi->revent = rev;

        /* if focue rev not in focus ev, fix it */
        int want = fdi->event;
        if (rev & ~want) {
            evt->events = (((want & _EVT_READ)  ? EPOLLIN : 0)
                        | ((want & _EVT_WRITE) ? EPOLLOUT : 0));
            if (epoll_ctl(epd->fd, want ? EPOLL_CTL_MOD : EPOLL_CTL_DEL, fd, evt)) {
                log_warn("epoll(%d) update on fd:%d failed.", epd->fd, fd);
            } else {
                log_inner("epoll(%d) %s fd(%d)", epd->fd, want ? "mod" : "del", fd);
            }
        }

        evt_base *eb;
        for_splst_each(fdi->evq, eb) {
            evt_io *eio = (evt_io*)eb;
            if (eio->event & rev) {
                _evt_append_pending(loop, (evt_base*)eio);
            }
        }


    }

    /* if evtcnt == nevent, expand events array */
    if (evtcnt == epd->nevent && epd->nevent < _EPOLL_MAX_NEVENT) {
        check_and_expand_array(epd->events, epd->nevent, epd->nevent * 2,
            expand_multi_two, init_array_none);
        log_inner("epoll(%d) event size expand to %d", epd->fd, epd->nevent);
    }

}
