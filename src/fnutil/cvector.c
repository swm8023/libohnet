#include <stdio.h>
#include <assert.h>
#include <fnutil/cvector.h>


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
vector_t* _new_vector(const char* typestr) {
    assert(typestr != NULL);

    vector_t* vec = (vector_t*)fn_malloc(sizeof(vector_t));
    RETURN_IF_NULL(vec, NULL);

    if (_init_vector(vec, typestr) < 0) {
        fn_free(vec);
        return NULL;
    }

    return vec;
}

int _init_vector(vector_t* vec, const char* typestr) {
    assert(vec != NULL);
    assert(typestr != NULL);

    /* container base attr*/
    _CTR_STYPE(vec)  = _get_type_bystr(_VECTOR_TYPE_NAME);
    _CTR_CTYPE(vec)  = _get_type_bystr(typestr);
    _CTR_ITERIF(vec) = &_default_vector_iter_if;

    /* check if type rigth*/
    assert(_CTR_STYPE(vec) != NULL);
    assert(_CTR_CTYPE(vec) != NULL);

    /* vector attr */
    vec->_capacity   = VECTOR_INIT_CAPACITY;
    vec->_size       = 0;
    vec->_data       = (char*)fn_malloc(_VECTOR_CAPACITY(vec) * _VECTOR_TYPE_SIZE(vec));

    if (vec->_data == NULL) {
        return -1;
    }


    return 0;
}

void vector_reserve(vector_t* vec, size_t newcap) {
    assert (vec != NULL);

    /* if vec->capactity >= newcap, just return */
    RETURN_IF(_VECTOR_CAPACITY(vec) >= newcap);

    while (_VECTOR_CAPACITY(vec) < newcap)
        vec->_capacity *= 2;
    vec->_data = fn_realloc(vec->_data, _VECTOR_CAPACITY(vec) * _VECTOR_TYPE_SIZE(vec));
}

void destroy_vector(vector_t* vec) {
    assert(vec != NULL);

    vector_clear(vec);
    if (vec->_data) fn_free(vec->_data);
}

void delete_vector(vector_t* vec) {
    assert(vec != NULL);

    destroy_vector(vec);
    fn_free(vec);
}

void* vector_at(vector_t* vec, int pos) {
    assert(vec != NULL);
    assert(pos >= 0 && pos < _VECTOR_SIZE(vec));

    return _VECTOR_DATA_OFFSET(vec, pos);
}

void* vector_front(vector_t* vec) {
    assert(vec != NULL);

    RETURN_IF(_VECTOR_SIZE(vec) == 0, NULL);
    return vector_at(vec, 0);
}

void* vector_back(vector_t* vec) {
    assert(vec != NULL);

    RETURN_IF(_VECTOR_SIZE(vec) == 0, NULL);
    return vector_at(vec, _VECTOR_SIZE(vec) - 1);
}

void vector_push_back(vector_t* vec, ...) {
    va_list elm_arg;

    assert(vec != NULL);

    va_start(elm_arg, vec);
    _vector_push_back_varg(vec, elm_arg);
    va_end(elm_arg);
}

void _vector_push_back_varg(vector_t *vec, va_list elm_arg) {
    char *ins_ptr;

    assert(vec != NULL);

    vector_reserve(vec, _VECTOR_SIZE(vec) + 1);

    ins_ptr = _VECTOR_DATA_OFFSET(vec, _VECTOR_SIZE(vec));
    _get_varg_value_bytype(_CTR_CTYPE(vec), elm_arg, ins_ptr);
    vec->_size++;
}

void vector_pop_back(vector_t* vec) {
    vector_erase_pos(vec, _VECTOR_SIZE(vec)- 1);
}

void vector_update(vector_t* vec, int pos, ...) {
    va_list elm_arg;
    char *upd_ptr;

    assert(vec != NULL);
    assert(pos >= 0 && pos < _VECTOR_SIZE(vec));

    /* destroy old element and set new element*/
    upd_ptr = _VECTOR_DATA_OFFSET(vec, pos);
    _type_destroy(_CTR_CTYPE(vec), upd_ptr);

    va_start(elm_arg, pos);
    _get_varg_value_bytype(_CTR_CTYPE(vec), elm_arg, upd_ptr);
    va_end(elm_arg);
}

void vector_insert_pos(vector_t* vec, int pos, ...) {
    va_list elm_arg;
    int typesize;
    char *ins_ptr;

    assert(vec != NULL);
    assert(pos >= 0 && pos <= _VECTOR_SIZE(vec));

    vector_reserve(vec, _VECTOR_SIZE(vec) + 1);

    /* leave space to put element */
    typesize = _VECTOR_TYPE_SIZE(vec);
    ins_ptr =  _VECTOR_DATA_OFFSET(vec, pos);
    memmove(ins_ptr + typesize, ins_ptr, _VECTOR_DATA_END(vec) - ins_ptr);

    /* insert element */
    va_start(elm_arg, pos);
    _get_varg_value_bytype(_CTR_CTYPE(vec), elm_arg, ins_ptr);
    va_end(elm_arg);
    vec->_size++;
}

void vector_erase_pos(vector_t* vec, int pos) {
    int typesize;
    char *era_ptr;

    assert(vec != NULL);
    assert(pos >= 0 && pos < _VECTOR_SIZE(vec));

    /* destroy and remove element */
    typesize = _VECTOR_TYPE_SIZE(vec);
    era_ptr = _VECTOR_DATA_OFFSET(vec, pos);
    _type_destroy(_CTR_CTYPE(vec), era_ptr);
    memmove(era_ptr, era_ptr + typesize, _VECTOR_DATA_END(vec) - era_ptr - typesize);
    vec->_size--;
}

void vector_clear(vector_t* vec) {
    while (!vector_empty(vec))
        vector_pop_back(vec);
}

iterator_t vector_insert(vector_t* vec, iterator_t iter, ...) {
    va_list elm_arg;
    char *old_data = NULL;
    char *ins_ptr   = NULL;
    int  typesize  = 0;

    assert(vec != NULL);
    assert(_iter_is_vaild(iter));

    /* iterator maybe invaild after vector_reserve*/
    old_data = _VECTOR_DATA(vec);
    vector_reserve(vec, _VECTOR_SIZE(vec) + 1);
    _ITER_VPTR(iter) = _VECTOR_DATA(vec) + (_ITER_VPTR(iter) - old_data);

    /* leave space to put element */
    typesize = _VECTOR_TYPE_SIZE(vec);
    ins_ptr   = _ITER_VPTR(iter);
    memmove(ins_ptr + typesize, ins_ptr, _VECTOR_DATA_END(vec) - ins_ptr);

    /* insert element */
    va_start(elm_arg, iter);
    _get_varg_value_bytype(_CTR_CTYPE(vec), elm_arg, ins_ptr);
    va_end(elm_arg);
    vec->_size++;

    /* iterator pos not changed */
    return iter;
}


iterator_t vector_erase(vector_t* vec, iterator_t iter) {
    va_list elm_arg;
    char *era_ptr   = NULL;
    int  typesize  = 0;

    assert(vec != NULL);
    assert(_iter_is_vaild(iter));
    assert(!iter_equal(iter, vector_end(_ITER_CONTAIN_VECTOR(iter))));

    /* destroy and remove element */
    typesize = _VECTOR_TYPE_SIZE(vec);
    era_ptr   = _ITER_VPTR(iter);
    _type_destroy(_CTR_CTYPE(vec), era_ptr);
    memmove(era_ptr, era_ptr + typesize, _VECTOR_DATA_END(vec) - era_ptr - typesize);
    vec->_size--;

    /* iterator pos not changed */
    return iter;
}

iterator_t vector_begin(vector_t* vec) {
    iterator_t iter;

    assert(vec != NULL);

    _ITER_INIT(iter);
    _ITER_CONTAINER(iter) = vec;
    _ITER_VPTR(iter) = _VECTOR_DATA_BEGIN(vec);
    return iter;
}

iterator_t vector_end(vector_t* vec) {
    iterator_t iter;

    assert(vec != NULL);

    _ITER_INIT(iter);
    _ITER_CONTAINER(iter) = vec;
    _ITER_VPTR(iter) = _VECTOR_DATA_END(vec);
    return iter;
}


/* vector iterator functions */
iterator_t _vector_iter_next(iterator_t iter) {
    assert(_iter_is_vaild(iter));

    _ITER_VPTR(iter) += _VECTOR_TYPE_SIZE(_ITER_CONTAIN_VECTOR(iter));
    /* check if iterator vaild after change */
    assert(_iter_is_vaild(iter));
    return iter;
}

iterator_t _vector_iter_pre(iterator_t iter) {
    assert(_iter_is_vaild(iter));

    _ITER_VPTR(iter) -= _VECTOR_TYPE_SIZE(_ITER_CONTAIN_VECTOR(iter));
    /* check if iterator vaild after change */
    assert(_iter_is_vaild(iter));
    return iter;
}

iterator_t _vector_iter_next_n(iterator_t iter, int step) {
    assert(_iter_is_vaild(iter));

    _ITER_VPTR(iter) += step * _VECTOR_TYPE_SIZE(_ITER_CONTAIN_VECTOR(iter));
    /* check if iterator vaild after change */
    assert(_iter_is_vaild(iter));
    return iter;
}

iterator_t _vector_iter_pre_n(iterator_t iter, int step) {
    assert(_iter_is_vaild(iter));

    _ITER_VPTR(iter) -= step * _VECTOR_TYPE_SIZE(_ITER_CONTAIN_VECTOR(iter));
    /* check if iterator vaild after change */
    assert(_iter_is_vaild(iter));
    return iter;
}

bool_t _vector_iter_equal(iterator_t iter0, iterator_t iter1) {
    assert(_iter_is_vaild(iter0));
    assert(_iter_is_vaild(iter1));

    return _ITER_VPTR(iter0)  == _ITER_VPTR(iter1) ? true : false;
}

int   _vector_iter_distance(iterator_t iter0, iterator_t iter1) {
    assert(_iter_is_vaild(iter0));
    assert(_iter_is_vaild(iter1));

    size_t delta = _ITER_VPTR(iter1)  - _ITER_VPTR(iter0);
    assert(delta %  _VECTOR_TYPE_SIZE(_ITER_CONTAIN_VECTOR(iter0)) == 0);
    return delta / _VECTOR_TYPE_SIZE(_ITER_CONTAIN_VECTOR(iter0));
}

void* _vector_iter_get_pointer(iterator_t iter) {
    assert(_iter_is_vaild(iter));

    return (void*)_ITER_VPTR(iter);
}

void  _vector_iter_get_value(iterator_t iter, void* elmp) {
    assert(elmp != NULL);
    assert(_iter_is_vaild(iter));
    assert(!iter_equal(iter, vector_end(_ITER_CONTAIN_VECTOR(iter))));

    _type_copy(_CTR_CTYPE(_ITER_CONTAIN_VECTOR(iter)), elmp, _ITER_VPTR(iter));
}

void  _vector_iter_set_value(iterator_t iter, void* elmp) {
    assert(elmp != NULL);
    assert(_iter_is_vaild(iter));
    assert(!iter_equal(iter, vector_end(_ITER_CONTAIN_VECTOR(iter))));

    _type_copy(_CTR_CTYPE(_ITER_CONTAIN_VECTOR(iter)), _ITER_VPTR(iter), elmp);
}

bool_t _vector_iter_is_vaild(iterator_t iter) {
    /* !! can't call iter_is_vaild here*/
    vector_t *vec = _ITER_CONTAIN_VECTOR(iter);
    RETURN_IF_NULL(vec, false);
    RETURN_IF(_ITER_VPTR(iter) < _VECTOR_DATA_BEGIN(vec) , false);
    RETURN_IF(_ITER_VPTR(iter) > _VECTOR_DATA_END(vec), false);
    return true;
}

/* not standard function */
void _vector_swap_elem(vector_t* vec, int p1, int p2) {
    int typesize, vecsize;

    assert(vec != NULL);

    /* use vector's data memory to avoid alloc new memory*/

    vector_reserve(vec, _VECTOR_SIZE(vec) + 1);
    vector_reserve(vec, p1 + 1);
    vector_reserve(vec, p2 + 1);

    typesize = _VECTOR_TYPE_SIZE(vec);
    vecsize  = _VECTOR_SIZE(vec);
    memmove(_VECTOR_DATA_OFFSET(vec, vecsize), _VECTOR_DATA_OFFSET(vec, p1), typesize);
    memmove(_VECTOR_DATA_OFFSET(vec, p1), _VECTOR_DATA_OFFSET(vec, p2), typesize);
    memmove(_VECTOR_DATA_OFFSET(vec, p2), _VECTOR_DATA_OFFSET(vec, vecsize), typesize);
}
