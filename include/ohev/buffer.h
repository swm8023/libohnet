#ifndef OHEV_BUFFER_H
#define OHEV_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <ohutil/util.h>

#define OHBUFFER_OBJPOOL_BLOCKSZ        32
#define OHBUFFER_UNIT_DEFAULT_SIZE      4096

#define OHBUFFER_UNITPOOL_LOCK   1
#define OHBUFFER_UNITPOOL_NOLOCK 0

typedef struct _tag_ohbuffer_unit_objpool ohbuffer_unit_objpool;
/* !! the next pointer !!
 * when in object pool, next point to next free buffer
 * when in connection , next point to next buffer of same connection
 */
typedef struct _tag_ohbuffer_unit {
    OBJPOOL_OBJ_BASE(struct _tag_ohbuffer_unit);

    char *rptr;
    char *wptr;
    int size;
    char data[0];
} ohbuffer_unit;

/* operation on buff unit */
#define bufunit_size(bufu)      ((bufu)->size)
#define bufunit_start(bufu)     ((bufu)->data)
#define bufunit_end(bufu)       ((bufu)->data + bufunit_size(bufu))
#define bufunit_rptr(bufu)      ((bufu)->rptr)
#define bufunit_wptr(bufu)      ((bufu)->wptr)
#define bufunit_empty(bufu)     (bufunit_rptr(bufu) == bufunit_wptr(bufu))
#define bufunit_clear(bufu)     ((bufu)->rptr = (bufu)->wptr = bufunit_start(bufu))
#define bufunit_left(bufu)      (bufunit_end(bufu) - bufunit_wptr(bufu))
#define bufunit_used(bufu)      (bufunit_wptr(bufu) - bufunit_rptr(bufu))
#define bufunit_real_left(bufu) (bufunit_size(bufu) - bufunit_used(bufu))

#define bufunit_set_rptr(bufu, pos) (buffunit_rptr(buf) = (pos))
#define bufunit_set_wptr(bufu, pos) (buffunit_wptr(buf) = (pos))

int bufunit_read(ohbuffer_unit*, int, char*, int);
int bufunit_write(ohbuffer_unit*, const char*, int);



/* ohbuffer is composed of buffer unit */
typedef struct _tag_ohbuffer {
    ohbuffer_unit_objpool *pool;
    int uselock;
    int unit_size;
    int unit_cnt;
    ohbuffer_unit *unit_head;
    ohbuffer_unit *unit_tail;
} ohbuffer;


void buf_remove_unit(ohbuffer*, ohbuffer_unit*);
ohbuffer_unit* buf_get_unit(ohbuffer*);

ohbuffer* buf_new(int, ohbuffer_unit_objpool*, int);
void buf_init(ohbuffer*, int, ohbuffer_unit_objpool*, int);
void buf_destroy(ohbuffer*);
void buf_delete(ohbuffer*);

int buf_read(ohbuffer*, int, char*, int);
int buf_readall(ohbuffer*, char*, int);
int buf_write(ohbuffer*, const char*, int);

int buf_fd_read(ohbuffer*, int);
int buf_fd_write(ohbuffer*, int);

int buf_used(ohbuffer*);

/* buffer pool */
typedef struct _tag_ohbuffer_unit_objpool {
    OBJPOOL_BASE(struct _tag_ohbuffer_unit);
} ohbuffer_unit_objpool;

#define bufunitpool_init(pool, bsz, osz)                 \
    objpool_init((objpool_base*)(pool), (bsz), (osz))
#define bufunitpool_destroy(pool)                        \
    objpool_destroy((objpool_base*)(pool))

#define bufunitpool_get(pool)                            \
    (ohbuffer_unit*)objpool_get_obj((objpool_base*)pool);
#define bufunitpool_free(pool, elem)                     \
    objpool_free_obj((objpool_base*)pool, elem)

#define bufunitpool_get_lock(pool)                       \
    (ohbuffer_unit*)objpool_get_obj_lock((objpool_base*)pool);
#define bufunitpool_free_lock(pool, elem)                \
    objpool_free_obj_lock((objpool_base*)pool, elem)




#ifdef __cplusplus
}
#endif
#endif  //OHEV_BUFFER_H