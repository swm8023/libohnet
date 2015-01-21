#ifndef OHUTIL_SPCONTAINER_H
#define OHUTIL_SPCONTAINER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>


/* === simple list function === */
#define SPLST_INITVAL NULL
#define SPLST_DEFNEXT(type) type *next;

#define for_splst_each(head, elm)  \
    for ((elm) = (head); (elm); (elm) = (elm)->next)


#define splst_init(head)  ((head) = NULL)
#define splst_empty(head) ((head) == NULL)

#define splst_add(head, elm) do {   \
    (elm)->next = (head);           \
    (head) = (elm);                 \
} while (0)

#define splst_del(head, elm) do {   \
    typeof(head) *_spe = &(head);   \
    for (;*_spe && *_spe != (elm);  \
        _spe = &(*_spe)->next);     \
    if(*_spe) *_spe = (elm)->next;  \
} while (0)

#define splst_len(head) ({                          \
    int _splen = 0;                                 \
    typeof(head) _spe = (head);                     \
    for (;(_spe); (_spe) = (_spe)->next, _splen++); \
    _splen;                                         \
})

/* === simple array function ===*/
#define expand_multi_two(x) ((x) ? ((x) << 1) : ((x) | 1))
#define expand_add_one(x)   ((x)  + 1)
#define init_array_zero(x, size) memset((x), 0, (size) * (sizeof((x)[0])))
#define init_array_none(x, size)
#define check_and_expand_array(x, size, need, expand, init) do {    \
    if ((need) > (size)) {                                          \
        int osize_ = (size);                                        \
        int need_  = (need);                                        \
        while ((size) < (need_)) (size) = expand((size));            \
        (x) = (typeof(x[0])*)ohrealloc(x, (size) * (sizeof((x)[0]))); \
        init((x) + (osize_), (size) - (osize_));                    \
    }                                                               \
} while (0);


/* === simple heap function */
/* bit operation */
#define RSHIFT(x) ((x) >> 1)
#define LSHIFT(x) ((x) << 1)
#define LCHILD(x) LSHIFT(x)
#define RCHILD(x) (LSHIFT(x)|1)


/* min heap (root=1)*/
#define spheap_update_pos(heap, pos_, npos_)               \
    ((heap)[pos_]->heap_pos = (npos_))

#define spheap_fix_up(heap, pos, size, comp) do {          \
    int pos_ = (pos);                                      \
    typeof((heap)[pos_]) val_ =  (heap)[pos_];             \
    while (pos_ > 1 && comp(val_, (heap)[RSHIFT(pos_)])) { \
        (heap)[pos_] = (heap)[RSHIFT(pos_)];               \
        spheap_update_pos(heap, pos_, pos_);               \
        pos_ = RSHIFT(pos_);                               \
    }                                                      \
    (heap)[pos_] = val_;                                   \
    spheap_update_pos(heap, pos_, pos_);                   \
} while (0)

#define spheap_fix_down(heap, pos, size, comp) do {        \
    int pos_ = (pos);                                      \
    typeof((heap)[pos_]) val_ = (heap)[pos_];              \
    while (LCHILD(pos_) <= (size)) {                       \
        int npos_ = LCHILD(pos_);                          \
        npos_ += (RCHILD(pos_) <= (size) &&                \
            comp((heap)[npos_+1], (heap)[npos_]) ? 1 : 0); \
        if (comp(val_, (heap)[npos_]))                     \
            break;                                         \
        (heap)[pos_] = (heap)[npos_];                      \
        spheap_update_pos(heap, pos_, pos_);               \
        pos_ = npos_;                                      \
    }                                                      \
    (heap)[pos_] = val_;                                   \
    spheap_update_pos(heap, pos_, pos_);                   \
} while (0)

/* here init from  size is  for setting spheap_pos */
#define spheap_init(heap, size, comp) do {                 \
    int ind_ = (size) /*/ 2 */+ 1;                         \
    while (--ind_)                                         \
        spheap_fix_down(heap, ind_, size, comp);           \
} while (0)

#define spheap_sort(heap, size, comp) do {                 \
    int size_ = (size);                                    \
    while (size_ > 0) {                                    \
        typeof((heap)[1]) tmp_ = (heap)[1];                \
        (heap)[1] = (heap[size_]);                         \
        (heap)[size_--] = tmp_;                            \
        spheap_fix_down(heap, 1, size_, comp);             \
    }                                                      \
} while (0)

/* here pos will be updated, so use ps_ = pos */
#define spheap_delete(heap, pos, size, comp) do {         \
    int ps_ = pos;                                        \
    spheap_update_pos(heap, ps_, 0);                      \
    (heap)[ps_] = (heap)[(size)--];                       \
    if ((size) == (ps_) - 1) break;                       \
    if (ps_ > 1 && comp((heap)[ps_],(heap)[RSHIFT(ps_)])){\
        spheap_fix_up(heap, ps_, size, comp);             \
    } else {                                              \
        spheap_fix_down(heap, ps_, size, comp);           \
    }                                                     \
} while (0)

#define spheap_push(heap, size, elm, comp) do {           \
    (heap)[++(size)] = (elm);                             \
    spheap_fix_up(heap, size, size, comp);                \
} while (0)

#define spheap_pop(heap, size, comp)                      \
    spheap_delete(heap, 1, size, comp)




#ifdef __cplusplus
}
#endif
#endif  //OHUTIL_SPCONTAINER_H