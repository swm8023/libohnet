#ifndef OHSTL_MEMORY_H
#define OHSTL_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ohutil/util.h>

typedef struct _tag_alloc_if {
    void* (*alloc)(size_t);
    void* (*realloc)(void*, size_t);
    void (*free)(void*);
}_alloc_if;

#define sysalloc(size) ohmalloc(size)
#define sysfree(ptr)   ohfree(ptr)

#define _CMMP_MIN_SIZE_EXP    2
#define _CMMP_MAX_SIZE_EXP   30
#define _CMMP_MAX_BLOCKSIZE  102400000   /* 10M*/


typedef struct _tag_cmemblock_t _cmemblock_t;
typedef struct _tag_cmemunit_t _cmemunit_t;
typedef struct _tag_cmempool_t _cmempool_t;

typedef struct _tag_cmempool_t {
    int unitsize;
    int unitnum;
    int blknum;
    _cmemblock_t *blkhead;
    struct _tag_cmemunit_t *idleunit;
} _cmempool_t;

typedef struct _tag_cmemblock_t {
    int unitnum;
    int idlenum;
    _cmempool_t *pool;
    _cmemblock_t *next;
    char data[0];
} _cmemblock_t;

typedef struct _tag_cmemunit_t {
    _cmemblock_t *block;
    struct _tag_cmemunit_t *nextunit;
    char data[0];
} _cmemunit_t;

extern _alloc_if *_default_cmem_if;

int get_memory_leak_num();

#ifdef __cplusplus
}
#endif
#endif  //OHSTL_MEMORY_H

