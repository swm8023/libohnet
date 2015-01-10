#ifndef OHSTL_CITERATOR_H
#define OHSTL_CITERATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ohstl/ctypes.h>
#include <ohstl/callocate.h>

#include <ohutil/util.h>


/* container base class */
struct _tag_iterator_if;

#define _CONTAINER_BASE                \
    type_node* tpnode;                 \
    struct _tag_iterator_if *iter_if;  \
    _alloc_if *allocif;


typedef struct _tag_container_base {
    _CONTAINER_BASE;
    char data[0];
} _container_base;

/* marcos to access main attributes of container
 * _CTR_SXXX attributes of the container
 * _CTR_CXXX attributes of the only type which container contain
 * _CTR_LXXX attributes of first type which container contain
 * _CTR_RXXX attributes of second type which container contain
 */
#define _CTR_STNODE(cont_p)     ((cont_p)->tpnode)
#define _CTR_CTNODE(cont_p)     _TPNODE_LCNODE(_CTR_STNODE((cont_p)))
#define _CTR_LTNODE(cont_p)     _TPNODE_LCNODE(_CTR_STNODE((cont_p)))
#define _CTR_RTNODE(cont_p)     _TPNODE_RCNODE(_CTR_STNODE((cont_p)))
#define _CTR_STYPE(cont_p)      _TPNODE_TYPE(_CTR_STNODE((cont_p)))
#define _CTR_CTYPE(cont_p)      _TPNODE_TYPE(_CTR_CTNODE((cont_p)))
#define _CTR_LTYPE(cont_p)      _TPNODE_TYPE(_CTR_LTNODE((cont_p)))
#define _CTR_RTYPE(cont_p)      _TPNODE_TYPE(_CTR_RTNODE((cont_p)))
#define _CTR_STYPE_SIZE(cont_p) (_CTR_STYPE((cont_p))->size)
#define _CTR_CTYPE_SIZE(cont_p) (_CTR_CTYPE((cont_p))->size)
#define _CTR_LTYPE_SIZE(cont_p) (_CTR_LTYPE((cont_p))->size)
#define _CTR_RTYPE_SIZE(cont_p) (_CTR_RTYPE((cont_p))->size)

#define _CTR_ITERIF(cont_p)     ((cont_p)->iter_if)

#define _CTR_MEMIF(cont_p)              ((cont_p)->allocif)
#define _CTR_ALLOC(cont_p, sz)          ((cont_p)->allocif->alloc(sz))
#define _CTR_REALLOC(cont_p, ptr, sz)   ((cont_p)->allocif->realloc(ptr, sz))
#define _CTR_FREE(cont_p, ptr)          ((cont_p)->allocif->free(ptr))

#define _CTR_INIT(cont_p, tp, tpstr, it) do {              \
    _CTR_STNODE((cont_p)) = _get_type_node((tp), (tpstr)); \
    _CTR_ITERIF((cont_p)) = (it);                          \
} while (0)

#define _CTR_DESTROY(cont_p) do {                          \
    _free_type_node(_CTR_STNODE((cont_p)));                \
} while (0)



/* iterator */
typedef struct _tag_iterator_t {
    union {
        char *ptr; /* for list and vector */
    }itpos;
    void *container;
} iterator_t;

typedef struct _tag_iterator_if {
    /* public interface */
    iterator_t (*next)(iterator_t);
    iterator_t (*pre)(iterator_t);
    iterator_t (*next_n)(iterator_t, int);
    iterator_t (*pre_n)(iterator_t, int);
    bool_t (*equal)(iterator_t, iterator_t);
    int (*distance)(iterator_t, iterator_t);
    void* (*get_pointer)(iterator_t);
    void (*get_value)(iterator_t, void*);
    void (*set_value)(iterator_t, void*);

    /* private interface */
    bool_t (*is_vaild)(iterator_t);
} _iterator_if;

#define _ITER_CONTAINER(iter)      ((iter).container)
#define _ITER_CONTAINER_BASE(iter) ((_container_base*)_ITER_CONTAINER(iter))
#define _ITER_TYPE(iter)           _CTR_STYPE(_ITER_CONTAINER_BASE(iter))
#define _ITER_IF(iter)             _CTR_ITERIF(_ITER_CONTAINER_BASE(iter))
#define _ITER_VPTR(iter)           ((iter).itpos.ptr)
#define _ITER_LPTR(iter)           ((iter).itpos.ptr)
#define _ITER_RBTPTR(iter)         ((iter).itpos.ptr)
#define _ITER_RBTREE(iter)         ((_rbtree_t*)(_ITER_CONTAINER_BASE(iter)->data))

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
#endif  //OHSTL_CITERATOR_H