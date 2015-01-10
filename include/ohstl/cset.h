#ifndef OHSTL_CSET_H
#define OHSTL_CSET_H

#ifdef __cplusplus
extern "C" {
#endif


#include <ohstl/citerator.h>
#include <ohstl/rbtree.h>


typedef struct _tag_set_t {
    _CONTAINER_BASE;
    _rbtree_t rbt;
    void *tmp_memory;
} set_t;

#define _SET_TYPE_SIZE(st)      _CTR_CTYPE_SIZE(st)
#define _SET_SIZE(st)           _RBT_SIZE(&(st)->rbt)
#define _ITER_CONTAIN_SET(iter) ((set_t*)((iter).container))
#define _SET_RBT(st)            (&(st)->rbt)

#define _SET_TMPMEM(st)         ((st)->tmp_memory)


#define set_new(...)      _set_new(#__VA_ARGS__)
#define set_init(st, ...) _set_init((st), #__VA_ARGS__)

set_t* _set_new(const char* typestr);
int _set_init(set_t*, const char*);

void set_destroy(set_t*);
void set_delete(set_t*);

void _set_destroy_aux(set_t*);
void _set_init_aux(set_t*, type_node*);

#define set_empty(st)       (_SET_SIZE(st) == 0)
#define set_size(st)        _SET_SIZE(st)

int set_count(set_t*, ...);
void set_clear(set_t*);

void set_erase_val(set_t*, ...);
void set_erase(set_t*, iterator_t);

iterator_t set_find(set_t*, ...);
iterator_t set_insert(set_t*, ...);
iterator_t set_begin(set_t*);
iterator_t set_end(set_t*);




#ifdef __cplusplus
}
#endif
#endif  //FNUTIL_CSET_H