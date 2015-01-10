#ifndef OHSTL_CPAIR_H
#define OHSTL_CPAIR_H

#ifdef __cplusplus
extern "C" {
#endif


#include <ohstl/citerator.h>


typedef struct _tag_pair_t {
    _alloc_if *allocif;
    type_node *tpnode;
    void *ptr[2];
} pair_t;

#define _PAIR_STNODE(pr)      _CTR_STNODE((pr))
#define _PAIR_CTNODE(pr, ind) (_PAIR_STNODE((pr))->conttype[(ind)])
#define _PAIR_CTYPE(pr, ind)  (_PAIR_CTNODE((pr), (ind))->type)
#define _PAIR_CPTR(pr, ind)   ((pr)->ptr[(ind)])

#define _PAIR_LPTR(pr)  ((pr)->ptr[0])
#define _PAIR_RPTR(pr)  ((pr)->ptr[1])

#define pair_new(...) _pair_new(#__VA_ARGS__)
#define pair_init(pr, ...) _pair_init(pr, #__VA_ARGS__)



#define pair_make(pr, el1, el2) do {    \
    _pair_make_elem((pr), 0, (el1));    \
    _pair_make_elem((pr), 1, (el2));    \
} while(0)

#define pair_value(pr, pel1, pel2) do { \
    _pair_get_value((pr), 0, (pel1));   \
    _pair_get_value((pr), 1, (pel2));   \
} while(0)

#define pair_first_value(pr, pel)       \
    _pair_get_value((pr), 0, (pel))

#define pair_second_value(pr, pel)      \
    _pair_get_value((pr), 1, (pel))

#define pair_first(pr)  _pair_get(pr, 0)
#define pair_second(pr) _pair_get(pr, 1)

pair_t* _pair_new(const char*);
int _pair_init(pair_t*, const char*);

void pair_destroy(pair_t*);
void pair_delete(pair_t*);

void _pair_destroy_aux(pair_t*);
void _pair_init_aux(pair_t*, type_node *);

void pair_assign(pair_t *,const pair_t *);
bool_t _pair_less(const pair_t *,const pair_t *);
bool_t _pair_key_less(const pair_t *, const pair_t *);

/* private function */
void* _pair_get(pair_t*, int);
void  _pair_make_elem(pair_t*, int, ...);
void  _pair_get_value(pair_t*, int, void*);



#ifdef __cplusplus
}
#endif
#endif  //OHSTL_CPAIR_H

