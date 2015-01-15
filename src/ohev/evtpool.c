#include <string.h>

#include <ohev/evtpool.h>
#include <ohev/log.h>
#include <ohutil/thread.h>


evt_pool* evt_pool_init(int size) {
    return evt_pool_init_flag(size, 0);
}

evt_pool* evt_pool_init_flag(int size, int flag) {
    int i;

    evt_pool *pool = (evt_pool*)ohmalloc(sizeof(evt_pool));
    if (pool == 0) {
        goto evt_pool_init_failed;
    }
    memset(pool, 0, sizeof pool);

    /* status => init*/
    pool->status =_POOL_STATUS_INIT;
    pool->owner_tid = 0;

    /* init loops */
    pool->loops_num = size;
    pool->loops_pid = (thread_t*)ohmalloc(sizeof(thread_t) * size);
    pool->loops = (evt_loop**)ohmalloc(sizeof(evt_loop*) * size);
    if (pool->loops == NULL || pool->loops_pid == NULL) {
        goto evt_pool_init_failed;
    }

    for (i = 0;i < size; i++) {
        pool->loops[i] = evt_loop_init_flag(flag);
        if (pool->loops[i] == NULL) {
            goto evt_pool_init_failed;
        }
    }

    /* default way to load balance */
    pool->lb_next_loop_init = lb_roundrobin_next_loop_init;

    if (pool->lb_next_loop_init) {
        pool->lb_next_loop_init(pool);
    }

    log_inner("event loops pool(%d) init, total(%d).", thread_tid(), size);
    return pool;

evt_pool_init_failed:
    log_error("event loops pool(%d) init failed!", thread_tid());
    if (pool != NULL) {
        evt_pool_destroy(pool);
    }
    return NULL;
}



void evt_pool_destroy(evt_pool* pool) {
    if (pool == NULL) {
        log_warn("destroy a NULL pool");
        return;
    }
    /* if running, stop first */
    if (pool->status & _POOL_STATUS_RUNNING) {
        pool->status |= _POOL_STATUS_WAITDESTROY;
        evt_pool_quit(pool);
        return;
    }

    int i;
    /* all event loops in pool must stopped */
    for (i = 0; i < pool->loops_num; i++) {
        evt_loop_destroy(pool->loops[i]);
    }

    ohfree(pool->loops);
    ohfree(pool->loops_pid);

    if (pool->lb_next_loop_destroy) {
        pool->lb_next_loop_destroy(pool);
    }

    ohfree(pool);

   log_inner("event loops pool(%d) destroy.", thread_tid());
}

void evt_pool_quit(evt_pool* pool) {
    int i;
    pool->status |= _POOL_STATUS_QUITING;
    for (i = 0;i < pool->loops_num; i++) {
        evt_loop_quit(pool->loops[i]);
    }
}

static THREAD_FUNC_START(_thread_func_new_loop, arg) {
    evt_loop_run((evt_loop*)arg);
}
THREAD_FUNC_END

void evt_pool_run(evt_pool* pool) {
    int i;

    pool->owner_tid = thread_tid();
    pool->status = _POOL_STATUS_STARTED | _POOL_STATUS_RUNNING;

    log_inner("event loops pool(%d) running.", thread_tid());
    /* run loop[1~n-1] in new thread and run loop[0] in this thread */
    for (i = 1; i < pool->loops_num; i++) {
        thread_start(pool->loops_pid[i], _thread_func_new_loop, pool->loops[i]);
    }
    pool->loops_pid[0] = thread_pid();
    evt_loop_run(pool->loops[0]);

    /* wait for all loops stop */
    for (i = 1; i < pool->loops_num; i++) {
        thread_join(pool->loops_pid[i]);
    }

    log_inner("event loops pool(%d) quit.", thread_tid());

    /* quited */
    pool->status &= ~_POOL_STATUS_RUNNING;
    pool->status &= ~_POOL_STATUS_QUITING;
    pool->status &= ~_POOL_STATUS_SUSPEND;
    pool->status |= _POOL_STATUS_STOP;

    /* if destroy called */
    if (pool->status & _POOL_STATUS_WAITDESTROY) {
        evt_pool_destroy(pool);
    }
}

/* round-robin way to get next loop int pool */
void lb_roundrobin_next_loop_init(evt_pool* pool) {
    pool->lb_data = ohmalloc(sizeof(int));
    pool->lb_next_loop = lb_roundrobin_next_loop;
    pool->lb_next_loop_destroy = lb_roundrobin_next_loop_destroy;
    *(int*)(pool->lb_data) = pool->loops_num - 1;;
}

void lb_roundrobin_next_loop_destroy(evt_pool* pool) {
    ohfree(pool->lb_data);
}

evt_loop* lb_roundrobin_next_loop(evt_pool* pool) {
    int pre_loop_ind = *(int*)(pool->lb_data);
    *(int*)(pool->lb_data) = (pre_loop_ind + 1) % pool->loops_num;
    return pool->loops[*(int*)(pool->lb_data)];
}