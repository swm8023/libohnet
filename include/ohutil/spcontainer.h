#ifndef OHUTIL_SPCONTAINER_H
#define OHUTIL_SPCONTAINER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>


/* === simple list function */
#define SPLST_INITVAL NULL
#define SPLST_DEFNEXT(type) type *next;

#define FOR_SPLST_START(head, elm) \
    for ((elm) = (head); (elm); (elm) = (elm)->next) {
#define FOR_SPLST_END() }

#define splst_init(head)  ((head) = NULL)
#define splst_empty(head) ((head) == NULL)

#define splst_add(head, elm) do {   \
    (elm)->next = (head);           \
    (head) = (elm);                 \
} while (0)

#define splst_del(head, elm) do {   \
    typeof(head) *_spe = &head;     \
    for (;*_spe && *_spe != (elm);  \
        _spe = &(*_spe)->next);     \
    if(*_spe) *_spe = elm->next;    \
} while (0)

#define splst_len(head) ({                          \
    int _splen = 0;                                 \
    typeof(head) _spe = (head);                     \
    for (;(_spe); (_spe) = (_spe)->next, _splen++); \
    _splen;                                         \
})




#ifdef __cplusplus
}
#endif
#endif  //OHUTIL_SPCONTAINER_H