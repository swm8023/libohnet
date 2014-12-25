#ifndef FNUTIL_CVECTOR_H
#define FNUTIL_CVECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fnutil/ctypes.h>
#include <fnutil/util.h>
#include <fnutil/citerator.h>


#define VECTOR_INIT_CAPACITY 16

typedef struct _tag_vector_t {
    _CONTAINER_BASE;
	size_t _size;
	int    _capacity;
	char   *_data;
} vector_t;

#define _VECTOR_TYPE_SIZE(vec) _CTR_CTYPE_SIZE(vec)

#define _VECTOR_DATA_BEGIN(vec) vector_data(vec)
#define _VECTOR_DATA_END(vec) (vector_data(vec) + vector_size(vec) * _VECTOR_TYPE_SIZE(vec))

#define _VECTOR_SIZE(vec)        ((vec)->_size)
#define _VECTOR_CAPACITY(vec)    ((vec)->_capacity)
#define _VECTOR_DATA(vec)        ((vec)->_data)

#define _VECTOR_DATA_OFFSET(vec, of) ((vec)->_data + (of) * _VECTOR_TYPE_SIZE(vec))

#define _ITER_CONTAIN_VECTOR(iter) ((vector_t*)((iter)._container))

/* public functions */
#define new_vector(x) _new_vector(#x)
#define init_vector(vec, x) _init_vector((vec), #x)

vector_t* _new_vector(const char* typestr);
int _init_vector(vector_t*, const char*);

void destroy_vector(vector_t*);
void delete_vector(vector_t*);

#define vector_empty(vec)       (_VECTOR_SIZE(vec) == 0)
#define vector_size(vec)        _VECTOR_SIZE(vec)
#define vector_capacity(vec)    _VECTOR_CAPACITY(vec)
#define vector_data(vec)        _VECTOR_DATA(vec)


void vector_reserve(vector_t*, size_t);

void* vector_at(vector_t*, int);
void* vector_front(vector_t*);
void* vector_back(vector_t*);

void vector_push_back(vector_t*, ...);
void _vector_push_back_varg(vector_t *, va_list);
void vector_pop_back(vector_t*);
void vector_update(vector_t*, int, ...);

void vector_erase_pos(vector_t*, int);
void vector_insert_pos(vector_t*, int, ...);
void vector_clear(vector_t*);

iterator_t vector_insert(vector_t*, iterator_t, ...);
iterator_t vector_erase(vector_t*, iterator_t);
iterator_t vector_begin(vector_t*);
iterator_t vector_end(vector_t*);


/* not standard function */
void _vector_swap_elem(vector_t*, int, int );


#ifdef __cplusplus
}
#endif
#endif  //FNUTIL_CVECTOR_H