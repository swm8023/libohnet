#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ohutil/thread.h>

/* === thread function === */
__thread int cr_thread_id = 0;
int thread_id() {
    return cr_thread_id ? cr_thread_id :
        (cr_thread_id = syscall(SYS_gettid));
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
    FOR_SPLST_START(_mtxdbg_rec_head, rec)
        printf("mutex status: %d. (%p)\n", rec->count - 1, rec->block);
    tot++;
    FOR_SPLST_END()
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