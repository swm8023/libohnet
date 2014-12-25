#ifndef FNUTIL_CITERATOR_H
#define FNUTIL_CITERATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fnutil/ctypes.h>
#include <fnutil/util.h>


/* container base class */
struct _tag_iterator_if;

#define _CONTAINER_BASE                 \
    _type_t *_self_type;                \
    _type_t *_contain_type;             \
    struct _tag_iterator_if *_iter_if;

typedef struct _tag_container_base {
    _CONTAINER_BASE;
} _container_base;

#define _CTR_STYPE(cont_p)        ((cont_p)->_self_type)
#define _CTR_CTYPE(cont_p)        ((cont_p)->_contain_type)
#define _CTR_ITERIF(cont_p)       ((cont_p)->_iter_if)
#define _CTR_CTYPE_SIZE(cont_p)   ((cont_p)->_contain_type->size)


/* iterator */
typedef struct _tag_iterator_t {
    union {
        char *_ptr; /* for list and vector */
    }_itpos;
    void *_container;
} iterator_t;

typedef struct _tag_iterator_if {
    /* public interface */
    iterator_t (*_next)(iterator_t);
    iterator_t (*_pre)(iterator_t);
    iterator_t (*_next_n)(iterator_t, int);
    iterator_t (*_pre_n)(iterator_t, int);
    bool_t (*_equal)(iterator_t, iterator_t);
    int (*_distance)(iterator_t, iterator_t);
    void* (*_get_pointer)(iterator_t);
    void (*_get_value)(iterator_t, void*);
    void (*_set_value)(iterator_t, void*);

    /* private interface */
    bool_t (*_is_vaild)(iterator_t);
} _iterator_if;

#define _ITER_TYPE(iter)        (((_container_base*)((iter)._container))->_self_type)
#define _ITER_IF(iter)          (((_container_base*)((iter)._container))->_iter_if)
#define _ITER_VPTR(iter)        (iter._itpos._ptr)
#define _ITER_LPTR(iter)        (iter._itpos._ptr)
#define _ITER_CONTAINER(iter)   (iter._container)

/* public functions */
#define _ITER_INIT(iter) memset(&(iter), 0, sizeof(iter))

iterator_t iter_next(iterator_t);
iterator_t iter_pre(iterator_t);
iterator_t iter_next_n(iterator_t, int);
iterator_t iter_pre_n(iterator_t, int);
iterator_t iter_advance(iterator_t, int);
bool_t iter_equal(iterator_t, iterator_t);
int   iter_distance(iterator_t, iterator_t);
void* iter_get_pointer(iterator_t);
void  iter_get_value(iterator_t, void*);
void  iter_set_value(iterator_t, void*);

/* private functions */
bool_t _iter_is_vaild(iterator_t);
bool_t _iter_is_same(iterator_t, iterator_t);
bool_t _iter_is_contained(iterator_t, void *);


#ifdef __cplusplus
}
#endif
#endif  //FNUTIL_CLITERATOR_H