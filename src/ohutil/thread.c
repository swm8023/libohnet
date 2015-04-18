#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ohutil/thread.h>
#include <ohutil/container.h>

/* === thread function === */
__thread int cr_thread_tid = 0;
int thread_tid() {
    return cr_thread_tid ? cr_thread_tid :
        (cr_thread_tid = syscall(SYS_gettid));
}

static mutex_t _oh_mutex_init(void *);
static int _oh_mutex_lock(mutex_t);
static int _oh_mutex_unlock(mutex_t);
static void _oh_mutex_destroy(mutex_t);
static mutex_t _oh_mutexd_init(void *);
static int _oh_mutexd_lock(mutex_t);
static int _oh_mutexd_unlock(mutex_t);
static void _oh_mutexd_destroy(mutex_t);

mutex_if _default_mutex_if = {
#ifdef DEBUG
    "oh_mutex_debug_interface",
    _oh_mutexd_init,
    _oh_mutexd_destroy,
    _oh_mutexd_lock,
    _oh_mutexd_unlock,
#else
    "oh_mutex_release_interface",
    _oh_mutex_init,
    _oh_mutex_destroy,
    _oh_mutex_lock,
    _oh_mutex_unlock,
#endif
};

#
mutex_if  *global_mutex_if  = &_default_mutex_if;

mutex_t _oh_mutex_init(void *attr) {
    pthread_mutex_t *mutex = (pthread_mutex_t*)ohmalloc(sizeof(pthread_mutex_t));
    if (pthread_mutex_init(mutex, (pthread_mutexattr_t*)attr)) {
        ohfree(mutex);
        return NULL;
    }
    return (mutex_t)mutex;
}

int _oh_mutex_lock(mutex_t mutex) {
    return pthread_mutex_lock((pthread_mutex_t*)mutex);
}

int _oh_mutex_unlock(mutex_t mutex) {
    return pthread_mutex_unlock((pthread_mutex_t*)mutex);
}

void _oh_mutex_destroy(mutex_t mutex) {
    pthread_mutex_destroy((pthread_mutex_t*)mutex);
    ohfree((pthread_mutex_t*)mutex);
}

typedef struct _tag_mtxdbg_rec {
    SPLST_DEFNEXT(struct _tag_mtxdbg_rec);
    int count;
    char *block;
}_mtxdbg_rec;
_mtxdbg_rec *_mtxdbg_rec_head = NULL;

mutex_t _oh_mutexd_init(void *attr) {
    /* allocate memory => |mutex|_mtxdbg_rec| */
    int realsize = sizeof(pthread_mutex_t) + sizeof(_mtxdbg_rec);
    char *block = (char*)ohmalloc(realsize);
    if (block == NULL) {
        return NULL;
    }

    pthread_mutex_t *mtx = (pthread_mutex_t*)block;
    _mtxdbg_rec *rec = (_mtxdbg_rec*)(block + sizeof(pthread_mutex_t));

    /* init mutex and _mtxdbg_rec */
    if (pthread_mutex_init(mtx, (pthread_mutexattr_t*)attr)) {
        ohfree(block);
        return NULL;
    }
    rec->count = 1;
    rec->block = block;
    splst_add(_mtxdbg_rec_head, rec);

    return mtx;
}

int _oh_mutexd_lock(mutex_t mutex) {
    pthread_mutex_t *mtx = (pthread_mutex_t*)mutex;
    _mtxdbg_rec *rec = (_mtxdbg_rec*)((char*)mutex + sizeof(pthread_mutex_t));

    rec->count++;

    return pthread_mutex_lock(mtx);
}

int _oh_mutexd_unlock(mutex_t mutex) {
    pthread_mutex_t *mtx = (pthread_mutex_t*)mutex;
    _mtxdbg_rec *rec = (_mtxdbg_rec*)((char*)mutex + sizeof(pthread_mutex_t));

    rec->count--;

    return pthread_mutex_unlock(mtx);
}

void _oh_mutexd_destroy(mutex_t mutex) {
    pthread_mutex_t *mtx = (pthread_mutex_t*)mutex;
    _mtxdbg_rec *rec = (_mtxdbg_rec*)((char*)mutex + sizeof(pthread_mutex_t));

    splst_del(_mtxdbg_rec_head, rec);
    pthread_mutex_destroy(mtx);

    ohfree(rec->block);
}


void oh_mtxdbg_print_rec() {
    _mtxdbg_rec *rec = NULL;
    int tot = 0;
    for_splst_each(_mtxdbg_rec_head, rec) {
        printf("mutex status: %d. (%p)\n", rec->count - 1, rec->block);
    }
    tot++;
    printf("total %d lock allocated.\n", tot);
}



static cond_t _oh_cond_init(void* attr);
static void _oh_cond_destroy(cond_t);
static int _oh_cond_signal(cond_t, int);
static int _oh_cond_wait(cond_t, ohtime_t);

cond_if _default_cond_if =  {
    _oh_cond_init,
    _oh_cond_destroy,
    _oh_cond_signal,
    _oh_cond_wait,
};
cond_if *using_cond_if = &_default_cond_if;

cond_t _oh_cond_init(void* attr) {
    cond_t cm = (cond_t)ohmalloc(sizeof(*cm));
    memset(cm, 0, sizeof(*cm));
    cm->mutex = mutex_init(NULL);
    cm->cond = (pthread_cond_t*)ohmalloc(sizeof(pthread_cond_t));
    if (cm->mutex && cm->cond && !pthread_cond_init(
        cm->cond, (pthread_condattr_t*)attr)) {
        return cm;
    }
    if (cm->mutex) mutex_destroy(cm->mutex);
    if (cm->cond) ohfree(cm->cond);
    return NULL;
}

void _oh_cond_destroy(cond_t cm) {
    mutex_destroy(cm->mutex);
    pthread_cond_destroy((pthread_cond_t*)(cm->cond));
    ohfree(cm->cond);
    ohfree(cm);
}

int _oh_cond_signal(cond_t cm, int type) {
    pthread_cond_t *cond = (pthread_cond_t*)cm->cond;
    if (type == 0) {
        return pthread_cond_signal(cond);
    } else{
        return pthread_cond_broadcast(cond);
    }
}

int _oh_cond_wait(cond_t cm, ohtime_t timeout_us) {
    pthread_mutex_t *mutex = (pthread_mutex_t*)cm->mutex;
    pthread_cond_t *cond = (pthread_cond_t*)cm->cond;
    if (timeout_us > 0) {
        struct timespec timeout;
        timeout_us += get_time();
        ohtime_to_timespec(timeout_us, &timeout);
        return pthread_cond_timedwait(cond, mutex, &timeout);
    }
    return pthread_cond_wait(cond, mutex);

}

#define THREAD_POOL_STATUS_STOP    0x00
#define THREAD_POOL_STATUS_RUNNING 0x01




thread_pool* thread_pool_create(int size, int threads) {
    thread_pool* pool = (thread_pool*)ohmalloc(sizeof(thread_pool));
    if (pool == NULL) {
        goto thread_pool_create_failed;
    }
    memset(pool, 0, sizeof(thread_pool));
    pool->q = (thread_pool_elem*)ohmalloc(sizeof(thread_pool_elem) * size);
    if (pool->q == NULL) {
        goto thread_pool_create_failed;
    }
    pool->size = size;
    pool->rind = 0;
    pool->wind = 0;
    pool->used = 0;
    pool->status = THREAD_POOL_STATUS_STOP;
    pool->running = 0;
    pool->threads = threads;
    pool->lock = mutex_init(NULL);
    pool->cond = cond_init(NULL);

    return pool;

thread_pool_create_failed:
    if (pool) {
        if (pool->q) {
            ohfree(pool->q);
        }
        ohfree(pool);
    }
    return NULL;
}

void *thread_pool_func(void *arg) {
    thread_pool *pool = (thread_pool*)arg;

    mutex_lock(pool->lock);
    pool->running++;
    mutex_unlock(pool->lock);
    thread_pool_elem elem;
    while (pool->status == THREAD_POOL_STATUS_RUNNING) {
        /* get an element */
        cond_lock(pool->cond);
        while (pool->status == THREAD_POOL_STATUS_RUNNING && pool->used == 0) {
            cond_wait(pool->cond);
        }
        if (pool->status == THREAD_POOL_STATUS_STOP) {
            cond_unlock(pool->cond);
            break;
        }
        elem = pool->q[pool->rind++];
        if (pool->rind == pool->size) {
            pool->rind = 0;
        }
        pool->used--;
        cond_unlock(pool->cond);

        /* run */

        if (elem.run) {
            elem.run(elem.arg);
        }
        if (elem.clean) {
            elem.clean(elem.arg);
        }
    }

    mutex_lock(pool->lock);
    pool->running--;
    mutex_unlock(pool->lock);
}

void thread_pool_run(thread_pool* pool) {
    int i;
    pool->status = THREAD_POOL_STATUS_RUNNING;
    for (i = 0; i < pool->threads; i++) {
        thread_t th;
        thread_start(th, thread_pool_func, (void*)pool);
    }
}

int thread_pool_push(thread_pool *pool, tp_func run, tp_func clean, void* arg) {
    cond_lock(pool->cond);
    if (pool->used == pool->size) {
        cond_unlock(pool->cond);
        return -1;
    }

    pool->q[pool->wind].run = run;
    pool->q[pool->wind].clean = clean;
    pool->q[pool->wind].arg = arg;
    pool->wind ++;
    pool->used ++;
    if (pool->wind == pool->size) {
        pool->wind = 0;
    }
    cond_unlock(pool->cond);
    cond_signal(pool->cond);
    return 0;
}

void thread_pool_destroy(thread_pool *pool) {
    pool->status = THREAD_POOL_STATUS_STOP;
    while (pool->running > 0) {
        cond_broadcast(pool->cond);
    }
    cond_destroy(pool->cond);
    mutex_destroy(pool->lock);
    ohfree(pool->q);
    ohfree(pool);
}
