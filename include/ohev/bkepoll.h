#ifndef OHEV_BKEPOLL_H
#define OHEV_BKEPOLL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/epoll.h>

#include <ohev/evt.h>


#define _EPOLL_INIT_NEVENT   32
#define _EPOLL_MAX_NEVENT    4096

typedef struct epoll_event epoll_event;
typedef struct _tag_epoll_data {
    int fd;
    int feature;
    int nevent;
    epoll_event *events;
} _epoll_data;


void *_epoll_init(evt_loop*);
void _epoll_destroy(evt_loop*);
int _epoll_dispatch(evt_loop*);
int _epoll_update(evt_loop*, int, uint8_t, uint8_t);


#ifdef __cplusplus
}
#endif
#endif  //OHEV_BKEPOLL_H