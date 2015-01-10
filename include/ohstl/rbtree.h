#ifndef OHSTL_RBTREE_H
#define OHSTL_RBTREE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>


#include <ohstl/citerator.h>



#define _RBT_RED 0
#define _RBT_BLK 1


typedef struct _tag_rbtreenode_t {
    struct _tag_rbtreenode_t *left, *right, *parent;
    char color;
    char data[0];
} _rbtreenode_t;

/* about guard node g: l[g]->min, r[g]->max, p[g]->root */
typedef struct _tag_rbtree_t {
    _alloc_if *allocif;
    type_node *tpnode;
    size_t size;
    bool_t is_multi;
    _rbtreenode_t guard;
    _tfunc_less less;
} _rbtree_t;

extern _iterator_if _default_rbt_iter_if;

#define _RBT_TYPE_SIZE(rbt) _CTR_CTYPE_SIZE((rbt))
#define _RBT_SIZE(rbt)      ((rbt)->size)
#define _RBT_TYPE(rbt)     _TPNODE_TYPE((rbt)->tpnode)

#define _RBT_GUARD(rbt)     (&(rbt)->guard)
#define _RBT_ROOT(rbt)      (_RBT_GUARD((rbt))->parent)

#define _RBT_ALLOC(rbt, sz) ((rbt)->allocif->alloc(sz))
#define _RBT_FREE(rbt, ptr) ((rbt)->allocif->free(ptr))

#define _RBTNODE_ISROOT(rbt, node)  ((node) == _RBT_ROOT(rbt))

#define _RBTNODE_ISBLK(node)        ((node) == NULL || (node)->color == _RBT_BLK)
#define _RBTNODE_ISRED(node)        (!_RBTNODE_ISBLK((node)))
#define _RBTNODE_SETCLR(node, clr)  ((node) && ((node)->color = (clr)))

#define _RBTNODE_SETRED(node)       ((node) && ((node)->color = _RBT_RED))
#define _RBTNODE_SETBLK(node)       ((node) && ((node)->color = _RBT_BLK))

#define _RBTNODE_DATAP(node)        ((char*)(node) + offsetof(_rbtreenode_t, data))
#define _RBTNODE_BEGINP(datap)      (container_of((datap), _rbtreenode_t, data))
#define _RBTNODE_SIZE(type)         (sizeof(_rbtreenode_t) + (type)->size)

#define _RBTNODE_VAL_LESS(rbt, val1, val2) (rbt)->less((val1), (val2))

#define _RBTNODE_LESS(rbt, nd1, nd2)        \
    (rbt)->less(_RBTNODE_DATAP((nd1)), _RBTNODE_DATAP((nd2)))



#define _RBTNODE_INIT(node, parent) do {    \
    (node)->parent   = paraent;             \
    (node)->child[0] = NULL;                \
    (node)->child[1] = NULL;                \
    (node)->color = _RBT_RED;               \
} while (0)



void _rbt_init(_rbtree_t*, type_node *, bool_t, bool_t, _alloc_if*);

// int _rbt_init_if_node(_rbtree_t*, type_node *, _func_less);

void _rbt_destroy(_rbtree_t *);

#define _rbtnode_less(rbt, nd1, nd2)        \
    _rbtnode_val_less(rbt, _RBTNODE_DATAP((nd1)), _RBTNODE_DATAP((nd2)));


void _rbt_left_rotate(_rbtree_t*, _rbtreenode_t*);
void _rbt_right_rotate(_rbtree_t*, _rbtreenode_t*);

_rbtreenode_t* _rbt_new_treenode(_rbtree_t*, const void*);
void _rbt_free_treenode(_rbtree_t *, _rbtreenode_t*);

_rbtreenode_t* _rbt_insert_val(_rbtree_t*, const void*);
int _rbt_insert_node_fixup(_rbtree_t*, _rbtreenode_t*);

int _rbt_erase_val(_rbtree_t*, const void*);
int _rbt_erase_node_fixup(_rbtree_t*, _rbtreenode_t*,_rbtreenode_t*);

_rbtreenode_t* _rbt_tree_predecessor(_rbtreenode_t*);
_rbtreenode_t* _rbt_tree_successor(_rbtreenode_t*);
_rbtreenode_t* _rbt_tree_minimum(_rbtreenode_t*);
_rbtreenode_t* _rbt_tree_maximum(_rbtreenode_t*);

_rbtreenode_t *_rbt_find_val(_rbtree_t*, _rbtreenode_t **parent, const void*);

_rbtreenode_t* _rbt_begin_node(_rbtree_t*);

bool_t _rbt_is_vaild(_rbtree_t *);

#ifdef __cplusplus
}
#endif
#endif  //OHSTL_RBTREE_H