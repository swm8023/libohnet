#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>

#include <ohutil/thread.h>
#include <ohev/log.h>

static void _log_console_output_cb(const char *, size_t);
static void _log_console_flush_cb();
static void _log_console_fatal_cb();

static void _log_file_output_cb(const char *, size_t);
static void _log_file_flush_cb();
static void _log_file_fatal_cb();


static const char* log_level_name[LOG_LEVELS] = {
    "INNER ",
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL "
};


/* console log interface */
static log_if _log_if_console = {
    (uint8_t)(LOG_MASK),
    _log_console_output_cb,
    _log_console_flush_cb,
    {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        _log_console_fatal_cb
    }
};

/* file log interface */
static log_if _log_if_file = {
    (uint8_t)(LOG_MASK),
    _log_file_output_cb,
    _log_file_flush_cb,
    {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        _log_file_fatal_cb
    }
};

log_if *default_log_if = &_log_if_console;


void _log_append(log_if *logif, uint8_t level_index, const char *fmt, ...) {
    char buf[LOG_BUFSIZE + 1], timebuf[30];
    size_t buflen = 0;

    /* here use cached time, because we only use the log system  in event loop,
     * otherwise update_catime should be called before log function.
     */
    get_catime_str(timebuf, sizeof timebuf);
    buflen = snprintf(buf, LOG_BUFSIZE, "%s%5d %s ", log_level_name[level_index],
        thread_tid(), timebuf);

    va_list ap;
    va_start(ap, fmt);
    buflen += vsnprintf(buf + buflen, LOG_BUFSIZE - buflen, fmt, ap);
    va_end(ap);

    if (level_index >= LOG_WARN_INDEX && errno != 0) {
        buflen += snprintf(buf + buflen, LOG_BUFSIZE - buflen, "(%s)", strerror(errno));
    }

    buf[buflen]   = '\n';
    buf[++buflen] = '\0';

    if (logif->output_cb) {
        logif->output_cb(buf, buflen);
    }

    if (logif->level_cb[level_index]) {
        logif->level_cb[level_index](buf, buflen);
    }
}

void _log_console_output_cb(const char *str, size_t size) {
    fwrite(str, size, 1, stdout);
}

void _log_console_flush_cb() {
    fflush(stdout);
}

void _log_console_fatal_cb() {
    printf("!!!fatal error, program exit!!!\n");
    _log_console_flush_cb();
    _exit(-1);
}

#define MAX_FILEPATH_LEN 255
#define FILEINFO_BUFSZ  4096
typedef struct _tag_ohlog_fileinfo {
    char filepath[MAX_FILEPATH_LEN];
    FILE *fp;
    mutex_t mutex;
} _ohlog_fileinfo;
static _ohlog_fileinfo _log_fileinfo;

int log_file_init(const char* filepath) {
    if (strlen(filepath) >= MAX_FILEPATH_LEN) {
        return -1;
    }
    strcpy(_log_fileinfo.filepath, filepath);
    if ((_log_fileinfo.fp = fopen(filepath, "a")) == NULL) {
        printf("....%s\n", strerror(errno));
        return -1;
    }
    default_log_if = &_log_if_file;
    return 0;
}

static void _log_file_output_cb(const char *str, size_t size) {
    fwrite(str, size, 1, _log_fileinfo.fp);
}

static void _log_file_flush_cb() {
    fflush(_log_fileinfo.fp);
}

static void _log_file_fatal_cb() {
    fflush(_log_fileinfo.fp);
    fclose(_log_fileinfo.fp);
    _exit(-1);
}


