#ifndef OHEV_EVT_H
#define OHEV_EVT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <ohev/log.h>

#include <ohutil/util.h>


typedef struct _tag_evt_loop evt_loop;

#define _EVT_CALLBACK(stype) void (*cb)

#define _EVT_BASE(type)             \
    uint8_t active;                 \
    uint8_t priority;               \
    int pendpos;                    \
    void *data;                     \
    void (*cb)(evt_loop*, type*);   \
    SPLST_DEFNEXT(type);

typedef struct _tag_evt_base {
    _EVT_BASE(struct _tag_evt_base);
} evt_base;

typedef struct _tag_evt_base_list {
    _EVT_BASE(struct _tag_evt_base);
} evt_base_list;

typedef struct _tag_evt_io {
    _EVT_BASE(struct _tag_evt_io);

    int fd;
    uint8_t event;
} evt_io;

typedef struct _tag_evt_timer {
    _EVT_BASE(struct _tag_evt_timer);

    ohtime_t repeat;
    ohtime_t timestamp;

    int heap_pos;
} evt_timer;


typedef struct _tag_evt_before {
    _EVT_BASE(struct _tag_evt_before);
} evt_before;

typedef evt_before evt_after;

typedef struct _tag_evt_async {
    _EVT_BASE(struct _tag_evt_async);

    void (*clean)(struct _tag_evt_async*);
} evt_async;


#define evt_set_data(ev_, data_) ((ev_)->data = data_)

#define evt_base_init(ev_, cb_) do { \
    (ev_)->active    = 0;            \
    (ev_)->priority  = 0;            \
    (ev_)->pendpos   = 0;            \
    (ev_)->data      = NULL;         \
    (ev_)->cb        = (cb_);        \
} while (0)

#define evt_io_init(ev_, cb_, fd_, event_) do { \
    evt_base_init((ev_), (cb_));                \
    (ev_)->event = (event_);                    \
    (ev_)->fd    = (fd_);                       \
} while (0)

#define evt_timer_init(ev_, cb_, after_, repeat_) do {  \
    evt_base_init((ev_), (cb_));                        \
    (ev_)->repeat = (repeat_);                          \
    (ev_)->timestamp = (after_);                        \
} while (0)

#define evt_before_init(ev_, cb_) evt_base_init((ev_), (cb_))

#define evt_after_init(ev_, cb_)  evt_base_init((ev_), (cb_))

#define evt_async_init(ev_, cb_, clean_) do {   \
    evt_base_init((ev_), (cb_));                \
    (ev_)->clean = (clean_);                    \
} while (0)

void evt_io_start(evt_loop*, evt_io*);
void evt_io_stop(evt_loop*, evt_io*);
void evt_timer_start(evt_loop*, evt_timer*);
void evt_timer_stop(evt_loop*, evt_timer*);
void evt_before_start(evt_loop*, evt_before*);
void evt_before_stop(evt_loop*, evt_before*);
void evt_after_start(evt_loop*, evt_after*);
void evt_after_stop(evt_loop*, evt_after*);
void evt_async_start(evt_loop*, evt_async*);
void evt_async_stop(evt_loop*, evt_async*);


#define _FDS_FLAG_CHANGED 0x01

typedef struct _tag_fd_info {
    evt_base *evq;
    uint8_t event;
    uint8_t revent;
    uint8_t flag;
} fd_info;

#define EVENT_PARAM_EIOST 1

typedef struct _tag_event_param {
    uint8_t type;
    uint8_t temp;
    void *arg;
} event_param;


/* io event */
#define EVTIO_READ  _EVT_READ
#define EVTIO_WRITE _EVT_WRITE

#define _EVT_READ   0x01
#define _EVT_WRITE  0x02

/* timer event */
#define _EVTTIMER_CMP(ta, tb) ((ta)->timestamp < (tb)->timestamp)



/* evt_loop */
#define _LOOP_STATUS_INIT    0x01
#define _LOOP_STATUS_STARTED 0x02
#define _LOOP_STATUS_RUNNING 0x04
#define _LOOP_STATUS_SUSPEND 0x08
#define _LOOP_STATUS_QUITING 0x10
#define _LOOP_STATUS_STOP    0x20
#define _LOOP_STATUS_WAITDESTROY 0x40

#define _LOOP_INIT_FDSSIZE      32
#define _LOOP_INIT_PENDSIZE     32
#define _LOOP_INIT_EVTSIZE      32
#define _LOOP_PRIORITY_MAX      10
#define _LOOP_PRIORITY_INIT_MAX 0
#define _LOOP_INIT_POLLUS       30000000

typedef struct _tag_evt_loop {
    /* status */
    int owner_tid;
    uint8_t status;

    /* config */
    uint8_t priority_max;   /* 0 ~ max*/

    /* io event */
    fd_info *fds;
    int fds_size;
    int *fds_change;
    int fds_change_size;
    int fds_change_cnt;

    /* before event && after event */
    evt_before *before_evtq;
    evt_after  *after_evtq;

    /* timer event */
    evt_timer** timer_heap;
    int timer_heap_cnt;
    int timer_heap_size;

    /* event pending to run */
    evt_base **pending[_LOOP_PRIORITY_MAX];
    int pending_cnt[_LOOP_PRIORITY_MAX];
    int pending_size[_LOOP_PRIORITY_MAX];

    /* async evnet */
    int evtfd;
    uint8_t evtfd_ref;
    evt_io* evtfd_ev;
    evt_async *async_evtq;
    evt_async *async_evtq_last;
    mutex_t async_mutex;

    /* backend */
    int poll_feature;
    int64_t poll_time_us;      /* in microsecond */
    void *poll_data;
    void *(*poll_init)(evt_loop*);
    void (*poll_destroy)(evt_loop*);
    int (*poll_dispatch)(evt_loop*);
    int (*poll_update)(evt_loop*, int, uint8_t, uint8_t);

    /* quit lock */
    cond_t quit_cond;

    /* some default callback */
    /* be used when stop a pending event */
    evt_base *empty_ev;

} evt_loop;

evt_loop* evt_loop_init_flag(int);
evt_loop* evt_loop_init();
int evt_loop_quit(evt_loop*);;
int evt_loop_destroy(evt_loop*);
int evt_loop_run(evt_loop*);
int evt_loop_wakeup(evt_loop*);
/* not implement */
int evt_loop_suspend(evt_loop*);
int evt_loop_ready(evt_loop*);

void _evt_do_wakeup(evt_loop*, evt_io*);
void _evt_fd_change(evt_loop*, int);
void _evt_append_pending(evt_loop*, evt_base*);
void _evt_execute_pending(evt_loop*);
void _evt_update_fdchanges(evt_loop*);


#ifdef __cplusplus
}
#endif
#endif  //FNEV_EVT_H