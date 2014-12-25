#include <stdio.h>
#include <fnutil/citerator.h>
#include <fnutil/ctypes.h>
#include <fnutil/cvector.h>


/* public functions */

iterator_t iter_next(iterator_t iter) {
    assert(_iter_is_vaild(iter));
    assert(_ITER_IF(iter)->_next != NULL);

    return _ITER_IF(iter)->_next(iter);
}

iterator_t iter_pre(iterator_t iter) {
    assert(_iter_is_vaild(iter));
    assert(_ITER_IF(iter)->_pre != NULL);

    return _ITER_IF(iter)->_pre(iter);
}

iterator_t iter_next_n(iterator_t iter, int step) {
    assert(_iter_is_vaild(iter));
    assert(_ITER_IF(iter)->_next_n != NULL);

    return _ITER_IF(iter)->_next_n(iter, step);
}

iterator_t iter_pre_n(iterator_t iter, int step) {
    assert(_iter_is_vaild(iter));
    assert(_ITER_IF(iter)->_pre_n != NULL);

    return _ITER_IF(iter)->_pre_n(iter, step);
}

iterator_t iter_advance(iterator_t iter, int step) {
    assert(_iter_is_vaild(iter));

    bool_t flag = step > 0 ? true : false;
    if (flag == false) step = -step;
    while (step-- > 0) {
        iter = flag ? iter_next(iter) : iter_pre(iter);
    }
    return iter;
}

int iter_distance(iterator_t iter0, iterator_t iter1) {
    assert(_iter_is_vaild(iter0));
    assert(_iter_is_vaild(iter1));
    assert(_iter_is_same(iter0, iter1));
    assert(_ITER_IF(iter0)->_distance != NULL);

    return _ITER_IF(iter0)->_distance(iter0, iter1);
}

void* iter_get_pointer(iterator_t iter) {
    assert(_iter_is_vaild(iter));
    assert(_ITER_IF(iter)->_get_pointer != NULL);

    return _ITER_IF(iter)->_get_pointer(iter);
}

void iter_get_value(iterator_t iter, void* pelm) {
    assert(pelm != NULL);
    assert(_iter_is_vaild(iter));
    assert(_ITER_IF(iter)->_get_value != NULL);

    _ITER_IF(iter)->_get_value(iter, pelm);
}

void iter_set_value(iterator_t iter, void* pelm) {
    assert(pelm != NULL);
    assert(_iter_is_vaild(iter));
    assert(_ITER_IF(iter)->_set_value != NULL);

    _ITER_IF(iter)->_set_value(iter, pelm);
}

bool_t iter_equal(iterator_t iter0, iterator_t iter1) {
    assert(_iter_is_vaild(iter0));
    assert(_iter_is_vaild(iter1));
    assert(_iter_is_same(iter0, iter1));
    assert(_ITER_IF(iter0)->_equal != NULL);

    return _ITER_IF(iter0)->_equal(iter0, iter1);
}


/* private functions */

bool_t _iter_is_vaild(iterator_t iter) {
    RETURN_IF(iter._container == NULL, false);
    RETURN_IF(_ITER_IF(iter) == NULL, false);

    _type_t *type = _ITER_TYPE(iter);

    if (type->type_id < _TYPEID_FNBUILTIN_START ||
        type->type_id > _TYPEID_FNBUILTIN_END) {
        return false;
    }
    bool_t ret = true;
    if (_ITER_IF(iter)->_is_vaild) {
        ret = _ITER_IF(iter)->_is_vaild(iter);
    }
    return ret;
}

bool_t _iter_is_same(iterator_t iter0, iterator_t iter1) {
    assert(_iter_is_vaild(iter0));
    assert(_iter_is_vaild(iter1));

    return iter0._container == iter1._container ? true : false;
}

bool_t _iter_is_contained(iterator_t iter, void *ptr) {
    assert(_iter_is_vaild(iter));

    return iter._container == ptr;
}