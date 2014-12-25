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

static void _list_append_node(list_t*, _listnode_t *, _listnode_t *);
static void _list_erase_node(list_t *, _listnode_t*);

list_t* _new_list(const char* typestr) {
    assert(typestr != NULL);

    list_t* lst = (list_t*)fn_malloc(sizeof(list_t));
    RETURN_IF_NULL(lst, NULL);

    _CTR_STYPE(lst)  = _get_type_bystr(_LIST_TYPE_NAME);
    _CTR_CTYPE(lst)  = _get_type_bystr(typestr);
    _CTR_ITERIF(lst) = &_default_list_iter_if;

    lst->_size = 0;
    lst->_guard = (_listnode_t*)fn_malloc(_LISTNODE_SIZE(_CTR_CTYPE(lst)));
    if (lst->_guard == NULL) {
        fn_free(lst);
        return NULL;
    }
    lst->_guard->_lst  = lst;
    lst->_guard->_pre  = lst->_guard;
    lst->_guard->_next = lst->_guard;
    return lst;
}

void delete_list(list_t* lst) {
    assert(lst != NULL);

    list_clear(lst);
    fn_free(lst->_guard);
    fn_free(lst);
}

void* list_front(list_t* lst) {
    assert(lst != NULL);

    RETURN_IF(_LIST_SIZE(lst) == 0, NULL);
    return _LISTNODE_DATAP(_LIST_GUARD(lst)->_next);
}

void* list_back(list_t* lst) {
    assert(lst != NULL);

    RETURN_IF(_LIST_SIZE(lst) == 0, NULL);
    return _LISTNODE_DATAP(_LIST_GUARD(lst)->_pre);
}

void list_push_back(list_t* lst, ...) {
    va_list elm_arg;
    _listnode_t *newnode;

    assert(lst != NULL);

    /* allocate new node */
    newnode = (_listnode_t*)fn_malloc(_LISTNODE_SIZE(_CTR_CTYPE(lst)));
    RETURN_IF_NULL(newnode);

    /* append new node to tail, guard->pre is the last element */
    _list_append_node(lst, _LIST_GUARD(lst)->_pre, newnode);

    /* set new node's attr and data */
    newnode->_lst = lst;
    va_start(elm_arg, lst);
    _get_varg_value_bytype(_CTR_CTYPE(lst), elm_arg, _LISTNODE_DATAP(newnode));
    va_end(elm_arg);
}


void list_push_front(list_t* lst, ...) {
    va_list elm_arg;
    _listnode_t *newnode;

    assert(lst != NULL);
    /* allocate new node */
    newnode = (_listnode_t*)fn_malloc(_LISTNODE_SIZE(_CTR_CTYPE(lst)));
    RETURN_IF_NULL(newnode);

    /* add new node before head, guard->next is the first element */
    _list_append_node(lst, _LIST_GUARD(lst), newnode);

    /* set new node's attr and data */
    newnode->_lst = lst;
    va_start(elm_arg, lst);
    _get_varg_value_bytype(_CTR_CTYPE(lst), elm_arg, _LISTNODE_DATAP(newnode));
    va_end(elm_arg);
}

void list_pop_back(list_t* lst) {
    assert(lst != NULL);

    _list_erase_node(lst, _LIST_GUARD(lst)->_pre);
}

void list_pop_front(list_t* lst) {
    assert(lst != NULL);

    _list_erase_node(lst, _LIST_GUARD(lst)->_next);
}


void list_clear(list_t* lst) {
    assert(lst != NULL);

    for (; !list_empty(lst); list_pop_front(lst));
}


void _list_append_node(list_t *lst, _listnode_t *node_p, _listnode_t *newnode) {
    assert(lst != NULL);
    assert(newnode != NULL);
    assert(node_p  != NULL);

    newnode->_pre  = node_p;
    newnode->_next = node_p->_next;
    newnode->_next->_pre = newnode;
    newnode->_pre->_next = newnode;

    lst->_size++;
}

void _list_erase_node(list_t *lst, _listnode_t *node) {
    assert(lst != NULL);
    assert(node != NULL);
    assert(node != _LIST_GUARD(lst));

    /* delete node and free node */
    node->_pre->_next = node->_next;
    node->_next->_pre = node->_pre;

    lst->_size--;

    /* free item memory */
    _type_destroy(_CTR_CTYPE(lst), _LISTNODE_DATAP(node));
    fn_free(node);
}

iterator_t list_begin(list_t* lst) {
    iterator_t iter;

    assert(lst != NULL);

    _ITER_INIT(iter);
    _ITER_CONTAINER(iter) = lst;
    _ITER_LPTR(iter) = (char*)(_LIST_GUARD(lst)->_next);
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
    va_list elm_arg;
    _listnode_t *newnode;

    assert(_iter_is_vaild(iter));
    assert(lst != NULL);

    newnode = (_listnode_t*)fn_malloc(_LISTNODE_SIZE(_CTR_CTYPE(lst)));
    if (newnode == NULL) {
        return iter;
    }
    /* insert element before iterator postion */
    _list_append_node(lst, ((_listnode_t*)_ITER_LPTR(iter))->_pre, newnode);

    newnode->_lst = lst;
    va_start(elm_arg, iter);
    _get_varg_value_bytype(_CTR_CTYPE(lst), elm_arg, _LISTNODE_DATAP(newnode));
    va_end(elm_arg);

    /* iterator's pointer point to the first newly insert element */
    _ITER_LPTR(iter) = (char*)newnode;
    return iter;

}

iterator_t list_erase(list_t* lst, iterator_t iter) {
    _listnode_t *eranode;
    assert(_iter_is_vaild(iter));
    assert(lst != NULL);

    /* iterator point to the node after erase node */
    eranode = ((_listnode_t*)_ITER_LPTR(iter));
    _ITER_LPTR(iter) = (char*)(eranode->_next);

    _list_erase_node(lst, eranode);

    return iter;
}

/* list iterator functions */

iterator_t _list_iter_next(iterator_t iter) {
    assert(_iter_is_vaild(iter));
    assert(!iter_equal(iter, list_end(_ITER_CONTAIN_LIST(iter))));

    _ITER_LPTR(iter) = (char*)((_listnode_t*)_ITER_LPTR(iter))->_next;
    return iter;
}

iterator_t _list_iter_pre(iterator_t iter) {
    assert(_iter_is_vaild(iter));
    assert(!iter_equal(iter, list_begin(_ITER_CONTAIN_LIST(iter))));

    _ITER_LPTR(iter) = (char*)((_listnode_t*)_ITER_LPTR(iter))->_pre;
    return iter;
}

iterator_t _list_iter_next_n(iterator_t iter, int step) {
    /* not used, instead by iterator_advance */
    list_t *lst = _ITER_CONTAIN_LIST(iter);

    /* optimize , often start from begin */
    if (iter_equal(iter, list_begin(lst)) && step > list_size(lst)) {
        return iter_advance(list_end(lst), step - list_size(lst));
    }
    return iter_advance(iter, step);
}

iterator_t _list_iter_pre_n(iterator_t iter, int step) {
    /* not used, instead by iterator_advance */
    return iter_advance(iter, - step);
}

bool_t _list_iter_equal(iterator_t iter0, iterator_t iter1) {
    assert(_iter_is_vaild(iter0));
    assert(_iter_is_vaild(iter1));
    assert(_iter_is_same(iter0, iter1));

    return _ITER_LPTR(iter0) == _ITER_LPTR(iter1) ? true : false;
}

int   _list_iter_distance(iterator_t iter0, iterator_t iter1) {
    int dist;
    iterator_t iteru;
    list_t *lst;

    assert(_iter_is_vaild(iter0));
    assert(_iter_is_vaild(iter0));
    assert(_iter_is_same(iter0, iter1));

    lst = _ITER_CONTAIN_LIST(iter0);
    /* guess iter0 < iter1 first */
    dist = 0;
    iteru = iter0;
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
    list_t *lst;
    _listnode_t *node;

    /* !! can't call iter_is_vaild here*/
    lst = _ITER_CONTAIN_LIST(iter);
    for (node = lst->_guard->_next;
        node != lst->_guard; node = node->_next) {
        if ((char*)node == _ITER_LPTR(iter))
            return true;
    }
    return node == lst->_guard ? true : false;
}
