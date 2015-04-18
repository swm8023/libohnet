#include <string.h>
#include <errno.h>

#include <ohev/buffer.h>
#include <ohev/log.h>


ohbuffer* buf_new(int size, ohbuffer_unit_objpool* pool, int lock) {
    ohbuffer *buf;
    buf = (ohbuffer*)ohmalloc(sizeof(ohbuffer));
    buf_init(buf, size, pool, lock);
    return buf;
}

void buf_delete(ohbuffer* buf) {
    buf_destroy(buf);
    ohfree(buf);
}

void buf_init(ohbuffer *buf, int size, ohbuffer_unit_objpool *pool, int uselock) {
    buf->pool      = pool;
    buf->uselock   = uselock;
    buf->unit_size = size;
    buf->unit_cnt  = 0;
    buf->unit_head = NULL;
    buf->unit_tail = NULL;
}

void buf_destroy(ohbuffer *buf) {
    ohbuffer_unit *unit = buf->unit_head;

    /* unit will be free here , so get next unit before free */
    while (unit) {
        ohbuffer_unit *uunit = unit;
        unit = unit->next;

        buf_remove_unit(buf, uunit);
    }
}

int bufunit_read(ohbuffer_unit* bufu, int readsz, char* dst, int dstsz) {
    readsz = min(readsz, dstsz);
    readsz = min(readsz, bufunit_used(bufu));
    if (dst != NULL){
        memcpy(dst, bufunit_rptr(bufu), readsz);
    }
    bufunit_rptr(bufu) += readsz;
    return readsz;
}

int bufunit_peek(ohbuffer_unit* bufu, int readsz, char* dst, int dstsz) {
    readsz = min(readsz, dstsz);
    readsz = min(readsz, bufunit_used(bufu));

    memcpy(dst, bufunit_rptr(bufu), readsz);
    return readsz;
}


int bufunit_write(ohbuffer_unit* bufu, const char* dst, int dstsz) {
    int wrtsz = min(dstsz, bufunit_left(bufu));

    memcpy(bufunit_wptr(bufu), dst, wrtsz);
    bufunit_wptr(bufu) += wrtsz;
    return wrtsz;
}

ohbuffer_unit* buf_get_unit(ohbuffer *buf) {
    ohbuffer_unit *unit = NULL;
    if (buf->pool) {
        unit = bufunit_pool_get(buf->pool, buf->uselock);
    } else {
        unit = (ohbuffer_unit*)ohmalloc(sizeof(ohbuffer_unit) + buf->unit_size);
    }
    /* init it */
    if (unit != NULL) {
        unit->next = NULL;
        unit->size = buf->unit_size;
        bufunit_clear(unit);

        buf->unit_cnt++;
    }
    return unit;
}

void buf_remove_unit(ohbuffer *buf, ohbuffer_unit *unit) {
    if (buf->pool) {
        unit->next = NULL;
        bufunit_pool_free(buf->pool, unit, buf->uselock);
    } else {
        ohfree(unit);
    }
    buf->unit_cnt--;
}

int buf_read(ohbuffer* buf, int readsz, char* dst, int dstsz) {
    readsz = min(readsz, dstsz);

    ohbuffer_unit *bufu = buf->unit_head;
    /* end when no buffer or read enough size*/
    int leftsz = readsz;
    while (bufu && leftsz) {
        /* leftsz <= readsz <= dstsz,  so dstz - readsz + leftsz >= leftsz,
         * it means only leftsz and bufunit_used decide how many bytes data will be read
         */
        leftsz -= bufunit_read(bufu, leftsz, dst ? dst + readsz - leftsz : NULL, dstsz - readsz + leftsz);
        /* read all data of this buffer, means bufunit_used <= leftsz */
        if (bufunit_empty(bufu)) {
            ohbuffer_unit *fbufu = bufu;
            bufu = bufu->next;

            bufunit_clear(fbufu);

            /* has next buffer unit, delete this one, keep at least one buffer unit is enough */
            if (fbufu->next != NULL) {
                /* actually fbufu == unit_head */
                buf->unit_head = fbufu->next;

                buf_remove_unit(buf, fbufu);
            }/* else bufu == NULL, will break next loop */
        }/* else leftsz == 0, will break next loop */
    }

    return readsz - leftsz;
}

int buf_peek(ohbuffer* buf, int readsz, char* dst, int dstsz) {
    readsz = min(readsz, dstsz);

    ohbuffer_unit *bufu = buf->unit_head;
    int leftsz = readsz;
    while (bufu && leftsz) {
        leftsz -= bufunit_peek(bufu, leftsz, dst + readsz - leftsz, dstsz - readsz + leftsz);
        bufu = bufu->next;
    }
    return readsz - leftsz;
}


int buf_readall(ohbuffer* buf, char* dst, int dstsz) {
    return buf_read(buf, dstsz, dst, dstsz);
}


int buf_write(ohbuffer *buf, const char* src, int srcsz) {
    int wrtsz = 0;

    /* first write, allocate one buffer_unit for the buffer*/
    if (buf->unit_head == NULL) {
        buf->unit_head = buf->unit_tail = buf_get_unit(buf);
    }
    while (wrtsz < srcsz) {
        if (bufunit_left(buf->unit_tail) == 0) {
            ohbuffer_unit *unit = buf_get_unit(buf);
            buf->unit_tail->next = unit;
            buf->unit_tail = unit;
        }
        wrtsz += bufunit_write(buf->unit_tail, src + wrtsz, srcsz - wrtsz);
    }

    return wrtsz;
}

int buf_fd_read(ohbuffer *buf, int fd, int *readsz) {
    *readsz = 0;

    /* first write, allocate one buffer_unit for the buffer */
    if (buf->unit_head == NULL) {
        buf->unit_head = buf->unit_tail = buf_get_unit(buf);
    }
    /* no buffer left, get new buffer */
    if (bufunit_left(buf->unit_tail) == 0) {
        ohbuffer_unit *unit = buf_get_unit(buf);
        buf->unit_tail->next = unit;
        buf->unit_tail = unit;
    }
    /* read data to tail unit */
    ohbuffer_unit *bufu = buf->unit_tail;

    while (1) {
        /* no buffer left, get new buffer */
        if (bufunit_left(buf->unit_tail) == 0) {
            ohbuffer_unit *unit = buf_get_unit(buf);
            buf->unit_tail->next = unit;
            buf->unit_tail = unit;
        }
        /* read data to tail unit */
        ohbuffer_unit *bufu = buf->unit_tail;
        int n = 0;
        do {
            n = read(fd, bufunit_wptr(bufu), bufunit_left(bufu));
        } while (n == -1 && errno == EINTR);
        /* read EOF */
        if (n <= 0) {
            return n;
        }
        bufunit_wptr(bufu) += n;
        *readsz += n;
        /* not use all buffer unit, means no data there */
        if (bufunit_left(bufu) > 0) {
            break;
        }
    }
    return *readsz;
}

int buf_fd_write(ohbuffer *buf, int fd, int *wrtsz) {
    *wrtsz = 0;

    ohbuffer_unit *bufu = buf->unit_head;
    /* write all data in buffer */
    while (bufu) {
        int n = 0;
        do {
            n = write(fd, bufunit_rptr(bufu), bufunit_used(bufu));
        } while (n == -1 && errno == EINTR);
        if (n < 0) {
            return n;
        }
        *wrtsz += n;
        bufunit_rptr(bufu) += n;

        /* buffer is empty, free it if not the last one */
        if (bufunit_empty(bufu)) {
            ohbuffer_unit *fbufu = bufu;
            bufu = bufu->next;

            bufunit_clear(fbufu);

            /* has next buffer unit, delete this one, keep at least one buffer unit is enough */
            if (fbufu->next != NULL) {
                /* actually fbufu == unit_head */
                buf->unit_head = fbufu->next;

                buf_remove_unit(buf, fbufu);
            }/* else bufu == NULL, will break next loop */
        }
    }
    return *wrtsz;
}

int buf_used(ohbuffer* buf) {
    int used = 0;
    if (buf->unit_head) {
        used += bufunit_used(buf->unit_head);
    }
    if (buf->unit_cnt > 1) {
        used += bufunit_used(buf->unit_tail);
        used += (buf->unit_cnt - 2) * buf->unit_size;
    }
    return used;
}
