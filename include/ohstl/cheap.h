#ifndef OHSTL_CHEAP_H
#define OHSTL_CHEAP_H

#ifdef __cplusplus
extern "C" {
#endif


#include <ohstl/cvector.h>
#include <ohstl/citerator.h>



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

#define heap_new(tp)                _heap_new(#tp, NULL)
#define heap_new_if(tp, ifc)        _heap_new(#tp, (ifc))
#define heap_init(hp, tp)           _heap_init((hp), #tp, NULL)
#define heap_init_if(hp, tp, ifc)   _heap_init((hp), #tp, (ifc))


heap_t* _heap_new(const char* , _heap_less);
int _heap_init(heap_t*, const char*, _heap_less);

void heap_destroy(heap_t*);
void heap_delete(heap_t*);

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
void _heap_copy(heap_t*, int, int);

#ifdef __cplusplus
}
#endif
#endif  //OHSTL_CHEAP_H

