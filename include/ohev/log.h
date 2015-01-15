#ifndef OHEV_LOG_H
#define OHEV_LOG_H

#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_INNER 0x01
#define LOG_TRACE 0x02
#define LOG_DEBUG 0x04
#define LOG_INFO  0x08
#define LOG_WARN  0x10
#define LOG_ERROR 0x20
#define LOG_FATAL 0x40
#define LOG_NONE  0x80
#define LOG_MASK  0x7F

#define LOG_INNER_INDEX 0
#define LOG_TRACE_INDEX 1
#define LOG_DEBUG_INDEX 2
#define LOG_INFO_INDEX  3
#define LOG_WARN_INDEX  4
#define LOG_ERROR_INDEX 5
#define LOG_FATAL_INDEX 6
#define LOG_LEVELS      7

//#define LOG_INDEX_BIT(p_index)        (POS_TO_BIT(p_index))

#define LOG_BUFSIZE 4096

/* log operation interface */
typedef struct _tag_log_if {
    uint8_t level_flag;
    void (*output_cb)(const char*, size_t);
    void (*flush_cb)();
    void (*level_cb[LOG_LEVELS])();
} log_if;

#define log_level_append(logif, level, fmt, arg...)  \
    if ((level) & (logif)->level_flag)               \
        _log_append(logif, level##_INDEX, fmt, ##arg)

#define set_logif_level(logif, level)        (logif->level_flag = (LOG_MASK ^ ((level) -1)))
#define get_logif_level_flag(logif)          (logif->level_flag)
#define set_logif_level_flag(logif, flag)    (logif->level_flag = (flag))
#define set_logif_output_cb(logif, cb)       (logif->output_cb = (cb))
#define set_logif_flush_cb(logif, cb)        (logif->flush_cb = (cb))
#define set_logif_level_cb(logif, level, cb) (logif->level_cb[level##_INDEX] = (cb))

#define logu_inner(logif, fmt, arg...) log_level_append(logif, LOG_INNER, fmt, ##arg)
#define logu_trace(logif, fmt, arg...) log_level_append(logif, LOG_TRACE, fmt, ##arg)
#define logu_debug(logif, fmt, arg...) log_level_append(logif, LOG_DEBUG, fmt, ##arg)
#define logu_info(logif, fmt, arg...)  log_level_append(logif, LOG_INFO, fmt, ##arg)
#define logu_warn(logif, fmt, arg...)  log_level_append(logif, LOG_WARN,  fmt, ##arg)
#define logu_error(logif, fmt, arg...) log_level_append(logif, LOG_ERROR, fmt, ##arg)
#define logu_fatal(logif, fmt, arg...) log_level_append(logif, LOG_FATAL, fmt, ##arg)


/* default log interface */
extern log_if *default_log_if;

#define set_default_logif_level(level)       set_logif_level(default_log_if, level)
#define get_default_logif_level_flag()       get_logif_level_flag(default_log_if)
#define set_default_logif_level_flag(flag)   set_logif_level_flag(default_log_if, flag)
#define set_default_logif_output_cb(cb)      set_logif_output_cb(default_log_if, cb)
#define set_default_logif_flush_cb(cb)       set_logif_flush_cb(default_log_if, cb)
#define set_default_logif_level_cb(level,cb) (default_log_if->level_cb[level##_INDEX] = (cb))

#define log_inner(fmt, arg...) logu_inner(default_log_if, fmt, ##arg)
#define log_trace(fmt, arg...) logu_trace(default_log_if, fmt, ##arg)
#define log_debug(fmt, arg...) logu_debug(default_log_if, fmt, ##arg)
#define log_info(fmt, arg...)  logu_info(default_log_if, fmt, ##arg)
#define log_warn(fmt, arg...)  logu_warn(default_log_if, fmt, ##arg)
#define log_error(fmt, arg...) logu_error(default_log_if, fmt, ##arg)
#define log_fatal(fmt, arg...) logu_fatal(default_log_if, fmt, ##arg)


/* log function */
void _log_append(log_if *logif, uint8_t level_index, const char *fmt, ...);

/* for file log */
int log_file_init(const char* name);


#ifdef __cplusplus
}
#endif

#endif //OHEV_LOG_H