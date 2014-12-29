#include <stdio.h>
#include <fnutil/citerator.h>
#include <fnutil/ctypes.h>
#include <fnutil/cvector.h>


/* public functions */

iterator_t iter_next(iterator_t iter) {
    assert(_iter_is_vaild(iter));
    assert(_ITER_IF(iter)->next != NULL);

    return _ITER_IF(iter)->next(iter);
}

iterator_t iter_pre(iterator_t iter) {
    assert(_iter_is_vaild(iter));
    assert(_ITER_IF(iter)->pre != NULL);

    return _ITER_IF(iter)->pre(iter);
}

iterator_t iter_next_n(iterator_t iter, int step) {
    assert(_iter_is_vaild(iter));
    assert(_ITER_IF(iter)->next_n != NULL);

    return _ITER_IF(iter)->next_n(iter, step);
}

iterator_t iter_pre_n(iterator_t iter, int step) {
    assert(_iter_is_vaild(iter));
    assert(_ITER_IF(iter)->pre_n != NULL);

    return _ITER_IF(iter)->pre_n(iter, step);
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
    assert(_ITER_IF(iter0)->distance != NULL);

    return _ITER_IF(iter0)->distance(iter0, iter1);
}

void* iter_get_pointer(iterator_t iter) {
    assert(_iter_is_vaild(iter));
    assert(_ITER_IF(iter)->get_pointer != NULL);

    return _ITER_IF(iter)->get_pointer(iter);
}

void iter_get_value(iterator_t iter, void* pelm) {
    assert(pelm != NULL);
    assert(_iter_is_vaild(iter));
    assert(_ITER_IF(iter)->get_value != NULL);

    _ITER_IF(iter)->get_value(iter, pelm);
}

void iter_set_value(iterator_t iter, void* pelm) {
    assert(pelm != NULL);
    assert(_iter_is_vaild(iter));
    assert(_ITER_IF(iter)->set_value != NULL);

    _ITER_IF(iter)->set_value(iter, pelm);
}

bool_t iter_equal(iterator_t iter0, iterator_t iter1) {
    assert(_iter_is_vaild(iter0));
    assert(_iter_is_vaild(iter1));
    assert(_iter_is_same(iter0, iter1));
    assert(_ITER_IF(iter0)->equal != NULL);

    return _ITER_IF(iter0)->equal(iter0, iter1);
}


/* private functions */

bool_t _iter_is_vaild(iterator_t iter) {
    RETURN_IF_NULL(_ITER_CONTAINER(iter), false);
    RETURN_IF_NULL(_ITER_IF(iter),        false);

    _type_t *type = _ITER_TYPE(iter);

    if (type->type_id < _TYPEID_FNBUILTIN_START ||
        type->type_id > _TYPEID_FNBUILTIN_END) {
        return false;
    }
    bool_t ret = true;
    if (_ITER_IF(iter)->is_vaild) {
        ret = _ITER_IF(iter)->is_vaild(iter);
    }
    return ret;
}

bool_t _iter_is_same(iterator_t iter0, iterator_t iter1) {
    assert(_iter_is_vaild(iter0));
    assert(_iter_is_vaild(iter1));

    return _ITER_CONTAINER(iter0) == _ITER_CONTAINER(iter1)  ? true : false;
}

bool_t _iter_is_contained(iterator_t iter, void *ptr) {
    assert(_iter_is_vaild(iter));

    return _ITER_CONTAINER(iter) == ptr;
}