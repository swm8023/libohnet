#ifndef OHEV_EVUTIL_H
#define OHEV_EVUTIL_H

#ifdef __cplusplus
extern "C" {
#endif



#define evmalloc fn_malloc
#define evfree fn_free


/* socket operation */
int fd_cloexec(int);
int fd_nonblock(int);
int fd_reuse(int);

int ignore_sigpipe();


#ifdef __cplusplus
}
#endif
#endif  //OHEV_EVUTIL_H