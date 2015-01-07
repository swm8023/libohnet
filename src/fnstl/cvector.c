#include <stdio.h>
#include <assert.h>
#include <fnstl/cvector.h>


/* vector iterator interface */
static iterator_t _vector_iter_next(iterator_t);
static iterator_t _vector_iter_pre(iterator_t);
static iterator_t _vector_iter_next_n(iterator_t, int);
static iterator_t _vector_iter_pre_n(iterator_t, int);
static bool_t _vector_iter_equal(iterator_t, iterator_t);
static int   _vector_iter_distance(iterator_t, iterator_t);
static void* _vector_iter_get_pointer(iterator_t);
static void  _vector_iter_get_value(iterator_t, void*);
static void  _vector_iter_set_value(iterator_t, void*);
static bool_t _vector_iter_is_vaild(iterator_t);

static _iterator_if _default_vector_iter_if = {
    _vector_iter_next,
    _vector_iter_pre,
    _vector_iter_next_n,
    _vector_iter_pre_n,
    _vector_iter_equal,
    _vector_iter_distance,
    _vector_iter_get_pointer,
    _vector_iter_get_value,
    _vector_iter_set_value,

    _vector_iter_is_vaild
};

/* vector opertion functions */
vector_t* _vector_new(const char* typestr) {
    assert(typestr != NULL);

    vector_t* vec = (vector_t*)fn_malloc(sizeof(vector_t));
    RETURN_IF_NULL(vec, NULL);

    if (_vector_init(vec, typestr) < 0) {
        fn_free(vec);
        return NULL;
    }

    return vec;
}

int _vector_init(vector_t* vec, const char* typestr) {
    assert(vec != NULL);
    assert(typestr != NULL);

    /* init container base attr*/
    _CTR_INIT(vec, _get_type_bystr(_VECT_TYPE_NAME), typestr, &_default_vector_iter_if);

    _CTR_MEMIF(vec) =  _default_cmem_if;

    /* check if type rigth*/
    assert(_CTR_STYPE(vec) != NULL);
    assert(_CTR_CTYPE(vec) != NULL);

    /* vector attr */
    vec->capacity   = _VECT_INIT_CAPACITY;
    vec->size       = 0;
    vec->data       = (char*)_CTR_ALLOC(vec, _VECT_CAPACITY(vec) * _VECT_TYPE_SIZE(vec));

    return _VECT_DATA(vec) ? 0 : -1;
}

void vector_reserve(vector_t* vec, size_t newcap) {
    assert (vec != NULL);

    /* if vec->capactity >= newcap, just return */
    RETURN_IF(_VECT_CAPACITY(vec) >= newcap);

    while (_VECT_CAPACITY(vec) < newcap) {
        vec->capacity *= 2;
    }

    vec->data = _CTR_REALLOC(vec, vec->data, _VECT_CAPACITY(vec) * _VECT_TYPE_SIZE(vec));

}

void vector_clear(vector_t* vec) {
    assert(vec != NULL);

    while (!vector_empty(vec))
        vector_pop_back(vec);
}

void vector_destroy(vector_t* vec) {
    assert(vec != NULL);

    vector_clear(vec);
    if (_VECT_DATA(vec)) {
        _CTR_FREE(vec, _VECT_DATA(vec));
    }

    _CTR_DESTROY(vec);
}

void vector_delete(vector_t* vec) {
    assert(vec != NULL);

    vector_destroy(vec);
    fn_free(vec);
}

void* vector_at(vector_t* vec, int pos) {
    assert(vec != NULL);
    assert(pos >= 0 && pos <= _VECT_SIZE(vec));

    return _VECT_DATA_OFFSET(vec, pos);
}

void* vector_front(vector_t* vec) {
    assert(vec != NULL);
    assert(!vector_empty(vec));

    return vector_at(vec, 0);
}

void* vector_back(vector_t* vec) {
    assert(vec != NULL);
    assert(!vector_empty(vec));

    return vector_at(vec, _VECT_SIZE(vec) - 1);
}

void vector_push_back(vector_t* vec, ...) {
    va_list arg;

    assert(vec != NULL);

    /* insert at the end of the vector */
    va_start(arg, vec);
    _vector_insert_varg(vec, _VECT_DATA_OFFSET(vec, _VECT_SIZE(vec)), arg);
    va_end(arg);
}

void vector_pop_back(vector_t* vec) {
    assert(vec != NULL);

    /* erase the last element */
    _vector_erase(vec, _VECT_DATA_OFFSET(vec, _VECT_SIZE(vec) - 1));
}

void vector_update(vector_t* vec, int pos, ...) {
    va_list arg;

    assert(vec != NULL);
    assert(pos >= 0 && pos < _VECT_SIZE(vec));

    /* just update, don't need call init */
    va_start(arg, pos);
    _get_varg_value_bytype(_CTR_CTYPE(vec), arg, _VECT_DATA_OFFSET(vec, pos));
    va_end(arg);
}

void vector_insert_pos(vector_t* vec, int pos, ...) {
    va_list arg;

    assert(vec != NULL);
    assert(pos >= 0 && pos <= _VECT_SIZE(vec));

    /* insert at this pos */
    va_start(arg, pos);
    _vector_insert_varg(vec, _VECT_DATA_OFFSET(vec, pos), arg);
    va_end(arg);
}

void vector_erase_pos(vector_t* vec, int pos) {
    assert(vec != NULL);
    assert(pos >= 0 && pos < _VECT_SIZE(vec));

    /* erase at this pos */
    _vector_erase(vec, _VECT_DATA_OFFSET(vec, pos));
}

iterator_t vector_insert(vector_t* vec, iterator_t iter, ...) {
    va_list arg;

    assert(vec != NULL);
    assert(_iter_is_vaild(iter));

    /* iterator maybe invaild after vector_reserve*/
    int dist = _ITER_VPTR(iter) - _VECT_DATA_BEGIN(vec);

    /* insert element at thie pos */
    va_start(arg, iter);
    _vector_insert_varg(vec, _ITER_VPTR(iter), arg);
    va_end(arg);

    /* iterator point to first inserted element */
    _ITER_VPTR(iter) = _VECT_DATA_BEGIN(vec) + dist;
    return iter;
}


iterator_t vector_erase(vector_t* vec, iterator_t iter) {
    assert(vec != NULL);
    assert(_iter_is_vaild(iter));
    assert(!iter_equal(iter, vector_end(_ITER_CONTAIN_VECT(iter))));

    /* destroy and remove element */
    _vector_erase(vec, _ITER_VPTR(iter));

    /* iterator point to the element after last erased element */
    return iter;
}

iterator_t vector_begin(vector_t* vec) {
    iterator_t iter;

    assert(vec != NULL);

    _ITER_INIT(iter);
    _ITER_CONTAINER(iter) = vec;
    _ITER_VPTR(iter) = _VECT_DATA_BEGIN(vec);
    return iter;
}

iterator_t vector_end(vector_t* vec) {
    iterator_t iter;

    assert(vec != NULL);

    _ITER_INIT(iter);
    _ITER_CONTAINER(iter) = vec;
    _ITER_VPTR(iter) = _VECT_DATA_END(vec);
    return iter;
}

/* private insert and erase functions */
void _vector_insert_varg(vector_t *vec, char* insptr, va_list arg) {
    assert(vec != NULL);
    assert(insptr != NULL);
    assert(insptr >= _VECT_DATA_BEGIN(vec));
    assert(insptr <= _VECT_DATA_END(vec));

    /* !!! insptr may be changed after vector reserve */
    int dist = insptr - _VECT_DATA_BEGIN(vec);
    vector_reserve(vec, _VECT_SIZE(vec) + 1);
    insptr = _VECT_DATA_BEGIN(vec) + dist;

    /* insert pointer is not the end of vector, leave space to put new element*/
    if (insptr != _VECT_DATA_END(vec)) {
        int tpsize = _VECT_TYPE_SIZE(vec);
        memmove(insptr + tpsize, insptr, _VECT_DATA_END(vec) - insptr);
    }

    vec->size ++;
    _type_init(_CTR_CTYPE(vec), insptr, _CTR_CTNODE(vec));
    _get_varg_value_bytype(_CTR_CTYPE(vec), arg, insptr);
}

void _vector_erase(vector_t *vec, char *eraptr) {
    assert(vec != NULL);
    assert(eraptr != NULL);

    _type_destroy(_CTR_CTYPE(vec), eraptr);
    vec->size --;

    /* erase pointer is not the last element of vector */
    if (eraptr != _VECT_DATA_END(vec)) {
        int tpsize = _VECT_TYPE_SIZE(vec);
        memmove(eraptr, eraptr + tpsize, _VECT_DATA_END(vec) - eraptr);
    }
}


/* vector iterator functions */
iterator_t _vector_iter_next(iterator_t iter) {
    assert(_iter_is_vaild(iter));

    return _vector_iter_next_n(iter, 1);
}

iterator_t _vector_iter_pre(iterator_t iter) {
    assert(_iter_is_vaild(iter));

    return _vector_iter_next_n(iter, -1);
}

iterator_t _vector_iter_next_n(iterator_t iter, int step) {
    assert(_iter_is_vaild(iter));

    _ITER_VPTR(iter) += step * _VECT_TYPE_SIZE(_ITER_CONTAIN_VECT(iter));

    /* check if iterator vaild after change */
    assert(_iter_is_vaild(iter));
    return iter;
}

iterator_t _vector_iter_pre_n(iterator_t iter, int step) {
    assert(_iter_is_vaild(iter));

    return _vector_iter_next_n(iter, -step);
}

bool_t _vector_iter_equal(iterator_t iter0, iterator_t iter1) {
    assert(_iter_is_vaild(iter0));
    assert(_iter_is_vaild(iter1));

    return _ITER_VPTR(iter0)  == _ITER_VPTR(iter1) ? true : false;
}

int _vector_iter_distance(iterator_t iter0, iterator_t iter1) {
    assert(_iter_is_vaild(iter0));
    assert(_iter_is_vaild(iter1));

    size_t dist = _ITER_VPTR(iter1)  - _ITER_VPTR(iter0);
    assert(dist %  _VECT_TYPE_SIZE(_ITER_CONTAIN_VECT(iter0)) == 0);
    return dist / _VECT_TYPE_SIZE(_ITER_CONTAIN_VECT(iter0));
}

void* _vector_iter_get_pointer(iterator_t iter) {
    assert(_iter_is_vaild(iter));

    return (void*)_ITER_VPTR(iter);
}

void  _vector_iter_get_value(iterator_t iter, void* elmp) {
    assert(elmp != NULL);
    assert(_iter_is_vaild(iter));
    assert(!iter_equal(iter, vector_end(_ITER_CONTAIN_VECT(iter))));

    _type_copy(_CTR_CTYPE(_ITER_CONTAIN_VECT(iter)), elmp, _ITER_VPTR(iter));
}

void  _vector_iter_set_value(iterator_t iter, void* elmp) {
    assert(elmp != NULL);
    assert(_iter_is_vaild(iter));
    assert(!iter_equal(iter, vector_end(_ITER_CONTAIN_VECT(iter))));

    _type_copy(_CTR_CTYPE(_ITER_CONTAIN_VECT(iter)), _ITER_VPTR(iter), elmp);
}

bool_t _vector_iter_is_vaild(iterator_t iter) {
    /* !! can't call iter_is_vaild here*/
    vector_t *vec = _ITER_CONTAIN_VECT(iter);
    RETURN_IF_NULL(vec, false);
    RETURN_IF(_ITER_VPTR(iter) < _VECT_DATA_BEGIN(vec) , false);
    RETURN_IF(_ITER_VPTR(iter) > _VECT_DATA_END(vec), false);
    return true;
}

/* not standard functions */
void _vector_swap_elem(vector_t* vec, int p1, int p2) {
    assert(vec != NULL);

    /* use vector's data memory to avoid alloc new memory*/
    vector_reserve(vec, _VECT_SIZE(vec) + 1);
    vector_reserve(vec, p1 + 1);
    vector_reserve(vec, p2 + 1);

    int typesize = _VECT_TYPE_SIZE(vec);
    int vecsize  = _VECT_SIZE(vec);
    memmove(_VECT_DATA_OFFSET(vec, vecsize), _VECT_DATA_OFFSET(vec, p1), typesize);
    memmove(_VECT_DATA_OFFSET(vec, p1), _VECT_DATA_OFFSET(vec, p2), typesize);
    memmove(_VECT_DATA_OFFSET(vec, p2), _VECT_DATA_OFFSET(vec, vecsize), typesize);
}

void _vector_copy_elem(vector_t* vec, int p1, int p2) {
    assert(vec != NULL);

    /* use vector's data memory to avoid alloc new memory*/
    vector_reserve(vec, _VECT_SIZE(vec) + 1);
    vector_reserve(vec, p1 + 1);
    vector_reserve(vec, p2 + 1);

    memmove(_VECT_DATA_OFFSET(vec, p1), _VECT_DATA_OFFSET(vec, p2), _VECT_TYPE_SIZE(vec));
}