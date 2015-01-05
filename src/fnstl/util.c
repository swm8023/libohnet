#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#include <sys/select.h>
#include <fnstl/util.h>


/* === normal function === */
int max_int(int x, int y) {
    return x > y ? x : y;
}

int min_int(int x, int y) {
    return x < y ? x : y;
}


#define ARGLIST_TO_SL(p_vp, p_sl) do {      \
    va_list arg_ptr;                        \
    va_start(arg_ptr, (p_vp));              \
    (p_sl).file = va_arg(arg_ptr, char*);   \
    (p_sl).line = va_arg(arg_ptr, int);     \
    va_end(arg_ptr);                        \
} while (0);

/* === simple list function */



/* === thread function === */
__thread int cr_thread_id = 0;
int thread_id() {
    return cr_thread_id ? cr_thread_id :
        (cr_thread_id = syscall(SYS_gettid));
}

static mutex_t _fn_mutex_init(void *, ...);
static int _fn_mutex_lock(mutex_t, ...);
static int _fn_mutex_unlock(mutex_t, ...);
static void _fn_mutex_destroy(mutex_t, ...);
static mutex_t _fn_mutexd_init(void *, ...);
static int _fn_mutexd_lock(mutex_t, ...);
static int _fn_mutexd_unlock(mutex_t, ...);
static void _fn_mutexd_destroy(mutex_t, ...);

mutex_if _default_mutex_if[] = {{
    "fn_mutex_release_interface",
    _fn_mutex_init,
    _fn_mutex_destroy,
    _fn_mutex_lock,
    _fn_mutex_unlock,
}, {
    "fn_mutex_debug_interface",
    _fn_mutexd_init,
    _fn_mutexd_destroy,
    _fn_mutexd_lock,
    _fn_mutexd_unlock,
}};

mutex_if  *global_mutex_if  = &_default_mutex_if[IFIND_USED];

mutex_t _fn_mutex_init(void *attr, ...) {
    pthread_mutex_t *mutex = (pthread_mutex_t*)fn_malloc(sizeof(pthread_mutex_t));
    if (pthread_mutex_init(mutex, (pthread_mutexattr_t*)attr)) {
        fn_free(mutex);
        return NULL;
    }
    return (mutex_t)mutex;
}

int _fn_mutex_lock(mutex_t mutex, ...) {
    return pthread_mutex_lock((pthread_mutex_t*)mutex);
}

int _fn_mutex_unlock(mutex_t mutex, ...) {
    return pthread_mutex_unlock((pthread_mutex_t*)mutex);
}

void _fn_mutex_destroy(mutex_t mutex, ...) {
    pthread_mutex_destroy((pthread_mutex_t*)mutex);
    fn_free((pthread_mutex_t*)mutex);
}

typedef struct _tag_mtxdbg_rec {
    SPLST_DEFNEXT(struct _tag_mtxdbg_rec);
    source_location sl;
    int count;
    char *block;
}mtxdbg_rec;
mtxdbg_rec *mtxdbg_rec_head = NULL;

mutex_t _fn_mutexd_init(void *attr, ...) {
    source_location sl;
    ARGLIST_TO_SL(attr, sl);

    /* allocate memory => |mutex|mtxdbg_rec| */
    int realsize = sizeof(pthread_mutex_t) + sizeof(mtxdbg_rec);
    char *block = (char*)fn_malloc(realsize);
    if (block == NULL) {
        return NULL;
    }

    pthread_mutex_t *mtx = (pthread_mutex_t*)block;
    mtxdbg_rec *rec = (mtxdbg_rec*)(block + sizeof(pthread_mutex_t));

    /* init mutex and mtxdbg_rec */
    if (pthread_mutex_init(mtx, (pthread_mutexattr_t*)attr)) {
        fn_free(block);
        return NULL;
    }
    rec->sl    = sl;
    rec->count = 1;
    rec->block = block;
    splst_add(mtxdbg_rec_head, rec);

    return mtx;
}

int _fn_mutexd_lock(mutex_t mutex, ...) {
    source_location sl;
    ARGLIST_TO_SL(mutex, sl);

    pthread_mutex_t *mtx = (pthread_mutex_t*)mutex;
    mtxdbg_rec *rec = (mtxdbg_rec*)((char*)mutex + sizeof(pthread_mutex_t));

    rec->sl = sl;
    rec->count++;

    return pthread_mutex_lock(mtx);
}

int _fn_mutexd_unlock(mutex_t mutex, ...) {
    source_location sl;
    ARGLIST_TO_SL(mutex, sl);

    pthread_mutex_t *mtx = (pthread_mutex_t*)mutex;
    mtxdbg_rec *rec = (mtxdbg_rec*)((char*)mutex + sizeof(pthread_mutex_t));

    rec->sl = sl;
    rec->count--;

    return pthread_mutex_unlock(mtx);
}

void _fn_mutexd_destroy(mutex_t mutex, ...) {
    pthread_mutex_t *mtx = (pthread_mutex_t*)mutex;
    mtxdbg_rec *rec = (mtxdbg_rec*)((char*)mutex + sizeof(pthread_mutex_t));

    splst_del(mtxdbg_rec_head, rec);
    pthread_mutex_destroy(mtx);

    fn_free(rec->block);
}


void fn_mtxdbg_print_rec() {
    mtxdbg_rec *rec = NULL;
    int tot = 0;
    FOR_SPLST_START(mtxdbg_rec_head, rec)
        printf("mutex status: %d. (%s:%d)\n", rec->count - 1,
            rec->sl.file, rec->sl.line);
    tot++;
    FOR_SPLST_END()
    printf("total %d lock allocated.\n", tot);
}



static cond_t _fn_cond_init(void* attr);
static void _fn_cond_destroy(cond_t);
static int _fn_cond_signal(cond_t, int);
static int _fn_cond_wait(cond_t, fntime_t);

cond_if _default_cond_if =  {
    _fn_cond_init,
    _fn_cond_destroy,
    _fn_cond_signal,
    _fn_cond_wait,
};
cond_if *using_cond_if = &_default_cond_if;

cond_t _fn_cond_init(void* attr) {
    cond_t cm = (cond_t)fn_malloc(sizeof(*cm));
    memset(cm, 0, sizeof(*cm));
    cm->mutex = mutex_init(NULL);
    cm->cond = (pthread_cond_t*)fn_malloc(sizeof(pthread_cond_t));
    if (cm->mutex && cm->cond && !pthread_cond_init(
        cm->cond, (pthread_condattr_t*)attr)) {
        return cm;
    }
    if (cm->mutex) mutex_destroy(cm->mutex);
    if (cm->cond) fn_free(cm->cond);
    return NULL;
}

void _fn_cond_destroy(cond_t cm) {
    mutex_destroy(cm->mutex);
    pthread_cond_destroy((pthread_cond_t*)(cm->cond));
    fn_free(cm->cond);
    fn_free(cm);
}

int _fn_cond_signal(cond_t cm, int type) {
    pthread_cond_t *cond = (pthread_cond_t*)cm->cond;
    if (type == 0) {
        return pthread_cond_signal(cond);
    } else{
        return pthread_cond_broadcast(cond);
    }
}

int _fn_cond_wait(cond_t cm, fntime_t timeout_us) {
    pthread_mutex_t *mutex = (pthread_mutex_t*)cm->mutex;
    pthread_cond_t *cond = (pthread_cond_t*)cm->cond;
    if (timeout_us > 0) {
        struct timespec timeout;
        timeout_us += get_time();
        fntime_to_timespec(timeout_us, &timeout);
        return pthread_cond_timedwait(cond, mutex, &timeout);
    }
    return pthread_cond_wait(cond, mutex);

}


/* === memory function === */
static void* _fn_mem_alloc(void*, size_t, int, ...);
static void _fn_mem_free(void*, ...);
static void* _fn_memd_alloc(void*, size_t, int, ...);
static void _fn_memd_free(void*, ...);
static mem_if _default_mem_if[] = {{
    "fn_mem_realese_interface",
    _fn_mem_alloc,
    _fn_mem_free,
}, {
    "fn_mem_debug_interface",
    _fn_memd_alloc,
    _fn_memd_free,
}};

mem_if *global_mem_if  = &_default_mem_if[IFIND_USED];

typedef struct _tag_memdbg_rec {
    SPLST_DEFNEXT(struct _tag_memdbg_rec);
    source_location sl;
    char *block;
    char *ptr;
    size_t size;
}memdbg_rec;
static memdbg_rec *memdbg_rec_head = NULL;

void *_fn_mem_alloc(void *ptr, size_t size, int init, ...) {
    return ptr ? realloc(ptr, size) :
            (init ? calloc(1, size) : malloc(size));
}

void _fn_mem_free(void *ptr, ...) {
    free(ptr);
}

void *_fn_memd_alloc(void *ptr, size_t size , int init, ...) {
    source_location sl;
    ARGLIST_TO_SL(init, sl);

    /* allocate memory => |pack|memdbg_rec|using memory|pack| */
    int realsize = size + sizeof(memdbg_rec) + MEM_ALIGNSIZE;
    char *block = (char*) (init ? calloc(1, realsize) : malloc(realsize));
    if (block == NULL) {
        return NULL;
    }

    /* let using memory align to ALIGNMENT */
    char *retptr = (char*)(((size_t)block + MEM_ALIGNSIZE +
        sizeof(memdbg_rec)) & ~(MEM_ALIGNSIZE - 1));
    memdbg_rec *rec = (memdbg_rec*) ((char*)retptr - sizeof(memdbg_rec));

    /* copy to new memory and free old memory if realloc */
    if (ptr != NULL) {
        memdbg_rec* old_rec = (memdbg_rec*)((char*)ptr - sizeof(memdbg_rec));
        memcpy(retptr, old_rec->ptr, old_rec->size);
        _fn_memd_free(ptr, sl);
    }

    /* init memdbg_rec */
    rec->sl    = sl;
    rec->block = block;
    rec->ptr   = retptr;
    rec->size  = size;
    splst_add(memdbg_rec_head, rec);
    return retptr;
}

void _fn_memd_free(void *ptr, ...) {
    memdbg_rec* old_rec = (memdbg_rec*)((char*)ptr - sizeof(memdbg_rec));
    splst_del(memdbg_rec_head, old_rec);
    free(old_rec->block);
}

int fn_memdbg_get_recnum() {
    return splst_len(memdbg_rec_head);
}

void fn_memdbg_print_rec() {
    memdbg_rec *rec = NULL;
    int tot = 0;
    FOR_SPLST_START(memdbg_rec_head, rec)
        printf("memory leak at %p, size %zu. (%s:%d)\n", rec->ptr,
            rec->size, rec->sl.file, rec->sl.line);
    tot++;
    FOR_SPLST_END()
    printf("totol %d memory leak.\n", tot);
}

/* === time operation === */
static int _is_time_cache = 1;
static __thread fntime_t _cached_time = 0;

void enable_time_cache() {
    _is_time_cache = 1;
}

void disable_time_cache() {
    _is_time_cache = 0;
}

fntime_t get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * US_ONE_SEC + tv.tv_usec;
}

fntime_t get_catime() {
    return (_cached_time && _is_time_cache) ?
        _cached_time : (_cached_time = get_time());
}

fntime_t update_catime() {
    return _cached_time = get_time();
}

fntime_t get_time_str(char *buf, int len) {
    if (len < TIMESTR_LEN) {
        return -1;
    }
    fntime_t us_now = get_time();
    fntime_to_str(us_now, buf, len);
    return us_now;
}

fntime_t get_catime_str(char *buf, int len) {
    if (len < TIMESTR_LEN) {
        return -1;
    }
    fntime_t us_canow = get_catime();
    fntime_to_str(us_canow, buf, len);
    return us_canow;
}

void fntime_to_str(fntime_t us_now, char *buf, int len) {
    struct tm tm_time;
    time_t sec_now = us_now / US_ONE_SEC;
    localtime_r(&sec_now, &tm_time);
    snprintf(buf, len, "%4d%02d%02d %02d:%02d:%02d.%06d",
      tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
      tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
      (int)(us_now % US_ONE_SEC));
}

void fntime_to_timespec(fntime_t fntime, struct timespec *spectime) {
    spectime->tv_sec  = fntime / US_ONE_SEC;
    spectime->tv_nsec = fntime % US_ONE_SEC * 1000;
}

void fntime_to_timeval(fntime_t fntime, struct timeval *valtime) {
    valtime->tv_sec  = fntime / US_ONE_SEC;
    valtime->tv_usec = fntime %US_ONE_SEC;
}

fntime_t timespec_to_fntime(struct timespec *spectime) {
    return spectime->tv_sec + spectime->tv_nsec / 1000;
}

fntime_t timeval_to_fntime(struct timeval *valtime) {
    return valtime->tv_sec + valtime->tv_usec;
}


fntime_t fnusleep(fntime_t sleepus) {
    struct timeval tv;
    fntime_t sleepus_real;
    int ret;

    fntime_to_timeval(sleepus, &tv);

    sleepus_real = get_time();
    if (0 == (ret = select(0, NULL, NULL, NULL, &tv))) {
        return 0;
    }

    sleepus_real = get_time() - sleepus_real;
    return sleepus_real >= sleepus ? 0: sleepus - sleepus_real;
}