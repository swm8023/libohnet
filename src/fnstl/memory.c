#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <fnstl/memory.h>


/* mempools */
 __thread _cmempool_t *_mempools = NULL;

static int _append_memblock(_cmempool_t*);
static _cmempool_t * _create_mempools();
static void* _cmemory_alloc(size_t);
static void* _cmemory_realloc(void*, size_t);
static void _cmemory_free(void*);

static _alloc_if _cmem_if = {
    _cmemory_alloc,
    _cmemory_realloc,
    _cmemory_free,
};
static _alloc_if _sysmem_if = {
    malloc,
    realloc,
    free,
};
_alloc_if *_default_cmem_if = &_sysmem_if;

void* _cmemory_alloc(size_t size) {
    /* if _mempools not init, alloc it */
    if (_mempools == NULL) {
        _mempools = _create_mempools();
    }

    /* find which pool should use */
    int ind = 0;
    while (size > (1 << ind)) ind++;
    RETURN_IF(ind > _CMMP_MAX_SIZE_EXP, NULL);
    ind = (ind >= _CMMP_MIN_SIZE_EXP ? ind - _CMMP_MIN_SIZE_EXP : 0);

    _cmempool_t *upool = &_mempools[ind];

    /* alloc new block if no space in this pool */
    if (upool->idleunit == NULL) {
        _append_memblock(upool);
    }

    /* get free unit */
    _cmemunit_t *uunit   = upool->idleunit;
    _cmemblock_t *ublock = uunit->block;
    upool->idleunit = uunit->nextunit;
    ublock->idlenum--;

   return (void*)(uunit->data);
}

void *_cmemory_realloc(void *ptr, size_t size) {
    printf("%d\n", size);
    _cmemunit_t *uunit = container_of(ptr, _cmemunit_t, data);
    _cmemblock_t *ublock = uunit->block;
    _cmempool_t *upool = ublock->pool;

    if (size <= upool->unitsize) {
        return ptr;
    }
    void *newptr = _cmemory_alloc(size);
    assert(newptr != NULL);
    memcpy((char*)newptr, (char*)ptr, upool->unitsize);
    _cmemory_free(ptr);

    return newptr;
}

void _cmemory_free(void *ptr) {
    _cmemunit_t *uunit = container_of(ptr, _cmemunit_t, data);
    _cmemblock_t *ublock = uunit->block;
    _cmempool_t *upool = ublock->pool;

    uunit->nextunit = upool->idleunit;
    upool->idleunit = uunit;
    ublock->idlenum++;

}


_cmempool_t* _create_mempools() {
    _cmempool_t *pools;

    int pools_num = _CMMP_MAX_SIZE_EXP - _CMMP_MIN_SIZE_EXP + 1;
    pools = (_cmempool_t*)sysalloc(pools_num * sizeof(_cmempool_t));

    size_t size_exp;
    /* init different size pool*/
    for (size_exp = _CMMP_MIN_SIZE_EXP; size_exp <= _CMMP_MAX_SIZE_EXP; size_exp ++) {
        _cmempool_t *pool = &pools[size_exp - _CMMP_MIN_SIZE_EXP];
        pool->unitsize = (1UL << size_exp);
        /* cal how many unit in a block, at least 1 */
        pool->unitnum  = (_CMMP_MAX_BLOCKSIZE - sizeof(_cmemblock_t)) / (pool->unitsize + sizeof(_cmemunit_t));
        if (pool->unitnum == 0) {
            pool->unitnum = 1;
        }
        pool->blknum   = 0;
        pool->blkhead  = NULL;
        pool->idleunit = NULL;
    }
    return pools;
}

int _append_memblock(_cmempool_t *pool) {
    assert(pool != NULL);

    /* alloc block */
    int ind;
    int realsize = sizeof(_cmemblock_t) + pool->unitnum * (pool->unitsize + sizeof(_cmemunit_t));
    _cmemblock_t* block = (_cmemblock_t*)sysalloc(realsize);
    RETURN_IF_NULL(block, -1);

    /* init block */
    block->unitnum = pool->unitnum;
    block->idlenum = block->unitnum;
    block->pool    = pool;

    /* update pool */
    block->next    = pool->blkhead;
    pool->blkhead  = block;
    pool->blknum++;

    /* init units */
    _cmemunit_t *unit = (_cmemunit_t*)block->data;
    for (ind = 0; ind < block->unitnum; ind++) {
        unit->block = block;
        unit->nextunit = pool->idleunit;
        pool->idleunit = unit;
        unit = (_cmemunit_t*)((char*)unit + sizeof(_cmemunit_t) + pool->unitsize);
    }
}

int get_memory_leak_num() {
    if (_mempools == NULL) {
        return 0;
    }
    int ind = 0, ret = 0;
    for (ind = 0; ind < _CMMP_MAX_SIZE_EXP - _CMMP_MIN_SIZE_EXP; ind++) {
        _cmempool_t *pool = _mempools + ind;
        _cmemblock_t *block = pool->blkhead;
        for (; block != NULL; block = block->next) {
            ret += block->unitnum - block->idlenum;
        }
    }
    return ret;
}