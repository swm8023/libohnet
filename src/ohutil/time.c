#include <sys/select.h>
#include <stdio.h>
#include <stdint.h>
#include <ohutil/time.h>

static int _is_time_cache = 1;
static __thread ohtime_t _cached_time = 0;

void enable_time_cache() {
    _is_time_cache = 1;
}

void disable_time_cache() {
    _is_time_cache = 0;
}

ohtime_t get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * US_ONE_SEC + tv.tv_usec;
}

ohtime_t get_catime() {
    return (_cached_time && _is_time_cache) ?
        _cached_time : (_cached_time = get_time());
}

ohtime_t update_catime() {
    return _cached_time = get_time();
}

ohtime_t get_time_str(char *buf, int len) {
    if (len < TIMESTR_LEN) {
        return -1;
    }
    ohtime_t us_now = get_time();
    ohtime_to_str(us_now, buf, len);
    return us_now;
}

ohtime_t get_catime_str(char *buf, int len) {
    if (len < TIMESTR_LEN) {
        return -1;
    }
    ohtime_t us_canow = get_catime();
    ohtime_to_str(us_canow, buf, len);
    return us_canow;
}

void ohtime_to_str(ohtime_t us_now, char *buf, int len) {
    struct tm tm_time;
    time_t sec_now = us_now / US_ONE_SEC;
    localtime_r(&sec_now, &tm_time);
    snprintf(buf, len, "%4d%02d%02d %02d:%02d:%02d.%06d",
      tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
      tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
      (int)(us_now % US_ONE_SEC));
}

void ohtime_to_timespec(ohtime_t ohtime, struct timespec *spectime) {
    spectime->tv_sec  = ohtime / US_ONE_SEC;
    spectime->tv_nsec = ohtime % US_ONE_SEC * 1000;
}

void ohtime_to_timeval(ohtime_t ohtime, struct timeval *valtime) {
    valtime->tv_sec  = ohtime / US_ONE_SEC;
    valtime->tv_usec = ohtime %US_ONE_SEC;
}

ohtime_t timespec_to_ohtime(struct timespec *spectime) {
    return spectime->tv_sec + spectime->tv_nsec / 1000;
}

ohtime_t timeval_to_ohtime(struct timeval *valtime) {
    return valtime->tv_sec + valtime->tv_usec;
}

ohtime_t fnusleep(ohtime_t sleepus) {
    struct timeval tv;
    ohtime_t sleepus_real;
    int ret;

    ohtime_to_timeval(sleepus, &tv);

    sleepus_real = get_time();
    if (0 == (ret = select(0, NULL, NULL, NULL, &tv))) {
        return 0;
    }

    sleepus_real = get_time() - sleepus_real;
    return sleepus_real >= sleepus ? 0: sleepus - sleepus_real;
}