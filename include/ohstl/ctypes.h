#ifndef OHSTL_CTYPES_H
#define OHSTL_CTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include <ohutil/util.h>

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
    _typeid_pair_t,
    _typeid_string_t,


    /* user define type */
    _typeid_userdef,

    /* invaild type */
    _typeid_invaild,
} _type_id_t;

#define _TYPEID_CBUILTIN_START _typeid_int8_t
#define _TYPEID_CBUILTIN_END   _typeid_pointer

#define _TYPEID_FNBUILTIN_START _typeid_vector_t
#define _TYPEID_FNBUILTIN_END   _typeid_string_t

#define _VECT_TYPE_NAME   "vector_t"
#define _LIST_TYPE_NAME   "list_t"
#define _HEAP_TYPE_NAME   "heap_t"
#define _SET_TYPE_NAME    "set_t"
#define _MAP_TYPE_NAME    "map_t"
#define _PAIR_TYPE_NAME   "pair_t"
#define _STR_TYPE_NAME    "string_t"


/* define type tree, used to analyse nested types decalaration */
typedef struct _tag_type_t _type_t;
typedef struct _tag_type_node {
    _type_t *type;
    struct _tag_type_node *conttype[2];
} type_node;

#define _TYPE_NODE_INIT(node, tp) do { \
    (node)->type = (tp);               \
    (node)->conttype[0] = NULL;        \
    (node)->conttype[1] = NULL;        \
} while (0)

#define _TPNODE_TYPE(node)     ((node)->type)
#define _TPNODE_LCNODE(node)   ((node)->conttype[0])
#define _TPNODE_RCNODE(node)   ((node)->conttype[1])


/* callback function used to define a type */
typedef bool_t (*_tfunc_destroy)(void*);
typedef bool_t (*_tfunc_less)(const void*, const void*);
typedef bool_t (*_tfunc_init)(void*, type_node*);
typedef bool_t (*_tfunc_copy)(void*, const void*);
/* define struct type and type_list */
typedef struct _tag_type_t {
    const char *name;
    _type_id_t type_id;
    size_t size;
    _typestyle_t style;
    _tfunc_copy copy;
    _tfunc_less less;
    _tfunc_init init;
    _tfunc_destroy destroy;
} _type_t;

#define _TYPE_NAME(tp)          ((tp)->name)
#define _TYPE_ID(tp)            ((tp)->type_id)
#define _TYPE_SIZE(tp)          ((tp)->size)
#define _TYPE_STYLE(tp)         ((tp)->style)
#define _TYPE_FUNC_LESS(tp)     ((tp)->less)
#define _TYPE_FUNC_COPY(tp)     ((tp)->copy)
#define _TYPE_FUNC_INIT(tp)     ((tp)->init)
#define _TYPE_FUNC_DESTROY(tp)  ((tp)->destroy)

typedef struct _tag_type_t_lst {
    SPLST_DEFNEXT(struct _tag_type_t_lst)
    _type_t type;
} _type_lst;


typedef void* pointer;

extern _type_t    _builtin_types[];
extern _type_lst *_userdef_types;

/*  regist or unregist a user defined type */
#define type_regist(type, fcopy, fless, finit, fdestroy) \
    _type_regist(#type, sizeof(type), fcopy, fless, finit, fdestroy)
#define type_unregist(type) _type_unregist(#type)

void _type_unregist(const char*);
void _type_regist(const char *, size_t, _tfunc_copy, _tfunc_less, _tfunc_init, _tfunc_destroy);

/* get type tree by special string  */
type_node* _get_type_node(_type_t *, const char*);
void _free_type_node(type_node *);
void _type_node_debug(type_node*, int);

/* get type by special string */
_type_t* _get_type_bystr(const char*);

/* set value of special type from variable args */
void _get_varg_value_bytype(_type_t *, va_list, void *);

/* avoid check whether callback fucntion exist or not*/
bool_t _type_copy(_type_t*, void *, const void *);
bool_t _type_less(_type_t*, const void *, const void *);
bool_t _type_init(_type_t*, void *, type_node*);
bool_t _type_destroy(_type_t*, void*);

bool_t _type_greater(_type_t*, const void*, const void*);
bool_t _type_equal(_type_t*, const void*, const void*);




#ifdef __cplusplus
}
#endif
#endif  //OHSTL_CTYPES_H

