#include <stdio.h>
#include <float.h>
#include <assert.h>

#include <fnstl/ctypes.h>
#include <fnstl/cvector.h>
#include <fnstl/clist.h>
#include <fnstl/cheap.h>
#include <fnstl/cpair.h>
#include <fnstl/cset.h>
#include <fnstl/cmap.h>

/* for mappint type name to standard name*/
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
    {"map_t",              "map_t"},
    {"pair_t",             "pair_t"},


    {"vector",             "vector_t"},
    {"list",               "list_t"},
    {"heap",               "heap_t"},
    {"set",                "set_t"},
    {"map",                "map_t"},
    {"pair",               "pair_t"},
    /* end with NULL*/
    {NULL, NULL},
};

/* for c-builtin varg copy*/
typedef void (*_cbuiltin_varg_copy)(void*, va_list);
#define _FUNC_NAME_CBUILTIN_VARG_COPY(type) _##type##_varg_copy
#define _FUNC_CBUILTIN_VARG_COPY(wtype, rtype)            \
static void _##wtype##_varg_copy(void *ret, va_list arg){ \
    *(wtype*)ret = (wtype)va_arg(arg, rtype);             \
}

_FUNC_CBUILTIN_VARG_COPY(int8_t, int32_t);
_FUNC_CBUILTIN_VARG_COPY(int16_t, int32_t);
_FUNC_CBUILTIN_VARG_COPY(int32_t, int32_t);
_FUNC_CBUILTIN_VARG_COPY(int64_t, int64_t);
_FUNC_CBUILTIN_VARG_COPY(uint8_t, int32_t);
_FUNC_CBUILTIN_VARG_COPY(uint16_t, int32_t);
_FUNC_CBUILTIN_VARG_COPY(uint32_t, uint32_t);
_FUNC_CBUILTIN_VARG_COPY(uint64_t, uint64_t);
_FUNC_CBUILTIN_VARG_COPY(float, double);
_FUNC_CBUILTIN_VARG_COPY(double, double);
_FUNC_CBUILTIN_VARG_COPY(pointer, pointer);

_cbuiltin_varg_copy _cbuiltin_varg_copy_funcs[11] = {
    /* the order is same as enum typeid */
    _FUNC_NAME_CBUILTIN_VARG_COPY(int8_t),
    _FUNC_NAME_CBUILTIN_VARG_COPY(int16_t),
    _FUNC_NAME_CBUILTIN_VARG_COPY(int32_t),
    _FUNC_NAME_CBUILTIN_VARG_COPY(int64_t),
    _FUNC_NAME_CBUILTIN_VARG_COPY(uint8_t),
    _FUNC_NAME_CBUILTIN_VARG_COPY(uint16_t),
    _FUNC_NAME_CBUILTIN_VARG_COPY(uint32_t),
    _FUNC_NAME_CBUILTIN_VARG_COPY(uint64_t),
    _FUNC_NAME_CBUILTIN_VARG_COPY(float),
    _FUNC_NAME_CBUILTIN_VARG_COPY(double),
    _FUNC_NAME_CBUILTIN_VARG_COPY(pointer),
};




/* for C-builtin functions */
#define _FUNC_CBUILTIN_INIT(type)                               \
static bool_t _##type##_init(void *t1, type_node *node) {       \
    return (*(type*)t1 = 0, true);                              \
}

#define _FUNC_CBUILTIN_DESTROY(type)                            \
static bool_t _##type##_destroy(void *t1) {               \
    return (*(type*)t1 = 0, true);                              \
}

#define _FUNC_CBUILTIN_LESS(type, val)                          \
static bool_t _##type##_less(const void* t1, const void* t2) {  \
    return *(type*)t1 - *(type*)t2 < (val) ? true : false;      \
}

#define _FUNC_CBUILTIN_COPY(type)                               \
static bool_t _##type##_copy(void* t1, const void* t2) {        \
    return (*(type*)t1 = *(type*)t2, true);                     \
}

#define _FUNC_CBUILTIN_ALL(type, val)   \
    _FUNC_CBUILTIN_INIT(type);          \
    _FUNC_CBUILTIN_DESTROY(type);       \
    _FUNC_CBUILTIN_LESS(type, val);     \
    _FUNC_CBUILTIN_COPY(type);


// /* == c-builtin type == */
_FUNC_CBUILTIN_ALL(int8_t,  0);
_FUNC_CBUILTIN_ALL(uint8_t, 0);
_FUNC_CBUILTIN_ALL(int16_t, 0);
_FUNC_CBUILTIN_ALL(uint16_t,0);
_FUNC_CBUILTIN_ALL(int32_t, 0);
_FUNC_CBUILTIN_ALL(uint32_t,0);
_FUNC_CBUILTIN_ALL(int64_t, 0);
_FUNC_CBUILTIN_ALL(uint64_t,0);
_FUNC_CBUILTIN_ALL(float,   -DBL_EPSILON);
_FUNC_CBUILTIN_ALL(double,  -FLT_EPSILON);
_FUNC_CBUILTIN_ALL(pointer, 0);

#define _CBUILTIN_TYEP_DECLARE(type)                              \
    {#type, _typeid_##type, sizeof(type), _TYPE_C,                \
        _##type##_copy, _##type##_less, _##type##_init, NULL}


/* == fn-builtin type == */
/* pair */
static bool_t _pair_t_copy(void* ptr1, const void* ptr2) {
    pair_assign((pair_t*)ptr1, (pair_t*)ptr2);
    return true;
}

static bool_t _pair_t_less(const void* ptr1, const void* ptr2) {
    return _pair_less((pair_t*)ptr1, (pair_t*)ptr2);
}

static bool_t _pair_t_init(void* ptr, type_node* ipr) {
    _pair_init_aux((pair_t*)ptr, ipr);
    return true;
}

static bool_t _pair_t_destroy(void* ptr) {
    _pair_destroy_aux((pair_t*)ptr);
    return true;
}

_type_t _builtin_types[] = {
    /* C-Builtin types */
    _CBUILTIN_TYEP_DECLARE(int8_t),
    _CBUILTIN_TYEP_DECLARE(uint8_t),
    _CBUILTIN_TYEP_DECLARE(int16_t),
    _CBUILTIN_TYEP_DECLARE(uint16_t),
    _CBUILTIN_TYEP_DECLARE(int32_t),
    _CBUILTIN_TYEP_DECLARE(uint32_t),
    _CBUILTIN_TYEP_DECLARE(int64_t),
    _CBUILTIN_TYEP_DECLARE(uint64_t),
    _CBUILTIN_TYEP_DECLARE(float),
    _CBUILTIN_TYEP_DECLARE(double),
    _CBUILTIN_TYEP_DECLARE(pointer),

    /* Fn-builtin types */
    {"vector_t", _typeid_vector_t, sizeof(vector_t), _TYPE_FN, NULL, NULL, NULL, NULL},
    {"list_t",   _typeid_list_t,   sizeof(list_t),   _TYPE_FN, NULL, NULL, NULL, NULL},
    {"heap_t",   _typeid_heap_t,   sizeof(heap_t),   _TYPE_FN, NULL, NULL, NULL, NULL},
    {"pair_t",   _typeid_pair_t,   sizeof(pair_t),   _TYPE_FN, _pair_t_copy, _pair_t_less, _pair_t_init, _pair_t_destroy},
    {"set_t",    _typeid_set_t,    sizeof(set_t),    _TYPE_FN, NULL, NULL, NULL, NULL},
    {"map_t",    _typeid_map_t,    sizeof(map_t),    _TYPE_FN, NULL, NULL, NULL, NULL},

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
    for (;_TYPE_NAME(elm) && strcmp (_TYPE_NAME(elm), type_name); elm++);
    RETURN_IF(_TYPE_NAME(elm), elm);

    _type_lst *elml = _userdef_types;
    for(;elml && strcmp(_TYPE_NAME(&elml->type), type_name); elml = elml->next);
    return elml ? &elml->type : NULL;
}

void _get_varg_value_bytype(_type_t *type, va_list varg, void *retp) {
    void *pointerp = NULL;
    switch(_TYPE_STYLE(type)) {
        case _TYPE_C:
            _cbuiltin_varg_copy_funcs[_TYPE_ID(type)](retp, varg);
            break;
        case _TYPE_FN:
        case _TYPE_USER:
            pointerp = va_arg(varg, void*);
            if (pointerp) {
                _type_copy(type, retp, pointerp);
            } else {
                memset(retp, 0, type->size);
            }
            break;
    }
}


void _type_regist(const char *name, size_t size,
    _tfunc_copy fcopy, _tfunc_less fless, _tfunc_init finit, _tfunc_destroy fdestroy) {
    _type_lst *type_new, *elml;
    /* already regist */
    FOR_SPLST_START(_userdef_types, elml)
        RETURN_IF(0 == strcmp(_TYPE_NAME(&elml->type), name))
    FOR_SPLST_END()

    /* same with builtin type */
    _type_t *elm = &_builtin_types[0];
    for (;_TYPE_NAME(elm) && strcmp (_TYPE_NAME(elm), name); elm++);
    RETURN_IF(_TYPE_NAME(elm));

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
    for(;elm && strcmp(_TYPE_NAME(&elm->type), name); elm = elm->next);
    if (elm) {
        splst_del(_userdef_types, elm);
        fn_free(elm);
    }
}

bool_t _type_copy(_type_t* type, void *t1, const void *t2) {
    assert(type != NULL);
    assert(t1 != NULL);
    assert(t2 != NULL);

    return type->copy ? type->copy(t1, t2) : true;
}

bool_t _type_less(_type_t* type, const void *t1, const void *t2) {
    assert(type != NULL);
    assert(t1 != NULL);
    assert(t2 != NULL);

    return type->less ? type->less(t1, t2) : true;
}

bool_t _type_init(_type_t* type, void *t1, type_node* node) {
    assert(type != NULL);
    assert(t1 != NULL);

    return type->init ? type->init(t1, node) : true;
}

bool_t _type_destroy(_type_t* type, void* t1) {
    assert(type != NULL);
    assert(t1 != NULL);

    return type->destroy ? type->destroy(t1) : true;
}

bool_t _type_equal(_type_t* type, const void *t1, const void *t2) {
    assert(type != NULL);
    assert(t1 != NULL);
    assert(t2 != NULL);

    return type->less ? (!type->less(t1, t2) && !type->less(t2, t1)): true;
}

bool_t _type_greater(_type_t* type, const void *t1, const void *t2) {
    assert(type != NULL);
    assert(t1 != NULL);
    assert(t2 != NULL);

    return type->less ? type->less(t2, t1) : true;
}


char* _get_type_node_find_part(char *strp) {
    int incomp_brace = 0;
    /* end while end of string OR a comma when no incomplete brackets */
    while (*strp && (incomp_brace || *strp != ',')) {
        if (*strp == '<') incomp_brace ++;
        if (*strp == '>') incomp_brace --;
        strp++;
    }
    return strp;
}

type_node* _get_type_node_recur(type_node *parent, char* str) {
    int index = 0, next = 1;
    type_node *node;
    char *strp = str;
    /* can be divided into 1~2 part, stop when strp point to the end of str */
    while (*(strp = _get_type_node_find_part(str)) || next) {
        /* strp point to comma or empty, stop when strp point to empty */
        next = (*strp ? 1 : 0);
        *strp = 0;
        /* if find, alloc and set type node */
        node = (type_node*)fn_malloc(sizeof(type_node));
        RETURN_IF_NULL(node, NULL);
        _TYPE_NODE_INIT(node, _get_type_bystr(str));

        assert(node->type != NULL);
        assert(index <= 1);

        parent->conttype[index++] = node;

        /* if fn-builtin type, go on */
        if (node->type->style == _TYPE_FN) {
            /* the last char must be '>', and start after 'type_t<'*/
            *(strp - 1) = 0;
            _get_type_node_recur(node, str + strlen(node->type->name) + 1);
        }
        /* break before update pointer str, because strp may be the end of str */
        if(next == 0) {
            break;
        }
        str = strp + 1;
    }
}


type_node* _get_type_node(_type_t *type, const char* str) {
    assert(type!= NULL);
    assert(type->style == _TYPE_FN);
    assert(str != NULL);

    char *rstr = (char*)fn_malloc(strlen(str) + 1), *rstrp;
    /* remove space */
    for (rstrp = rstr ; *str; str++) {
        if (*str != ' ') *rstrp++ = *str;
    }
    *rstrp = 0;

    type_node *root = (type_node*)fn_malloc(sizeof(type_node));
    RETURN_IF_NULL(root, NULL);

    _TYPE_NODE_INIT(root, type);
    _get_type_node_recur(root, rstr);

    fn_free(rstr);
    return root;
}

void _free_type_node(type_node *node) {
    if (node != NULL) {
        _free_type_node(node->conttype[0]);
        _free_type_node(node->conttype[1]);
        fn_free(node);
    }
}

void _type_node_debug(type_node* node, int level) {
    if (node == NULL) return;
    int i;
    for (i = 0; i < level; i++) printf("  ");
    printf("%s\n", node->type->name);
    _type_node_debug(node->conttype[0], level + 1);
    _type_node_debug(node->conttype[1], level + 1);
}