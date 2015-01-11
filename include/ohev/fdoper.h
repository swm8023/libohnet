#ifndef OHEV_FDOPER_H
#define OHEV_FDOPER_H

#ifdef __cplusplus
extern "C" {
#endif


/* socket operation */
int fd_cloexec(int);
int fd_nonblock(int);
int fd_reuse(int);

int ignore_sigpipe();


#ifdef __cplusplus
}
#endif
#endif  //OHEV_FDOPER_H