#ifndef OHUTIL_THREAD_H
#define OHUTIL_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <assert.h>

#include <sys/syscall.h>
#include <pthread.h>
#include <ohutil/time.h>
#include <ohutil/memory.h>

/* === thread function === */
/* just the tid,  can't instead of pthread_t*/
int thread_tid();

typedef pthread_t thread_t;
#define THREAD_FUNC_START(fn, val) void* fn(void *(val)) {
#define THREAD_FUNC_END return NULL; }

#define thread_pid() pthread_self()
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
    mutex_t (*init)(void*);
    void (*destroy)(mutex_t);
    int (*lock)(mutex_t);
    int (*unlock)(mutex_t);
}mutex_if;

extern mutex_if  *global_mutex_if;

#define mutex_init(attr) global_mutex_if->init((attr))
#define mutex_destroy(p) global_mutex_if->destroy((p))
#define mutex_lock(p)    global_mutex_if->lock((p))
#define mutex_unlock(p)  global_mutex_if->unlock((p))

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

#ifdef __cplusplus
}
#endif
#endif  //OHEV_THREAD_H