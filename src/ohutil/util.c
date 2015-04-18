#include <ohutil/util.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int make_daemon() {
    int i;
    pid_t pid;

    if ((pid = fork()) < 0) 
        return -1;
    else if (pid)
        _exit(0);

    if (setsid() < 0)
        return -1;

    signal(SIGHUP, SIG_IGN);

    if ((pid = fork()) < 0) 
        return -1;
    else if (pid)
        _exit(0);

    int r = chdir("/");         //change working directory
    for (i = 0; i < 64; i++) close(i);
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
    return 0;
}