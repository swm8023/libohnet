#include <assert.h>
#include <fnutil/cpair.h>


pair_t* _new_pair(const char* tpstr) {
    assert(tpstr != NULL);

    pair_t *pair = (pair_t*)fn_malloc(sizeof(pair_t));
    RETURN_IF_NULL(pair, NULL);

    if (_init_pair(pair, tpstr) < 0) {
        fn_free(pair);
        return NULL;
    }
    return pair;
}

int _init_pair(pair_t *pair, const char* tpstr) {
    assert(pair   != NULL);
    assert(tpstr != NULL);

    pair->tpnode = _get_type_node(_get_type_bystr(_PAIR_TYPE_NAME), tpstr);

    assert(_CTR_STYPE(pair) != NULL);

    int i;
    for (i = 0; i < 2; i++) {
        assert(_PAIR_CTYPE(pair, i) != NULL);

        pair->ptr[i]  = fn_malloc(_PAIR_CTYPE(pair, i)->size);
        _type_init(_PAIR_CTYPE(pair, i), pair->ptr[i], _PAIR_CTNODE(pair, i));
    }

    return 0;
}

void destroy_pair(pair_t *pair) {
    assert(pair != NULL);

    int i;
    for (i = 0; i < 2; i++) {
        assert(_PAIR_CTYPE(pair, i) != NULL);

        /* not null, destroy and free this type object*/
        if (_PAIR_CPTR(pair, i) != NULL) {
            _type_destroy(_PAIR_CTYPE(pair, i), _PAIR_CPTR(pair, i));
            fn_free(_PAIR_CPTR(pair, i));
        }
    }
    _free_type_node(pair->tpnode);
}

void delete_pair(pair_t *pair) {
    assert(pair != NULL);

    destroy_pair(pair);
    fn_free(pair);
}


void pair_assign(pair_t *pair1, pair_t *pair2) {

}

bool_t pair_less(pair_t *pair1, pair_t *pair2) {

}

void _pair_make_elem(pair_t *pair, int index, ...) {
    va_list arg;

    assert(pair != NULL);
    assert(index < 2);
    assert(_PAIR_CTYPE(pair, index) != NULL);

    va_start(arg, index);
    _get_varg_value_bytype(_PAIR_CTYPE(pair, index), arg, _PAIR_CPTR(pair, index));
    va_end(arg);
}

void _pair_get_value(pair_t *pair, int index, void* elmp) {
    assert(pair != NULL);
    assert(index < 2);
    assert(elmp != NULL);
    assert(_PAIR_CTYPE(pair, index) != NULL);

    _type_copy(_PAIR_CTYPE(pair, index), elmp, _PAIR_CPTR(pair, index));
}

void* _pair_get(pair_t* pair, int index) {
    assert(pair != NULL);
    assert(index < 2);
    assert(_PAIR_CTYPE(pair, index) != NULL);

    return _PAIR_CPTR(pair, index);
}

