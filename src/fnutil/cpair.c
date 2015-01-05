#include <assert.h>
#include <fnutil/cpair.h>


pair_t* _pair_new(const char* tpstr) {
    assert(tpstr != NULL);

    pair_t *pair = (pair_t*)fn_malloc(sizeof(pair_t));
    RETURN_IF_NULL(pair, NULL);

    if (_pair_init(pair, tpstr) < 0) {
        fn_free(pair);
        return NULL;
    }
    return pair;
}

int _pair_init(pair_t *pair, const char* tpstr) {
    assert(pair   != NULL);
    assert(tpstr != NULL);

    _pair_init_aux(pair, _get_type_node(_get_type_bystr(_PAIR_TYPE_NAME), tpstr));

    return 0;
}

void pair_destroy(pair_t *pair) {
    assert(pair != NULL);

    _pair_destroy_aux(pair);
    _free_type_node(pair->tpnode);
}

void pair_delete(pair_t *pair) {
    assert(pair != NULL);

    pair_destroy(pair);
    fn_free(pair);
}

void _pair_destroy_aux(pair_t *pair) {
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
}

void _pair_init_aux(pair_t *pair, type_node *node) {
    assert(pair != NULL);
    assert(_TPNODE_TYPE(node) == _get_type_bystr(_PAIR_TYPE_NAME));
    assert(_TPNODE_LCNODE(node) != NULL);
    assert(_TPNODE_RCNODE(node) != NULL);

    pair->tpnode = node;

    int i;
    for (i = 0; i < 2; i++) {
        pair->ptr[i]  = fn_malloc(_PAIR_CTYPE(pair, i)->size);
        _type_init(_PAIR_CTYPE(pair, i), pair->ptr[i], _PAIR_CTNODE(pair, i));
    }


}

void pair_assign(pair_t *pair1, const pair_t *pair2) {
    assert(pair1 != NULL);
    assert(pair2 != NULL);
    assert(_PAIR_CTYPE(pair1, 0) == _PAIR_CTYPE(pair2, 0));
    assert(_PAIR_CTYPE(pair1, 1) == _PAIR_CTYPE(pair2, 1));

    _type_copy(_PAIR_CTYPE(pair1, 0), _PAIR_LPTR(pair1), _PAIR_LPTR(pair2));
    _type_copy(_PAIR_CTYPE(pair1, 1), _PAIR_RPTR(pair1), _PAIR_RPTR(pair2));
}

bool_t _pair_less(const pair_t *pair1, const pair_t *pair2) {
    assert(pair1 != NULL);
    assert(pair2 != NULL);
    assert(_PAIR_CTYPE(pair1, 0) == _PAIR_CTYPE(pair2, 0));
    assert(_PAIR_CTYPE(pair1, 1) == _PAIR_CTYPE(pair2, 1));


    return _type_less(_PAIR_CTYPE(pair1, 0), _PAIR_LPTR(pair1), _PAIR_LPTR(pair2)) || (
           _type_equal(_PAIR_CTYPE(pair1, 0), _PAIR_LPTR(pair1), _PAIR_LPTR(pair2)) &&
           _type_less(_PAIR_CTYPE(pair1, 1), _PAIR_RPTR(pair1), _PAIR_RPTR(pair2)));

}

bool_t _pair_key_less(const pair_t *pair1, const pair_t *pair2) {
    assert(pair1 != NULL);
    assert(pair2 != NULL);
    assert(_PAIR_CTYPE(pair1, 0) == _PAIR_CTYPE(pair2, 0));
    assert(_PAIR_CTYPE(pair1, 1) == _PAIR_CTYPE(pair2, 1));

    return _type_less(_PAIR_CTYPE(pair1, 0), _PAIR_LPTR(pair1), _PAIR_LPTR(pair2));
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

