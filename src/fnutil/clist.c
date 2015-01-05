#include <stddef.h>
#include <assert.h>
#include <fnutil/clist.h>

/* list  iterator interface */

static iterator_t _list_iter_next(iterator_t);
static iterator_t _list_iter_pre(iterator_t);
static iterator_t _list_iter_next_n(iterator_t, int);
static iterator_t _list_iter_pre_n(iterator_t, int);
static bool_t _list_iter_equal(iterator_t, iterator_t);
static int   _list_iter_distance(iterator_t, iterator_t);
static void* _list_iter_get_pointer(iterator_t);
static void  _list_iter_get_value(iterator_t, void*);
static void  _list_iter_set_value(iterator_t, void*);
static bool_t _list_iter_is_vaild(iterator_t);

static _iterator_if _default_list_iter_if = {
    _list_iter_next,
    _list_iter_pre,
    _list_iter_next_n,
    _list_iter_pre_n,
    _list_iter_equal,
    _list_iter_distance,
    _list_iter_get_pointer,
    _list_iter_get_value,
    _list_iter_set_value,

    _list_iter_is_vaild,
};

/* list operation functions */



list_t* _new_list(const char* typestr) {
    assert(typestr != NULL);

    list_t* lst = (list_t*)fn_malloc(sizeof(list_t));
    RETURN_IF_NULL(lst, NULL);

    if (_init_list(lst, typestr) < 0) {
        fn_free(lst);
        return NULL;
    }

    return lst;
}

int _init_list(list_t* lst, const char* typestr) {
    assert(lst != NULL);
    assert(typestr != NULL);

    /* init container base attr */
    _CTR_INIT(lst, _get_type_bystr(_LIST_TYPE_NAME), typestr, &_default_list_iter_if);

    /* list attr*/
    lst->size = 0;
    lst->guard = (_listnode_t*)fn_malloc(_LISTNODE_SIZE(_CTR_CTYPE(lst)));
    if (lst->guard == NULL) {
        fn_free(lst);
        return -1;
    }
    //lst->guard->lst  = lst;
    lst->guard->pre  = lst->guard;
    lst->guard->next = lst->guard;
}

void destroy_list(list_t* lst) {
    assert(lst != NULL);

    list_clear(lst);
    fn_free(lst->guard);

    _CTR_DESTROY(lst);
}

void delete_list(list_t* lst) {
    assert(lst != NULL);

    destroy_list(lst);
    fn_free(lst);
}

void* list_front(list_t* lst) {
    assert(lst != NULL);

    RETURN_IF(_LIST_SIZE(lst) == 0, NULL);
    return _LISTNODE_DATAP(_LIST_GUARD(lst)->next);
}

void* list_back(list_t* lst) {
    assert(lst != NULL);

    RETURN_IF(_LIST_SIZE(lst) == 0, NULL);
    return _LISTNODE_DATAP(_LIST_GUARD(lst)->pre);
}

void list_push_back(list_t* lst, ...) {
    va_list arg;

    assert(lst != NULL);

    /* append new node to tail, guard->pre is the last element */
    va_start(arg, lst);
    _list_append_node_varg(lst, _LIST_GUARD(lst)->pre, arg);
    va_end(arg);
}


void list_push_front(list_t* lst, ...) {
    va_list arg;

    assert(lst != NULL);

    /* add new node before head, guard->next is the first element */
    va_start(arg, lst);
    _list_append_node_varg(lst, _LIST_GUARD(lst), arg);
    va_end(arg);
}

void list_pop_back(list_t* lst) {
    assert(lst != NULL);

    _list_erase_node(lst, _LIST_GUARD(lst)->pre);
}

void list_pop_front(list_t* lst) {
    assert(lst != NULL);

    _list_erase_node(lst, _LIST_GUARD(lst)->next);
}


void list_clear(list_t* lst) {
    assert(lst != NULL);

    for (; !list_empty(lst); list_pop_front(lst));
}


void _list_append_node_varg(list_t *lst, _listnode_t *prenode, va_list arg) {
    assert(lst != NULL);
    assert(prenode  != NULL);

    /* allocate newnode memory */
    _listnode_t *newnode = (_listnode_t*)fn_malloc(_LISTNODE_SIZE(_CTR_CTYPE(lst)));
    RETURN_IF_NULL(newnode);

    /* set attrbution of newnode*/
    //newnode->lst = lst;
    newnode->pre  = prenode;
    newnode->next = prenode->next;
    newnode->next->pre = newnode;
    newnode->pre->next = newnode;

    /* init and copy from arg */
    _type_init(_CTR_CTYPE(lst), _LISTNODE_DATAP(newnode), _CTR_CTNODE(lst));
    _get_varg_value_bytype(_CTR_CTYPE(lst), arg, _LISTNODE_DATAP(newnode));

    lst->size++;
}

void _list_erase_node(list_t *lst, _listnode_t *node) {
    assert(lst != NULL);
    assert(node != _LIST_GUARD(lst));

    /* delete the node */
    node->pre->next = node->next;
    node->next->pre = node->pre;

    /* destroy and free memory */
    _type_destroy(_CTR_CTYPE(lst), _LISTNODE_DATAP(node));
    fn_free(node);

    lst->size--;
}

iterator_t list_begin(list_t* lst) {
    iterator_t iter;

    assert(lst != NULL);

    _ITER_INIT(iter);
    _ITER_CONTAINER(iter) = lst;
    _ITER_LPTR(iter) = (char*)(_LIST_GUARD(lst)->next);
    return iter;
}

iterator_t list_end(list_t* lst) {
    iterator_t iter;

    assert(lst != NULL);

    _ITER_INIT(iter);
    _ITER_CONTAINER(iter) = lst;
    _ITER_LPTR(iter) = (char*)_LIST_GUARD(lst);
    return iter;
}

iterator_t list_insert(list_t* lst, iterator_t iter, ...) {
    va_list arg;

    assert(_iter_is_vaild(iter));
    assert(lst != NULL);

    /* the node which iterator point will be the next node of newnode*/
    va_start(arg, iter);
    _list_append_node_varg(lst, ((_listnode_t*)_ITER_LPTR(iter))->pre, arg);
    va_end(arg);

    /* iterator point the first inserted node */
    _ITER_LPTR(iter) = (char*)(((_listnode_t*)_ITER_LPTR(iter))->pre);
    return iter;

}

iterator_t list_erase(list_t* lst, iterator_t iter) {
    _listnode_t *eranode;
    assert(_iter_is_vaild(iter));
    assert(lst != NULL);

    /* iterator point to the node after erase node */
    eranode = ((_listnode_t*)_ITER_LPTR(iter));
    _ITER_LPTR(iter) = (char*)(eranode->next);

    _list_erase_node(lst, eranode);

    return iter;
}

/* list iterator functions */

iterator_t _list_iter_next(iterator_t iter) {
    assert(_iter_is_vaild(iter));
    assert(!iter_equal(iter, list_end(_ITER_CONTAIN_LIST(iter))));

    _ITER_LPTR(iter) = (char*)((_listnode_t*)_ITER_LPTR(iter))->next;
    return iter;
}

iterator_t _list_iter_pre(iterator_t iter) {
    assert(_iter_is_vaild(iter));
    assert(!iter_equal(iter, list_begin(_ITER_CONTAIN_LIST(iter))));

    _ITER_LPTR(iter) = (char*)((_listnode_t*)_ITER_LPTR(iter))->pre;
    return iter;
}

iterator_t _list_iter_next_n(iterator_t iter, int step) {
    assert(_iter_is_vaild(iter));

    list_t *lst = _ITER_CONTAIN_LIST(iter);

    /* optimize , often start from begin */
    if (iter_equal(iter, list_begin(lst)) && step > list_size(lst) - step) {
        step = step - list_size(lst);
    }

    _listnode_t *node = (_listnode_t*)_ITER_LPTR(iter);
    if (step > 0) {
        for (; step && node != _LIST_GUARD(lst); node = node->next, step--);
    } else if (step < 0) {
        for (; step && node != _LIST_GUARD(lst)->next; node = node->pre, step++);
    }

    assert(step == 0);

    _ITER_LPTR(iter) = (char*)node;
    return iter;
}

iterator_t _list_iter_pre_n(iterator_t iter, int step) {
    /* not used, instead by iterator_advance */
    return _list_iter_next_n(iter, -step);
}

bool_t _list_iter_equal(iterator_t iter0, iterator_t iter1) {
    assert(_iter_is_vaild(iter0));
    assert(_iter_is_vaild(iter1));
    assert(_iter_is_same(iter0, iter1));

    return _ITER_LPTR(iter0) == _ITER_LPTR(iter1) ? true : false;
}

int   _list_iter_distance(iterator_t iter0, iterator_t iter1) {
    assert(_iter_is_vaild(iter0));
    assert(_iter_is_vaild(iter0));
    assert(_iter_is_same(iter0, iter1));

    list_t *lst = _ITER_CONTAIN_LIST(iter0);
    /* guess iter0 < iter1 first */
    int dist = 0;
    iterator_t iteru = iter0;
    while (!iter_equal(iteru, list_end(lst)) && !iter_equal(iter1, iteru)) {
        dist++;
        iteru = iter_next(iteru);
    }
    RETURN_IF(iter_equal(iteru, iter1), dist);

    /* failed, iter1 < iter0 */
    dist = 0;
    iteru = iter1;
    while (!iter_equal(iteru, list_end(lst)) && !iter_equal(iter0, iteru)) {
        dist++;
        iteru = iter_next(iteru);
    }

    /* must be equal */
    assert(iter_equal(iteru, iter0));

    return -dist;
}

void* _list_iter_get_pointer(iterator_t iter) {
    assert(_iter_is_vaild(iter));

    return _LISTNODE_DATAP((_listnode_t*)_ITER_LPTR(iter));
}

void  _list_iter_get_value(iterator_t iter, void* elmp) {
    assert(elmp != NULL);
    assert(_iter_is_vaild(iter));
    assert(!iter_equal(iter, list_end(_ITER_CONTAIN_LIST(iter))));

    _type_copy(_CTR_CTYPE(_ITER_CONTAIN_LIST(iter)), elmp,
        _LISTNODE_DATAP((_listnode_t*)_ITER_LPTR(iter)));
}

void  _list_iter_set_value(iterator_t iter, void* elmp) {
    assert(elmp != NULL);
    assert(_iter_is_vaild(iter));
    assert(!iter_equal(iter, list_end(_ITER_CONTAIN_LIST(iter))));

    _type_copy(_CTR_CTYPE(_ITER_CONTAIN_LIST(iter)),
        _LISTNODE_DATAP((_listnode_t*)_ITER_LPTR(iter)), elmp);
}

bool_t _list_iter_is_vaild(iterator_t iter) {
    /* !! can't call iter_is_vaild here*/
    list_t *lst = _ITER_CONTAIN_LIST(iter);
    _listnode_t *node;
    return true;
    // for (node = lst->guard->next;
    //     node != lst->guard; node = node->next) {
    //     if ((char*)node == _ITER_LPTR(iter))
    //         return true;
    // }
    // return node == lst->guard ? true : false;
}
