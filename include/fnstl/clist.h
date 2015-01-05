#ifndef FNUTIL_CLIST_H
#define FNUTIL_CLIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fnstl/ctypes.h>
#include <fnstl/util.h>
#include <fnstl/citerator.h>


struct _tag_list_t;

typedef struct _tag_listnode_t {
    //struct _tag_list_t* lst;
    struct _tag_listnode_t* pre;
    struct _tag_listnode_t* next;
    char data[0];
} _listnode_t;

typedef struct _tag_list_t {
    _CONTAINER_BASE;
    size_t size;
    _listnode_t *guard;
} list_t;

#define _LIST_TYPE_SIZE(lst) _CTR_CTYPE_SIZE((lst))
#define _LIST_SIZE(lst) ((lst)->size)
#define _LIST_GUARD(lst) ((lst)->guard)

#define _LISTNODE_DATAP(node_p) ((char*)(node_p) + offsetof(_listnode_t, data))
#define _LISTNDDE_BEGINP(data_p) (container_of((data_p), _listnode_t, data))
#define _LISTNODE_SIZE(type) (sizeof(_listnode_t) + (type)->size)

#define _ITER_CONTAIN_LIST(iter) ((list_t*)((iter).container))

#define new_list(x) _new_list(#x)
#define init_list(lst, x) _init_list((lst), #x)

list_t* _new_list(const char* typestr);
int _init_list(list_t*, const char*);

void destroy_list(list_t*);
void delete_list(list_t*);

#define list_empty(lst) (_LIST_SIZE(lst) == 0)
#define list_size(lst)  _LIST_SIZE(lst)

void* list_front(list_t*);
void* list_back(list_t*);

void list_push_back(list_t*, ...);
void list_push_front(list_t*, ...);
void list_pop_back(list_t*);
void list_pop_front(list_t*);

void list_clear(list_t*);

void _list_append_node_varg(list_t*, _listnode_t *, va_list);
void _list_erase_node(list_t *, _listnode_t*);

// void list_remove(list_t*, ...);
// void list_remove_if(list_t*, );
// void list_merge(list_t*, list_t*);
// void list_reverse(list_t*);

iterator_t list_begin(list_t*);
iterator_t list_end(list_t*);
iterator_t list_insert(list_t*, iterator_t, ...);
iterator_t list_erase(list_t*, iterator_t);



#ifdef __cplusplus
}
#endif
#endif  //FNUTIL_CLIST_H