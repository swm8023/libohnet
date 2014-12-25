#include <stdio.h>


#include <fnutil/ctypes.h>
#include <fnutil/cvector.h>
#include <fnutil/clist.h>
#include <fnutil/cheap.h>



typedef struct _tag_type_text_mapping {
    const char *ori_text;
    const char *map_text;
} _type_text_mapping_t;

_type_text_mapping_t _builtin_type_text_mapping[] = {
    {"char",               "int8_t"},
    {"unsigned char",      "uint8_t"},
    {"int8_t",             "int8_t"},
    {"uint8_t",            "uint8_t"},
    {"short",              "int16_t"},
    {"unsigned short",     "uint16_t"},
    {"int16_t",            "int16_t"},
    {"uint16_t",           "uint16_t"},
    {"int",                "int32_t"},
    {"unsigned int",       "uint32_t"},
    {"int32_t",            "int32_t"},
    {"uint32_t",           "uint32_t"},
    {"long long",          "int64_t"},
    {"unsigned long long", "uint64_t"},
    {"int64_t",            "int64_t"},
    {"uint64_t",           "uint64_t"},
    {"float",              "float"},
    {"double",             "double"},
    {"pointer",            "pointer"},

    {"vector_t",           "vector_t"},
    {"list_t",             "list_t"},
    {"heap_t",             "heap_t"},
    {"set_t",              "set_t"},
    {"map_t",             "map_t"},

    /* end with NULL*/
    {NULL, NULL},
};


#define EPS 1e-9

#define _CTYPE_FUNC_COPY(type)                                  \
static bool_t _##type##_copy(const void* t1, const void* t2) {  \
    return (*(type*)t1 = *(type*)t2, true);                     \
}

#define _CTYPE_FUNC_INTLESS(type)                              \
static bool_t _##type##_less(const void* t1, const void* t2) {  \
    return *(type*)t1 < *(type*)t2 ? true : false;              \
}

#define _CTYPE_FUNC_DBLLESS(type)                              \
static bool_t _##type##_less(const void* t1, const void* t2) {  \
    return *(type*)t1 - *(type*)t2 < -EPS ? true : false;       \
}

#define _CTYPE_PROPERTY_DECLARE(type) \
   {#type, _typeid_##type, sizeof(type), _TYPE_C, _##type##_copy, _##type##_less, NULL, NULL}

_CTYPE_FUNC_COPY(int8_t);
_CTYPE_FUNC_COPY(uint8_t);
_CTYPE_FUNC_COPY(int16_t);
_CTYPE_FUNC_COPY(uint16_t);
_CTYPE_FUNC_COPY(int32_t);
_CTYPE_FUNC_COPY(uint32_t);
_CTYPE_FUNC_COPY(int64_t);
_CTYPE_FUNC_COPY(uint64_t);
_CTYPE_FUNC_COPY(float);
_CTYPE_FUNC_COPY(double);
_CTYPE_FUNC_INTLESS(int8_t);
_CTYPE_FUNC_INTLESS(uint8_t);
_CTYPE_FUNC_INTLESS(int16_t);
_CTYPE_FUNC_INTLESS(uint16_t);
_CTYPE_FUNC_INTLESS(int32_t);
_CTYPE_FUNC_INTLESS(uint32_t);
_CTYPE_FUNC_INTLESS(int64_t);
_CTYPE_FUNC_INTLESS(uint64_t);
_CTYPE_FUNC_DBLLESS(float);
_CTYPE_FUNC_DBLLESS(double);

_CTYPE_FUNC_COPY(pointer);
_CTYPE_FUNC_INTLESS(pointer);

_type_t _builtin_types[] = {
    /* C-Builtin types */
    _CTYPE_PROPERTY_DECLARE(int8_t),
    _CTYPE_PROPERTY_DECLARE(uint8_t),
    _CTYPE_PROPERTY_DECLARE(int16_t),
    _CTYPE_PROPERTY_DECLARE(uint16_t),
    _CTYPE_PROPERTY_DECLARE(int32_t),
    _CTYPE_PROPERTY_DECLARE(uint32_t),
    _CTYPE_PROPERTY_DECLARE(int64_t),
    _CTYPE_PROPERTY_DECLARE(uint64_t),
    _CTYPE_PROPERTY_DECLARE(float),
    _CTYPE_PROPERTY_DECLARE(double),
    _CTYPE_PROPERTY_DECLARE(pointer),

    /* Fn-builtin types */
    {"vector_t", _typeid_vector_t, sizeof(vector_t), _TYPE_FN, NULL, NULL, NULL, NULL},
    {"list_t",   _typeid_list_t,   sizeof(list_t),   _TYPE_FN, NULL, NULL, NULL, NULL},
    {"heap_t",   _typeid_heap_t,   sizeof(heap_t),   _TYPE_FN, NULL, NULL, NULL, NULL},
    //{"set_t",    _typeid_set_t,    sizeof(set_t),    _TYPE_FN, NULL, NULL, NULL, NULL},
    //{"map_t",    _typeid_map_t,    sizeof(map_t),    _TYPE_FN, NULL, NULL, NULL, NULL},
    /* End with NULL*/
    {NULL, _typeid_invaild, 0, 0, NULL, NULL, NULL, NULL},
};

_type_lst *_userdef_types = NULL;


_type_t* _get_type_bystr(const char* typestr) {
    /**
     * int int32_t pointer
     * vector_t vector_t<int>
     * userdef
     */
    char type_name[_TYPE_NAME_SIZE];
    memset(type_name, 0, sizeof type_name);
    int typelen = strlen(typestr);
    char *dem_pos = NULL;
    /* end with '*', must be a pointer*/
    if (typestr[typelen - 1] == '*') {
        strcpy(type_name, "pointer");
    /* find a character '<', must be fn-butin type */
    } else if (NULL != (dem_pos = strchr(typestr, '<'))){
        strncpy(type_name, typestr, dem_pos - typestr);
    /* find  type text map */
    } else {
        _type_text_mapping_t *elm = &_builtin_type_text_mapping[0];
        for (;elm->ori_text && strcmp(elm->ori_text, typestr); elm++);
        if (elm->map_text) {
            strcpy(type_name, elm->map_text);
        /* not found, may be a user define type */
        } else {
            strcpy(type_name, typestr);
        }
    }

    /* find _type_t pointer, builtin -> uer defined -> NULL */
    _type_t *elm = &_builtin_types[0];
    for (;elm->name && strcmp (elm->name, type_name); elm++);
    RETURN_IF(elm->name, elm);

    _type_lst *elml = _userdef_types;
    for(;elml && strcmp(elml->type.name, type_name); elml = elml->next);
    return elml ? &elml->type : NULL;

}

void _get_varg_value_bytype(_type_t *type, va_list varg, void *p_ret) {
    void *p_pointer = NULL;
    switch(type->type_id) {
        case _typeid_int8_t:
            *(int8_t*)p_ret  = (int8_t)va_arg(varg, int32_t);
            break;
        case _typeid_uint8_t:
            *(uint8_t*)p_ret = (uint8_t)va_arg(varg, int32_t);
            break;
        case _typeid_int16_t:
            *(int16_t*)p_ret = (int16_t)va_arg(varg, int32_t);
            break;
        case _typeid_uint16_t:
            *(uint16_t*)p_ret= (uint16_t)va_arg(varg, int32_t);
            break;
        case _typeid_int32_t:
            *(int32_t*)p_ret = (int32_t)va_arg(varg, int32_t);
            break;
        case _typeid_uint32_t:
            *(uint32_t*)p_ret= (uint32_t)va_arg(varg, uint32_t);
            break;
        case _typeid_int64_t:
            *(int64_t*)p_ret = (int64_t)va_arg(varg, int64_t);
            break;
        case _typeid_uint64_t:
            *(uint64_t*)p_ret= (uint64_t)va_arg(varg, uint64_t);
            break;
        case _typeid_float:
            *(float*)p_ret   = (float)va_arg(varg, double);
            break;
        case _typeid_double:
            *(double*)p_ret  = (double)va_arg(varg, double);
            break;
        case _typeid_pointer:
            *(pointer*)p_ret = (pointer)va_arg(varg, pointer);
            break;
        case _typeid_invaild:
            // p_ret = NULL;
            break;
        /* fn-builtin type or user-define type, call copy function */
        default:
            p_pointer = va_arg(varg, void*);
            if (p_pointer && type->copy) {
                type->copy(p_ret, p_pointer);
            } else {
                memset(p_ret, 0, type->size);
            }
            break;
    }
}


void _type_regist(const char *name, size_t size,
    _tfunc2 fcopy, _tfunc2 fless, _tfunc1 finit, _tfunc1 fdestroy) {
    _type_lst *type_new, *elml;
    /* already regist */
    FOR_SPLST_START(_userdef_types, elml)
        RETURN_IF(0 == strcmp(elml->type.name, name))
    FOR_SPLST_END()

    /* same with builtin type */
    _type_t *elm = &_builtin_types[0];
    for (;elm->name && strcmp (elm->name, name); elm++);
    RETURN_IF(elm->name);

    /* insert new type to userdef_type list */
    type_new = (_type_lst*)fn_malloc(sizeof(_type_lst));
    type_new->type.name = name;
    type_new->type.size = size;
    type_new->type.style = _TYPE_USER;
    type_new->type.copy = fcopy;
    type_new->type.less = fless;
    type_new->type.init = finit;
    type_new->type.destroy = fdestroy;
    type_new->type.type_id = _typeid_userdef;
    splst_add(_userdef_types, type_new);
}

void _type_unregist(const char *name) {
    _type_lst *elm = _userdef_types;
    for(;elm && strcmp(elm->type.name, name); elm = elm->next);
    if (elm) {
        splst_del(_userdef_types, elm);
        fn_free(elm);
    }
}

bool_t _type_copy(_type_t* type, const void *t1, const void *t2) {
    return type->copy ? type->copy(t1, t2) : true;
}

bool_t _type_less(_type_t* type, const void *t1, const void *t2) {
    return type->less ? type->less(t1, t2) : true;
}

bool_t _type_init(_type_t* type, const void *t1) {
    return type->init ? type->init(t1) : true;
}

bool_t _type_destroy(_type_t* type, const void* t1) {
    return type->destroy ? type->destroy(t1) : true;
}