#ifndef FNUTIL_CTYPES_H
#define FNUTIL_CTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdint.h>

#include <fnutil/util.h>

#define _TYPE_NAME_SIZE 255


typedef enum _tag_typestyle_t {
    _TYPE_INVAILD = 0,
    _TYPE_C,
    _TYPE_FN,
    _TYPE_USER,
} _typestyle_t;

typedef enum _tag_typeid_t {
    /* c-builtin type */

    _typeid_int8_t = 0,
    _typeid_int16_t,
    _typeid_int32_t,
    _typeid_int64_t,
    _typeid_uint8_t,
    _typeid_uint16_t,
    _typeid_uint32_t,
    _typeid_uint64_t,
    _typeid_float,
    _typeid_double,
    _typeid_pointer,

    /* fn-builtin type*/
    _typeid_vector_t,
    _typeid_list_t,
    _typeid_heap_t,
    _typeid_set_t,
    _typeid_map_t,


    /* user define type */
    _typeid_userdef,

    /* invaild type */
    _typeid_invaild,
} _type_id_t;

#define _TYPEID_CBUILTIN_START _typeid_int8_t
#define _TYPEID_CBUILTIN_END   _typeid_pointer

#define _TYPEID_FNBUILTIN_START _typeid_vector_t
#define _TYPEID_FNBUILTIN_END   _typeid_list_t

#define _VECTOR_TYPE_NAME "vector_t"
#define _LIST_TYPE_NAME   "list_t"
#define _HEAP_TYPE_NAME   "heap_t"
#define _SET_TYPE_NAME    "set_t"
#define _MAP_TYPE_NAME    "map_t"





typedef bool_t (*_tfunc2)(const void*, const void*);
typedef bool_t (*_tfunc1)(const void*);

typedef struct _tag_type_t {
    const char *name;
    _type_id_t type_id;
    size_t size;
    _typestyle_t style;
    bool_t (*copy)(const void*, const void*);
    bool_t (*less)(const void*, const void*);
    bool_t (*init)(const void*);
    bool_t (*destroy)(const void*);
} _type_t;

typedef struct _tag_type_t_lst {
    SPLST_DEFNEXT(struct _tag_type_t_lst)
    _type_t type;
} _type_lst;

typedef void* pointer;


extern _type_t    _builtin_types[];
extern _type_lst *_userdef_types;

/* public functions */
#define type_regist(type, fcopy, fless, finit, fdestroy) \
    _type_regist(#type, sizeof(type), fcopy, fless, finit, fdestroy)
#define type_unregist(type) _type_unregist(#type)

/* private functions */
void _type_unregist(const char*);
void _type_regist(const char *, size_t, _tfunc2, _tfunc2, _tfunc1, _tfunc1);

_type_t* _get_type_bystr(const char*);
void _get_varg_value_bytype(_type_t *, va_list, void *);

bool_t _type_copy(_type_t*, const void *, const void *);
bool_t _type_less(_type_t*, const void *, const void *);
bool_t _type_init(_type_t*, const void *);
bool_t _type_destroy(_type_t*, const void*);


#ifdef __cplusplus
}
#endif
#endif  //FNUTIL_CTYPES_H