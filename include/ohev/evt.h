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
    void (*cb)(evt_loop*, type*);

#define _EVT_BASE_LIST(type)         \
    _EVT_BASE(type);                 \
    SPLST_DEFNEXT(struct _tag_evt_base_list);

typedef struct _tag_evt_base {
    _EVT_BASE(struct _tag_evt_base);
} evt_base;

typedef struct _tag_evt_base_list {
    _EVT_BASE_LIST(struct _tag_evt_base);
} evt_base_list;

typedef struct _tag_evt_io {
    _EVT_BASE_LIST(struct _tag_evt_io);

    int fd;
    uint8_t event;
} evt_io;

typedef struct _tag_evt_timer {
    _EVT_BASE(struct _tag_evt_timer);

    int64_t repeat;
    int64_t timestamp;
} evt_timer;

typedef struct _tag_evt_before {
    _EVT_BASE_LIST(struct _tag_evt_before);
} evt_before;

typedef struct _tag_evt_after {
    _EVT_BASE_LIST(struct _tag_evt_after);
} evt_after;

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
    (ev_)->event = (evtnt_);                    \
    (ev_)->fd    = (fd_);                       \
} while (0)

#define evt_timer_init(ev_, cb_, after_, repeat_) do {  \
    evt_base_init((ev_), (cb_));                        \
    (ev_)->repeat = (repeat_);                          \
    (ev_)->timestamp = (after_);                        \
} while (0)

#define evt_before_init(ev_, cb_) evt_base_init((ev_), (cb_))
#define evt_after_init(ev_, cb_)  evt_base_init((ev_), (cb_))

void evt_io_start(evt_loop*, evt_io*);
void evt_io_stop(evt_loop*, evt_io*);
void evt_timer_start(evt_loop*, evt_timer*);
void evt_timer_stop(evt_loop*, evt_timer*);
void evt_before_start(evt_loop*, evt_before*);
void evt_before_stop(evt_loop*, evt_before*);
void evt_after_start(evt_loop*, evt_after*);
void evt_after_stop(evt_loop*, evt_after*);

typedef struct _tag_fd_info {

    uint8_t event;
    uint8_t revents;
    uint8_t flag;
} fd_info;

#define EVENT_PARAM_EIOST 1

typedef struct _tag_event_param {
    uint8_t type;
    uint8_t temp;
    void *arg;
} event_param;


/* evt_loop */
#define _LOOP_STATUS_INIT    0x01
#define _LOOP_STATUS_STARTED 0x02
#define _LOOP_STATUS_RUNNING 0x04
#define _LOOP_STATUS_PAUSE   0x08
#define _LOOP_STATUS_QUITING 0x10
#define _LOOP_STATUS_STOP    0x20

#define _LOOP_INIT_FDS      32
#define _LOOP_INIT_PENDSIZE 32
#define _LOOP_INIT_EVTSIZE  32
#define _LOOP_PRIORITY_MAX  10
#define _LOOP_INIT_POLLUS   30000000

typedef struct _tag_evt_loop {
    /* status */
    int owner_thread;
    uint8_t status;

    /* config */
    uint8_t priority_max;   /* 0 ~ max*/

    /* io event */
    // vector_t *fds;          /* fd_info */
    // vector_t *fds_mod;      /* fd_info */

    //  before event && after event 
    // list_t *bofore_evts;    /* evt_before */
    // list_t *after_evts;     /* evt_after */

    /* timer event */
    evt_timer** timer_heap;
    int timer_heap_cnt;
    int timer_heap_size;

    /* event pending to run */
    // vector_t *pending[_LOOP_PRIORITY_MAX];

    /* queue of callback from other thread */
    // list_t *asyncq;
    mutex_t asyncq_lock;
    int asyncq_size;
    int asyncq_cnt;
    int eventfd;
    evt_io* eventio;

    /* backend */
    int poll_feature;
    int64_t poll_time_us;      /*in microsecond*/
    void *poll_data;
    void *(*poll_init)(evt_loop*);
    void (*poll_destroy)(evt_loop*);
    int (*poll_dispatch)(evt_loop*);
    int (*poll_update)(evt_loop*, int, uint8_t, uint8_t);

    /* quit lock */
    mutex_t quit_lock;
    cond_t quit_cond;

    /* some default callback */
    evt_base *empty_ev;   /* be used when stop a pending event */
} evt_loop;

evt_loop* evt_loop_init_flag(int);
evt_loop* evt_loop_init();
int evt_loop_quit();
int evt_loop_destroy(evt_loop*);
int evt_loop_run(evt_loop*);

void evt_append_pending(evt_loop*, void*);
void evt_execute_pending(evt_loop*);

#ifdef __cplusplus
}
#endif
#endif  //FNEV_EVT_H