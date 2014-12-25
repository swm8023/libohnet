#ifndef FNBASE_UTIL_H
#define FNBASE_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <assert.h>

#ifdef DEBUG
#define DBG_PARAM , __FILE__, __LINE__
#define DBG_PDEF , const char* const file, int line
#define IFIND_USED 1
#else
#define DBG_PARAM
#define DBG_PDEF
#define IFIND_USED 0
#endif

typedef struct _tag_source_location {
    const char* file;
    int line;
}source_location;


/* === normal function === */
#define STRING(x) #x
#define DEFINE_TEST(x) STRING(x)

#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))
int max_int(int x, int y);
int min_int(int x, int y);

typedef size_t bool_t;
#define true   1
#define false  0

#define RETURN_IF(x, arg...)\
    if ((x)) return arg;

#define RETURN_IF_NULL(x, arg...)   \
    if ((x) == NULL) return arg;

#define container_of(ptr, type, member) ({                      \
        (type *)( (char *)(ptr) - offsetof(type,member) );})

/* === time operation === */
typedef int64_t fntime_t;
#define US_ONE_SEC      1000000
#define SECOND(x)       ((x) * 1000000)
#define TIMESTR_LEN     25

void enable_time_cache();
void disable_time_cache();

fntime_t get_time();
fntime_t get_catime();
fntime_t update_catime();

fntime_t get_time_str(char *, int);
fntime_t get_catime_str(char *, int);
fntime_t fnusleep(fntime_t);
void fntime_to_str(fntime_t, char *, int);
void fntime_to_timespec(fntime_t , struct timespec *);
void fntime_to_timeval(fntime_t,struct timeval *);
fntime_t timespec_to_fntime(struct timespec *);
fntime_t timeval_to_fntime(struct timeval *);


/* === simple list function */
#define SPLST_INITVAL NULL
#define SPLST_DEFNEXT(type) type *next;

#define FOR_SPLST_START(head, elm) \
    for ((elm) = (head); (elm); (elm) = (elm)->next) {
#define FOR_SPLST_END() }

#define splst_init(head)  ((head) = NULL)
#define splst_empty(head) ((head) == NULL)

#define splst_add(head, elm) do {   \
    (elm)->next = (head);           \
    (head) = (elm);                 \
} while (0)

#define splst_del(head, elm) do {   \
    typeof(head) *_spe = &head;     \
    for (;*_spe && *_spe != (elm);  \
        _spe = &(*_spe)->next);     \
    if(*_spe) *_spe = elm->next;    \
} while (0)

#define splst_len(head) ({                          \
    int _splen = 0;                                 \
    typeof(head) _spe = (head);                     \
    for (;(_spe); (_spe) = (_spe)->next, _splen++); \
    _splen;                                         \
})


/* === thread function === */
int thread_id();
typedef pthread_t thread_t;
#define THREAD_FUNC_START(fn, val) void* fn(void *(val)) {
#define THREAD_FUNC_END return NULL; }

#define thread_start(pt, fn, arg) pthread_create(&(pt), NULL, fn, arg)
#define thread_join(pt) pthread_join((pt), NULL)
#define thread_detach(pt) pthread_detach(pt)

typedef volatile int8_t atomic8_t;
typedef volatile int16_t atomic16_t;
typedef volatile int32_t atomic32_t;
typedef volatile int64_t atomic64_t;
#define atomic_get(x)           (__sync_val_compare_and_swap(&(x), 0, 0))
#define atomic_get_add(x, t)    (__sync_fetch_and_add(&(x), t))
#define atomic_add_get(x, t)    (__sync_add_and_fetch(&(x), t))
#define atomic_increment(x)     (__sync_add_and_fetch(&(x), 1))
#define atomic_decrement(x)     (__sync_add_and_fetch(&(x), -1))


typedef void* mutex_t;
typedef struct _tag_mutex_if {
    const char* name;
    mutex_t (*init)(void*, ...);
    void (*destroy)(mutex_t, ...);
    int (*lock)(mutex_t, ...);
    int (*unlock)(mutex_t, ...);
}mutex_if;

extern mutex_if  *global_mutex_if;

#define mutex_init(attr) global_mutex_if->init((attr) DBG_PARAM)
#define mutex_destroy(p) global_mutex_if->destroy((p) DBG_PARAM)
#define mutex_lock(p)    global_mutex_if->lock((p) DBG_PARAM)
#define mutex_unlock(p)  global_mutex_if->unlock((p) DBG_PARAM)

void fn_mtxdbg_print_rec();


typedef void* _cond_t;
typedef struct _tag_cond_t {
    _cond_t cond;
    mutex_t mutex;
}*cond_t;

typedef struct _tag_cond_if {
    cond_t (*init)(void*);
    void (*destroy)(cond_t);
    int (*signal)(cond_t, int);
    int (*wait)(cond_t, fntime_t);
}cond_if;

extern cond_if  *using_cond_if;

#define cond_init(attr)      using_cond_if->init(attr)
#define cond_destroy(p)      using_cond_if->destroy(p)
#define cond_signal(p)       using_cond_if->signal((p), 0)
#define cond_broadcast(p)    using_cond_if->signal((p), 1)
#define cond_wait(p)         using_cond_if->wait((p), 0)
#define cond_timedwait(p, t) using_cond_if->wait((p), (t))
#define cond_lock(p)         mutex_lock((p)->mutex)
#define cond_unlock(p)       mutex_unlock((p)->mutex);


/* === memory function === */
typedef struct _tag_mem_if {
    const char* name;
    void* (*alloc)(void*, size_t, int, ...);
    void (*free)(void*, ...);
}mem_if;

extern mem_if *global_mem_if;

#define fn_malloc(size)      global_mem_if->alloc(NULL,(size),0 DBG_PARAM)
#define fn_realloc(p, size)  global_mem_if->alloc((p),(size),0 DBG_PARAM)
#define fn_calloc(num, size) global_mem_if->alloc(NULL,(num)*(size),1 DBG_PARAM)
#define fn_free(p)           global_mem_if->free((p) DBG_PARAM)

#define MEM_ALIGNSIZE sizeof(size_t)

int  fn_memdbg_get_recnum();
void fn_memdbg_print_rec();


#ifdef __cplusplus
}
#endif
#endif  //FNBASE_UTIL_H