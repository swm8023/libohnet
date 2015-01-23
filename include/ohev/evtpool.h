#ifndef OHEV_EVTPOOL_H
#define OHEV_EVTPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <ohev/evt.h>


#define _POOL_STATUS_INIT         0x01
#define _POOL_STATUS_STARTED      0x02
#define _POOL_STATUS_RUNNING      0x04
#define _POOL_STATUS_SUSPEND      0x08
#define _POOL_STATUS_QUITING      0x10
#define _POOL_STATUS_STOP         0x20
#define _POOL_STATUS_WAITDESTROY  0x40

typedef struct _tag_evt_pool {
    int owner_tid;
    uint8_t status;

    evt_loop** loops;
    int loops_num;
    thread_t *loops_pid;
} evt_pool;


evt_pool* evt_pool_init_flag(int, int);
evt_pool* evt_pool_init(int);
void evt_pool_destroy(evt_pool*);
void evt_pool_quit(evt_pool*);
void evt_pool_run(evt_pool*);




#ifdef __cplusplus
}
#endif
#endif  //OHEV_EVTPOOL_H