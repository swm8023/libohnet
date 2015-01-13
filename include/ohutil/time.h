#ifndef OHUTIL_TIME_H
#define OHUTIL_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>
#include <time.h>
#include <stdint.h>

typedef int64_t ohtime_t;
#define US_ONE_SEC      1000000
#define SECOND(x)       ((x) * 1000000)
#define TIMESTR_LEN     25

void enable_time_cache();
void disable_time_cache();

ohtime_t get_time();
ohtime_t get_catime();
ohtime_t update_catime();

ohtime_t get_time_str(char *, int);
ohtime_t get_catime_str(char *, int);
ohtime_t ohusleep(ohtime_t);
void ohtime_to_str(ohtime_t, char *, int);
void ohtime_to_timespec(ohtime_t , struct timespec *);
void ohtime_to_timeval(ohtime_t,struct timeval *);
ohtime_t timespec_to_ohtime(struct timespec *);
ohtime_t timeval_to_ohtime(struct timeval *);

/* compatibility with old version*/
#define fntime_t ohtime_t

#ifdef __cplusplus
}
#endif
#endif  //OHUTIL_UTIL_H