#ifndef OHSTL_CLIST_H
#define OHSTL_CLIST_H

#ifdef __cplusplus
extern "C" {
#endif


#include <ohstl/citerator.h>

struct _tag_list_t;

typedef struct _tag_listnode_t {
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

#define list_new(x) _list_new(#x)
#define list_init(lst, x) _list_init((lst), #x)

list_t* _list_new(const char* typestr);
int _list_init(list_t*, const char*);

void list_destroy(list_t*);
void list_delete(list_t*);

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
#endif  //OHSTL_CLIST_H