#include <assert.h>
#include <fnstl/cmap.h>
#include <fnstl/cpair.h>


map_t* _map_new(const char* typestr) {
    assert(typestr != NULL);

    map_t* mp = (map_t*)fn_malloc(sizeof(map_t));
    RETURN_IF_NULL(mp, NULL);

    if (_map_init(mp, typestr) < 0) {
        fn_free(mp);
        return NULL;
    }

    return mp;
}

int _map_init(map_t* mp, const char* typestr) {
    assert(mp != NULL);
    assert(typestr != NULL);

    /* init container base attr*/
    _CTR_INIT(mp, _get_type_bystr(_MAP_TYPE_NAME), typestr, &_default_rbt_iter_if);

    _CTR_MEMIF(mp) =  _default_cmem_if;

    /* check if type rigth*/
    assert(_CTR_STYPE(mp) != NULL);
    assert(_CTR_CTYPE(mp) != NULL);

    /* set pair node, use to init rbt tree  */
    mp->prnode.type = _get_type_bystr(_PAIR_TYPE_NAME);
    mp->prnode.conttype[0] = _CTR_LTNODE(mp);
    mp->prnode.conttype[1] = _CTR_RTNODE(mp);

    /* map attr */
    _rbt_init(_MAP_RBT(mp), &mp->prnode, false, false, _CTR_MEMIF(mp));

    /* alloc temp memory */
    _type_init(_MAP_PTYPE(mp), _MAP_TMPMEM(mp), &mp->prnode);
    return 0;
}

void map_clear(map_t* mp) {
    assert(mp != NULL);

    _rbt_destroy(_MAP_RBT(mp));
    _rbt_init(_MAP_RBT(mp), &mp->prnode, false, false, _CTR_MEMIF(mp));
}

void map_destroy(map_t* mp) {
    assert(mp != NULL);

    _map_destroy_aux(mp);
    _CTR_DESTROY(mp);
}

void _map_destroy_aux(map_t* mp) {
    assert(mp != NULL);

    _rbt_destroy(_MAP_RBT(mp));

    /* free temp memory */
    _type_destroy(_MAP_PTYPE(mp), _MAP_TMPMEM(mp));
}

void map_delete(map_t* mp) {
    assert(mp != NULL);

    map_destroy(mp);
    fn_free(mp);
}

int map_count(map_t* mp, ...) {
    assert(mp != NULL);

    /* copy to pair's left pointer */
    va_list arg;
    va_start(arg, mp);
    _get_varg_value_bytype(_MAP_LTYPE(mp), arg, _MAP_LTMPMEM(mp));
    va_end(arg);

    int count = _rbt_find_val(_MAP_RBT(mp), NULL, _MAP_TMPMEM(mp)) ? 1 : 0;

    return count;
}

iterator_t map_find(map_t* mp, ...) {
    assert(mp != NULL);

    va_list arg;
    iterator_t iter;
    va_start(arg, mp);
    _get_varg_value_bytype(_MAP_LTYPE(mp), arg, _MAP_LTMPMEM(mp));
    va_end(arg);

    _rbtreenode_t* node = _rbt_find_val(_MAP_RBT(mp), NULL, _MAP_TMPMEM(mp));

    RETURN_IF_NULL(node, map_end(mp));

    _ITER_INIT(iter);
    _ITER_CONTAINER(iter) = mp;
    _ITER_RBTPTR(iter) = (char*)node;
    return iter;
}

void *map_at(map_t* mp, ...) {
    assert(mp != NULL);

    va_list arg;
    va_start(arg, mp);
    _get_varg_value_bytype(_MAP_LTYPE(mp), arg, _MAP_LTMPMEM(mp));
    va_end(arg);

    _rbtreenode_t* node = _rbt_find_val(_MAP_RBT(mp), NULL, _MAP_TMPMEM(mp));

    RETURN_IF_NULL(node, NULL);

    return _PAIR_RPTR((pair_t*)(_RBTNODE_DATAP(node)));
}

int map_at_val(map_t *mp, void* valp, ...) {
    assert(mp != NULL);
    assert(valp != NULL);

    va_list arg;
    va_start(arg, valp);
    _get_varg_value_bytype(_MAP_LTYPE(mp), arg, _MAP_LTMPMEM(mp));
    va_end(arg);

    _rbtreenode_t* node = _rbt_find_val(_MAP_RBT(mp), NULL, _MAP_TMPMEM(mp));

    RETURN_IF_NULL(node, -1);
    _type_copy(_MAP_RTYPE(mp), valp, _PAIR_RPTR((pair_t*)(_RBTNODE_DATAP(node))));
    return 0;
}


void map_erase(map_t* mp, iterator_t iter) {
    assert(mp != NULL);
    assert(_iter_is_vaild(iter));

    RETURN_IF(iter_equal(iter, map_end(mp)));

    _rbt_erase_node(_MAP_RBT(mp), (_rbtreenode_t*)_ITER_RBTPTR(iter));
}

void map_erase_val(map_t* mp, ...) {
    assert(mp != NULL);

    va_list arg;
    va_start(arg, mp);
    _get_varg_value_bytype(_MAP_LTYPE(mp), arg, _MAP_LTMPMEM(mp));
    va_end(arg);

    _rbt_erase_val(_MAP_RBT(mp), _MAP_TMPMEM(mp));
}

iterator_t map_insert(map_t* mp, pair_t* pr) {
    assert(mp != NULL);

    va_list arg;
    iterator_t iter;
    _rbtreenode_t* node = _rbt_insert_val(_MAP_RBT(mp), pr);

    RETURN_IF_NULL(node, map_end(mp));

    _ITER_INIT(iter);
    _ITER_CONTAINER(iter) = mp;
    _ITER_RBTPTR(iter) = (char*)node;
    return iter;
}

void _map_make_key(map_t* mp, ...) {
    assert(mp != NULL);

    va_list arg;
    va_start(arg, mp);
    _get_varg_value_bytype(_MAP_LTYPE(mp), arg, _MAP_LTMPMEM(mp));
    va_end(arg);
}

void _map_make_value(map_t*mp, ...) {
    assert(mp != NULL);

    va_list arg;
    va_start(arg, mp);
    _get_varg_value_bytype(_MAP_RTYPE(mp), arg, _MAP_RTMPMEM(mp));
    va_end(arg);
}

iterator_t map_begin(map_t* mp) {
    assert(mp != NULL);

    RETURN_IF(_MAP_SIZE(mp) == 0, map_end(mp));

    iterator_t iter;
    _ITER_INIT(iter);
    _ITER_CONTAINER(iter) = mp;
    _ITER_RBTPTR(iter) = (char*)_rbt_begin_node(_MAP_RBT(mp));
    return iter;
}

iterator_t map_end(map_t* mp) {
    assert(mp != NULL);

    iterator_t iter;
    _ITER_INIT(iter);
    _ITER_CONTAINER(iter) = mp;
    _ITER_RBTPTR(iter) = (char*)_RBT_GUARD(_MAP_RBT(mp));
    return iter;
}