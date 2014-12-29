#ifndef FNUTIL_CPAIR_H
#define FNUTIL_CPAIR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fnutil/util.h>
#include <fnutil/ctypes.h>
#include <fnutil/cvector.h>
#include <fnutil/citerator.h>


typedef struct _tag_pair_t {
    type_node *tpnode;
    void *ptr[2];
} pair_t;

#define PAIR_LEFT  0
#define PAIR_RIGHT 1

#define _PAIR_STNODE(pr)      _CTR_STNODE((pr))
#define _PAIR_CTNODE(pr, ind) (_PAIR_STNODE((pr))->conttype[(ind)])
#define _PAIR_CTYPE(pr, ind)  (_PAIR_CTNODE((pr), (ind))->type)
#define _PAIR_CPTR(pr, ind)   ((pr)->ptr[(ind)])

#define PAIR_LPTR(pr)  ((pr)->ptr[0])
#define PAIR_RPTR(Pr)  ((pr)->ptr[1])

#define new_pair(...) _new_pair(#__VA_ARGS__)
#define init_pair(pr, ...) _init_pair(pr, #__VA_ARGS__)



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

pair_t* _new_pair(const char*);
int _init_pair(pair_t*, const char*);
int _pair_init_params(pair_t*, const type_node*);

void destroy_pair(pair_t*);
void delete_pair(pair_t*);

void pair_assign(pair_t *pr1, pair_t *pr2);
bool_t pair_less(pair_t *pt1, pair_t *pr2);

/* private function */
void* _pair_get(pair_t*, int);
void  _pair_make_elem(pair_t*, int, ...);
void  _pair_get_value(pair_t*, int, void*);



#ifdef __cplusplus
}
#endif
#endif  //FNUTIL_CPAIR_H

