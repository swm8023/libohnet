#include <ohutil/objpool.h>
#include <ohutil/memory.h>


static objpool_obj_base *objpool_get_obj_nolock(objpool_base*);
static void objpool_free_obj_nolock(objpool_base*, void*);

void objpool_init(objpool_base *opool, int blksize, int objsize) {
    opool->free_elem = NULL;
    opool->objsize = objsize;
    opool->blksize = blksize;
    opool->blkcnt  = 0;
    opool->blks    = NULL;
    opool->mutex   = mutex_init(NULL);
}

void objpool_destroy(objpool_base *opool) {
    /* free blocks */
    objpool_blk *blk = opool->blks;
    mutex_destroy(opool->mutex);
    while (blk) {
        /* block will be destroy, so get next block ahead of destroy it*/
        objpool_blk *ublk = blk;
        blk = blk->next;

        ohfree(ublk->data);
        ohfree(ublk);
    }
}

objpool_obj_base *objpool_get_obj_nolock(objpool_base* opool) {
    int i;
    /* no free element, create a new obj block */
    if (opool->free_elem == NULL) {
        objpool_blk *blk = (objpool_blk*)ohmalloc(sizeof(objpool_blk));
        if (blk == NULL) {
            return NULL;
        }

        blk->data = ohmalloc(opool->objsize * opool->blksize);
        if (blk->data == NULL) {
            free(blk);
            return NULL;
        }

        opool->blkcnt++;
        splst_add(opool->blks, blk);

        /* add all elements to free elment list  */
        objpool_obj_base *obj = blk->data;
        for (i = 0; i < opool->blksize; i++) {
            splst_add(opool->free_elem, obj);
            obj = (objpool_obj_base*)((char*)obj + opool->objsize);
        }
    }

    /* remove from free list and append to busy list */
    objpool_obj_base *robj = opool->free_elem;
    splst_del(opool->free_elem, robj);
    robj->next = NULL;

    return robj;
}

void objpool_free_obj_nolock(objpool_base* opool, void *elem) {
    splst_add(opool->free_elem, (objpool_obj_base*)elem);
}

objpool_obj_base *objpool_get_obj(objpool_base* opool, int lock) {
    if (lock == OBJPOOL_LOCK) {
        mutex_lock(opool->mutex);
        objpool_obj_base *robj = objpool_get_obj_nolock(opool);
        mutex_unlock(opool->mutex);
        return robj;
    } else {
        return objpool_get_obj_nolock(opool);
    }
}

void objpool_free_obj(objpool_base* opool, void *elem, int lock) {
    if (lock == OBJPOOL_LOCK) {
        mutex_lock(opool->mutex);
        objpool_free_obj_nolock(opool, elem);
        mutex_unlock(opool->mutex);
    } else {
        objpool_free_obj_nolock(opool, elem);
    }
}

