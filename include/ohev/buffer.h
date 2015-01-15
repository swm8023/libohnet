#ifndef OHEV_BUFFER_H
#define OHEV_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <ohutil/util.h>

#define OHBUFFER_OBJPOOL_BLOCKSZ 32
#define OHBUFFER_INIT_SIZE       4096
/* !! the next pointer !!
 * when in object pool, next point to next free buffer
 * when in connection , next point to next buffer of same connection
 */
typedef struct _tag_ohbuffer {
    OBJPOOL_OBJ_BASE(struct _tag_ohbuffer);

    char *start;
    char *end;
    char *rptr;
    char *wptr;
} ohbuffer;

#define buff_size(buf)      ((buf)->end - (buf)->start)
#define buff_start(buf)     ((buf)->start)
#define buff_end(buf)       ((buf)->end)
#define buff_rptr(buf)      ((buf)->rptr)
#define buff_wptr(buf)      ((buf)->wptr)
#define buff_empty(buf)     (buff_rptr(buf) == buff_wptr(buf))
#define buff_clear(buf)     ((buf)->rptr = (buf)->wptr = (buf)->start)
#define buff_left(buf)      (buff_end(buf) - buff_wptr(buf))
#define buff_used(buf)      (buff_wptr(buf) - buff_rptr(buf))
#define buff_real_left(buf) (buff_size(buf) - buff_used(buf))

#define buff_set_rptr(buf, pos) (buff_rptr(buf) = (pos))
#define buff_set_wptr(buf, pos) (buff_wptr(buf) = (pos))

ohbuffer* buff_new();
ohbuffer* buff_new_size(int);
void buff_init(ohbuffer*, int);
void buff_destroy(ohbuffer*);

int buff_read(ohbuffer*, int, char*, int);
int buff_readall(ohbuffer*, char*, int);
int buff_write(ohbuffer*, const char*, int);




/* buffer pool */
typedef struct _tag_ohbuffer_objpool {
    OBJPOOL_BASE(struct _tag_ohbuffer);
} ohbuffer_objpool;

#define ohbufferpool_init(pool, bsz, osz)   \
    objpool_init((objpool_base*)(pool), (bsz), (osz))
#define obhufferpool_get(pool)              \
    (ohbuffer*)objpool_get_obj((objpool_base*)pool)
#define obhufferpool_free(pool, elem)       \
    objpool_free_obj((objpool_base*)pool, elem)





#ifdef __cplusplus
}
#endif
#endif  //OHEV_BUFFER_H