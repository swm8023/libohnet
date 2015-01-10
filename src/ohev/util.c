#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>

#include <ohev/util.h>
#include <ohev/log.h>


int fd_reuse(int fd) {
    const int on = 1;
#ifdef SO_REUSEPORT
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(int));
#elif defined SO_REUSEADDR
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
#endif
    return 0;
}

int fd_cloexec(int fd) {
    int flags = fcntl(fd, F_GETFD, 0);
    fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
    return 0;
}

int fd_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    return 0;
}

int ignore_sigpipe() {
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &act, NULL) != 0){
        log_error("ignore SIGPIPRI error()");
    }
}
