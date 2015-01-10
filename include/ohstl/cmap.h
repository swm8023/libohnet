#ifndef OHSTL_CMAP_H
#define OHSTL_CMAP_H

#ifdef __cplusplus
extern "C" {
#endif


#include <ohstl/citerator.h>
#include <ohstl/rbtree.h>
#include <ohstl/cpair.h>


typedef struct _tag_map_t {
    _CONTAINER_BASE;
    _rbtree_t rbt;
    type_node prnode;
    pair_t tmp_memory;
} map_t;

#define _MAP_SIZE(st)           _RBT_SIZE(&(st)->rbt)
#define _ITER_CONTAIN_MAP(iter) ((map_t*)((iter).container))
#define _MAP_RBT(st)            (&(st)->rbt)

#define _MAP_PTYPE(mp)          _TPNODE_TYPE(&((mp)->prnode))
#define _MAP_LTYPE(mp)          _CTR_LTYPE(mp)
#define _MAP_RTYPE(mp)          _CTR_RTYPE(mp)
#define _MAP_TMPMEM(mp)         (&((mp)->tmp_memory))
#define _MAP_LTMPMEM(mp)        ((mp)->tmp_memory.ptr[0])
#define _MAP_RTMPMEM(mp)        ((mp)->tmp_memory.ptr[1])

#define map_new(...)      _map_new(#__VA_ARGS__)
#define map_init(st, ...) _map_init((st), #__VA_ARGS__)

map_t* _map_new(const char* typestr);
int _map_init(map_t*, const char*);

void map_destroy(map_t*);
void map_delete(map_t*);

void _map_destroy_aux(map_t*);
void _map_init_aux(map_t*, type_node*);

#define map_empty(st)       (_MAP_SIZE(st) == 0)
#define map_size(st)        _MAP_SIZE(st)


void _map_make_key(map_t*, ...);
void _map_make_value(map_t*, ...);

#define map_put(mp, elm1, elm2) ({   \
    _map_make_key(mp, elm1);                \
    _map_make_value(mp, elm2);              \
    map_insert(mp, _MAP_TMPMEM(mp));        \
})

int map_count(map_t*, ...);
void map_clear(map_t*);

void map_erase(map_t*, iterator_t);
void map_erase_val(map_t*, ...);

void* map_at(map_t*, ...);
int map_at_val(map_t*, void*, ...);
iterator_t map_find(map_t*, ...);
iterator_t map_insert(map_t*, pair_t *pr);
iterator_t map_begin(map_t*);
iterator_t map_end(map_t*);




#ifdef __cplusplus
}
#endif
#endif  //OHSTL_CMAP_H