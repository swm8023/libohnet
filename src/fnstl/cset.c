#include <stdio.h>
#include <assert.h>
#include <fnstl/cset.h>


set_t* _set_new(const char* typestr) {
    assert(typestr != NULL);

    set_t* st = (set_t*)fn_malloc(sizeof(set_t));
    RETURN_IF_NULL(st, NULL);

    if (_set_init(st, typestr) < 0) {
        fn_free(st);
        return NULL;
    }

    return st;
}

int _set_init(set_t* st, const char* typestr) {
    assert(st != NULL);
    assert(typestr != NULL);

    /* init container base attr*/
    _CTR_INIT(st, _get_type_bystr(_SET_TYPE_NAME), typestr, &_default_rbt_iter_if);

    _CTR_MEMIF(st) =  _default_cmem_if;

    /* check if type rigth*/
    assert(_CTR_STYPE(st) != NULL);
    assert(_CTR_CTYPE(st) != NULL);

    /* set attr */
    _rbt_init(_SET_RBT(st), _CTR_CTNODE(st), true, false, _CTR_MEMIF(st));

    /* alloc temp memory */
    st->tmp_memory = _CTR_ALLOC(st, _CTR_CTYPE_SIZE(st));
    _type_init(_CTR_CTYPE(st), st->tmp_memory, _CTR_CTNODE(st));
    return 0;
}

void set_clear(set_t* st) {
    assert(st != NULL);

    _rbt_destroy(_SET_RBT(st));
    _rbt_init(_SET_RBT(st), _CTR_CTNODE(st), true, false, _CTR_MEMIF(st));
}

void set_destroy(set_t* st) {
    assert(st != NULL);

    _set_destroy_aux(st);
    _CTR_DESTROY(st);
}

void _set_destroy_aux(set_t* st) {
    assert(st != NULL);

    _rbt_destroy(_SET_RBT(st));

    /* free temp memory */
    _type_destroy(_CTR_CTYPE(st), _SET_TMPMEM(st));
    _CTR_FREE(st, _SET_TMPMEM(st));
}

void set_delete(set_t* st) {
    assert(st != NULL);

    set_destroy(st);
    fn_free(st);
}

int set_count(set_t* st, ...) {
    va_list arg;

    assert(st != NULL);

    // void *temp = fn_malloc(_CTR_CTYPE_SIZE(st));
    // _type_init(_CTR_CTYPE(st), temp, _CTR_CTNODE(st));

    va_start(arg, st);
    _get_varg_value_bytype(_CTR_CTYPE(st), arg, _SET_TMPMEM(st));
    va_end(arg);

    int count = _rbt_find_val(_SET_RBT(st), NULL, _SET_TMPMEM(st)) ? 1 : 0;

    // _type_destroy(_CTR_CTYPE(st), temp);
    // fn_free(temp);

    return count;
}


void set_erase_val(set_t* st, ...) {
    va_list arg;

    assert(st != NULL);

    // void *temp = fn_malloc(_CTR_CTYPE_SIZE(st));
    // _type_init(_CTR_CTYPE(st), temp, _CTR_CTNODE(st));

    va_start(arg, st);
    _get_varg_value_bytype(_CTR_CTYPE(st), arg, _SET_TMPMEM(st));
    va_end(arg);

    _rbt_erase_val(_SET_RBT(st), _SET_TMPMEM(st));

    // _type_destroy(_CTR_CTYPE(st), temp);
    // fn_free(temp);
}

void set_erase(set_t* st, iterator_t iter) {
    assert(st != NULL);
    assert(_iter_is_vaild(iter));
    assert(!iter_equal(iter, set_end(st)));

    _rbt_erase_node(_SET_RBT(st), (_rbtreenode_t*)_ITER_RBTPTR(iter));
}

iterator_t set_find(set_t* st, ...) {
    va_list arg;
    iterator_t iter;

    assert(st != NULL);

    // void *temp = fn_malloc(_CTR_CTYPE_SIZE(st));
    // _type_init(_CTR_CTYPE(st), temp, _CTR_CTNODE(st));

    va_start(arg, st);
    _get_varg_value_bytype(_CTR_CTYPE(st), arg, _SET_TMPMEM(st));
    va_end(arg);

    _rbtreenode_t* node = _rbt_find_val(_SET_RBT(st), NULL, _SET_TMPMEM(st));
    // _type_destroy(_CTR_CTYPE(st), temp);
    // fn_free(temp);

    RETURN_IF_NULL(node, set_end(st));

    _ITER_INIT(iter);
    _ITER_CONTAINER(iter) = st;
    _ITER_RBTPTR(iter) = (char*)node;
    return iter;
}

iterator_t set_insert(set_t* st, ...) {
    va_list arg;
    iterator_t iter;

    assert(st != NULL);

    // void *temp = fn_malloc(_CTR_CTYPE_SIZE(st));
    // _type_init(_CTR_CTYPE(st), temp, _CTR_CTNODE(st));

    va_start(arg, st);
    _get_varg_value_bytype(_CTR_CTYPE(st), arg, _SET_TMPMEM(st));
    va_end(arg);

    _rbtreenode_t* node = _rbt_insert_val(_SET_RBT(st), _SET_TMPMEM(st));

    // _type_destroy(_CTR_CTYPE(st), temp);
    // fn_free(temp);

    RETURN_IF_NULL(node, set_end(st));

    _ITER_INIT(iter);
    _ITER_CONTAINER(iter) = st;
    _ITER_RBTPTR(iter) = (char*)node;
    return iter;
}

iterator_t set_begin(set_t* st) {
    assert(st != NULL);

    RETURN_IF(_SET_SIZE(st) == 0, set_end(st));

    iterator_t iter;
    _ITER_INIT(iter);
    _ITER_CONTAINER(iter) = st;
    _ITER_RBTPTR(iter) = (char*)_rbt_begin_node(_SET_RBT(st));
    return iter;
}

iterator_t set_end(set_t* st) {
    assert(st != NULL);

    iterator_t iter;
    _ITER_INIT(iter);
    _ITER_CONTAINER(iter) = st;
    _ITER_RBTPTR(iter) = (char*)_RBT_GUARD(_SET_RBT(st));
    return iter;
}