#include <string.h>
#include <ohev/bkepoll.h>
#include <ohev/log.h>


void *_epoll_init(evt_loop* loop) {
    _epoll_data *epd = (_epoll_data*)fn_malloc(sizeof(_epoll_data));
    memset(epd, 0, sizeof(_epoll_data));

    /* set close on exec */
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
    epd->events = (struct epoll_event*)fn_malloc(sizeof(struct epoll_event) * _EPOLL_INIT_NEVENT);

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

int _epoll_dispatch(evt_loop* loop) {

}

int _epoll_update(evt_loop* loop, int fd, uint8_t oev, uint8_t nev) {
    // _epoll_data *epd = (_epoll_data*)loop->poll_data;
    // epoll_event evt;
    // evt.data.fd = fd;
    // evt.events = (((nev & EVT_READ)  ? EPOLLIN : 0)
    //             | ((nev & EVT_WRITE) ? EPOLLOUT : 0);

    // /* do not focus on this fd*/
    // if (nev == 0) {

    // /* do not focus on this fd previous , add it */
    // } else if (oev == 0) {
    //     log_inner("epoll_update do EPOLL_CTL_ADD with fd: %d", fd);
    //     if (epoll_ctl(ept->fd, EPOLL_CTL_ADD, fd, &evt)) {

    //     }
    // }
}