#include <fn.h>
#include <list>
#include <stdio.h>
#include <assert.h>
using namespace std;

typedef struct _op_t {long long type; int num;} op_t;

bool_t _op_t_copy_func(const void* t1, const void* t2) {
    memcpy((void*)t1, (void*)t2, sizeof(op_t));
    return true;
}

bool_t _op_t_less_func(const void *t1, const void* t2) {
    op_t *o1 = (op_t*)t1;
    op_t *o2 = (op_t*)t2;
    return o1->type > o2->type || o1->type == o2->type && o1->num > o2->num;
}

#define ggg(x) #x
#define fff(x) ggg(x)
int main(int argc, char *argv[]) {

    map_t *mp = map_new(int, double);

    map_put(mp, 1, 3.4);
    map_put(mp, 2, 5.77);
    map_put(mp, 10, 4.6);
    map_put(mp, -1, 23.0);
    map_put(mp, 5, -4.3);
    map_put(mp, 3, 5.0);

    iterator_t it = map_begin(mp);
    while (!iter_equal(it, map_end(mp))) {
        pair_t *pr = (pair_t*)iter_get_pointer(it);
        printf("{%d, %f}\n", *(int*)pair_first(pr), *(double*)pair_second(pr));

        it = iter_next(it);
    }
    printf("\n");

    map_put(mp, 2, 17.0);
    map_erase(mp, map_find(mp, 10));

    it = map_begin(mp);
    while (!iter_equal(it, map_end(mp))) {
        pair_t *pr = (pair_t*)iter_get_pointer(it);
        printf("{%d, %f}\n", *(int*)pair_first(pr), *(double*)pair_second(pr));

        it = iter_next(it);
    }

    map_delete(mp);



    fn_memdbg_print_rec();
	return 0;
}