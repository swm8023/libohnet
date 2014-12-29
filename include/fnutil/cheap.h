#ifndef FNUTIL_CHEAP_H
#define FNUTIL_CHEAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fnutil/util.h>
#include <fnutil/ctypes.h>
#include <fnutil/cvector.h>
#include <fnutil/citerator.h>


typedef bool_t (*_heap_less)(const void*, const void*);
typedef struct _tag_heap_t {
    vector_t container;
    _heap_less less;
} heap_t;

#define _HEAP_CVEC(hp)  (&(hp)->container)
#define _HEAP_CTYPE(hp)      _CTR_CTYPE(_HEAP_CVEC(hp))
#define _HEAP_CTYPE_SIZE(hp) _TYPE_SIZE(_HEAP_CTYPE(hp))
#define _HEAP_SIZE(hp)       vector_size(_HEAP_CVEC(hp))
#define _HEAP_LESS(hp)       (hp->less)

#define _HEAP_PARENT(pos)   (((pos) - 1) >> 1)
#define _HEAP_LCHILD(pos)   (((pos) << 1) + 1)
#define _HEAP_RCHILD(pos)   (((pos) << 1) + 2)
#define _HEAP_AT(hp, pos)   vector_at(_HEAP_CVEC(hp), pos)

#define new_heap(tp)                _new_heap(#tp, NULL)
#define new_heap_if(tp, ifc)        _new_heap(#tp, (ifc))
#define init_heap(hp, tp)           _init_heap((hp), #tp, NULL)
#define init_heap_if(hp, tp, ifc)   _init_heap((hp), #tp, (ifc))


heap_t* _new_heap(const char* , _heap_less);
int _init_heap(heap_t*, const char*, _heap_less);

void destroy_heap(heap_t*);
void delete_heap(heap_t*);

#define heap_empty(hp) (_HEAP_SIZE(hp) == 0)
#define heap_size(hp)  _HEAP_SIZE(hp)

void* heap_top(heap_t*);
void heap_top_val(heap_t*, void*);
void heap_push(heap_t*, ...);
void heap_pop(heap_t*);

// void heap_sort(heap_t*);

void _heap_fix_up(heap_t*, int);
void _heap_fix_down(heap_t*, int);
void _heap_swap(heap_t*, int, int);

#ifdef __cplusplus
}
#endif
#endif  //FNUTIL_CHEAP_H

