#ifndef OHSTL_CVECTOR_H
#define OHSTL_CVECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ohstl/citerator.h>


#define _VECT_INIT_CAPACITY 32

typedef struct _tag_vector_t {
    _CONTAINER_BASE;
	size_t size;
	int    capacity;
	char   *data;
} vector_t;

#define _VECT_TYPE_SIZE(vec) _CTR_CTYPE_SIZE(vec)

#define _VECT_SIZE(vec)             ((vec)->size)
#define _VECT_CAPACITY(vec)         ((vec)->capacity)
#define _VECT_DATA(vec)             ((vec)->data)
#define _VECT_DATA_OFFSET(vec, of)  (_VECT_DATA(vec) + (of) * _VECT_TYPE_SIZE(vec))
#define _VECT_DATA_BEGIN(vec)       _VECT_DATA(vec)
#define _VECT_DATA_END(vec)         (_VECT_DATA_OFFSET(vec, _VECT_SIZE(vec)))

#define _ITER_CONTAIN_VECT(iter)    ((vector_t*)((iter).container))

/* public functions */
#define vector_new(x) _vector_new(#x)
#define vector_init(vec, x) _vector_init((vec), #x)

vector_t* _vector_new(const char* typestr);
int _vector_init(vector_t*, const char*);

void vector_destroy(vector_t*);
void vector_delete(vector_t*);

#define vector_empty(vec)       (_VECT_SIZE(vec) == 0)
#define vector_size(vec)        _VECT_SIZE(vec)
#define vector_capacity(vec)    _VECT_CAPACITY(vec)
#define vector_data(vec)        _VECT_DATA(vec)

void vector_reserve(vector_t*, size_t);

void* vector_at(vector_t*, int);
void* vector_front(vector_t*);
void* vector_back(vector_t*);

void vector_push_back(vector_t*, ...);
void vector_pop_back(vector_t*);
void vector_update(vector_t*, int, ...);

void vector_erase_pos(vector_t*, int);
void vector_insert_pos(vector_t*, int, ...);
void vector_clear(vector_t*);

void _vector_insert_varg(vector_t *, char*, va_list);
void _vector_erase(vector_t *, char *);

iterator_t vector_insert(vector_t*, iterator_t, ...);
iterator_t vector_erase(vector_t*, iterator_t);
iterator_t vector_begin(vector_t*);
iterator_t vector_end(vector_t*);


/* not standard function */
void _vector_swap_elem(vector_t*, int, int);
void _vector_copy_elem(vector_t*, int, int);


#ifdef __cplusplus
}
#endif
#endif  //OHSTL_CVECTOR_H