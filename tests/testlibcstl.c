#include <stdio.h>
#include <fnstl/util.h>
#include <cstl/cmap.h>
#include <cstl/cqueue.h>
#include <cstl/cset.h>

typedef struct _op_t {long long type; int num;} op_t;

static bool_t _op_t_copy_func(const void* t1, const void* t2, void* pv_output) {
    memcpy((void*)t1, (void*)t2, sizeof(op_t));
    *(bool_t*)pv_output = true;
}

int main(int argc, char *argv[]) {


    /* MAP */
    int i;
    int num = 5000000, base = 1000;
    int *x = (int*)malloc(num*sizeof(4));
    for (i = 0; i < base; i++) x[i] = i;
    for (i = base - 1; i >= 0; i--) {
        int pos = rand() % (i + 1);
        fnswap(x[i], x[pos]);
    }

    fntime_t delta = get_time();
    map_t *mp = create_map(int, int);
    pair_t *pr = create_pair(int, int);
    map_init(mp);
    pair_init(pr);

    for (i = 0; i < num; i++) {
        pair_make(pr, x[i], x[i]);
        map_insert(mp, pr);;
    }
    delta = get_time() - delta;
    printf("CSTL Map Insert Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    delta = get_time();
    for (i = 0; i < num; i++) {
        map_erase(mp, x[i]);
    }
    delta = get_time() - delta;
    printf("CSTL Map erase Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    free(x);
    map_destroy(mp);

    

    /* SET */
    num = 5000000;
    x = (int*)malloc(num*sizeof(4));
    for (i = 0; i < base; i++) x[i] = i;
    for (i = base - 1; i >= 0; i--) {
        int pos = rand() % (i + 1);
        fnswap(x[i], x[pos]);
    }
    for (i = base; i < num; i ++) {
        x[i] = x[i-base] + base;
    }


    set_t *st = create_set(int);
    set_init(st);

    delta = get_time();
    for (i = 0; i < num; i++) {
        set_insert(st, x[i]);
    }
    delta = get_time() - delta;
    printf("CSTL Set Insert Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    free(x);
    set_destroy(st);

    /* VECTOR */
    num = 200000;

    op_t *op = (op_t*)malloc(num * sizeof(op_t));

    type_register(op_t, NULL, _op_t_copy_func, NULL, NULL);
    int size = 0;
    for(i = 0; i < num; i++) {
        op[i].type = i < num/10 ? 0 : rand()%4;
        switch(op[i].type) {
            // push_back
            case 0: size++, op[i].num = rand()%100; break;
            // pop_back
            case 1: size > 0 ? size-- : 0; break;
            // insert
            case 2: size++; op[i].num = rand() % size; break;
            // erase
            case 3: size > 0 ? size-- : 0; op[i].num = rand() % size; break;
        }
    }

    // test c vector
    delta = get_time();
    vector_t *vec = create_vector(op_t);
    vector_init(vec);

    for (i = 0; i < num; i++) {
        switch(op[i].type) {
            case 0: vector_push_back(vec, &op[i]); break;
            //case 1: vector_pop_back(vec); break;
            case 2: vector_insert_n(vec, iterator_next_n(vector_begin(vec), op[i].num), 1, &op[i]); break;
            case 3: vector_erase(vec, iterator_next_n(vector_begin(vec), op[i].num)); break;
            // case 2: vector_insert_pos(vec, op[i].num, &op[i]); break;
            // case 3: vector_erase_pos(vec, op[i].num); break;
        }
    }
    delta = get_time() - delta;
    printf("CSTL Vector Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    free(op);
    vector_destroy(vec);

    /* LIST */
    num = 100000;

    op = (op_t*)malloc(num * sizeof(op_t));

    size = 0;
    for(i = 0; i < num; i++) {
        op[i].type = i < num/10 ? 0 : rand()%6;
        switch(op[i].type) {
            // push_front
            case 0: size++, op[i].num = rand()%100; break;
            // push_back
            case 1: size++, op[i].num = rand()%100; break;
            // pop_front
            case 2: if (size > 0)  size-- ; break;
            // pop_back
            case 3: if (size > 0)  size-- ; break;
            // insert
            case 4: size++; op[i].num = rand() % size; break;
            // erase
            case 5: if (size > 0) size--, op[i].num = rand() % size; break;
        }
    }

    // test c list
    delta = get_time();
    list_t *lst = create_list(op_t);
    list_init(lst);

    int ssize = 0;
    for (i = 0; i < num; i++) {
        switch(op[i].type) {
            case 0: list_push_front(lst, &op[i]); break;
            case 1: list_push_back(lst, &op[i]); break;
            case 2: if(!list_empty(lst))list_pop_front(lst); break;
            case 3: if(!list_empty(lst))list_pop_back(lst); break;
            case 4: list_insert(lst, iterator_next_n(list_begin(lst), op[i].num), &op[i]);break;
            case 5: if(!list_empty(lst))
                list_erase(lst, iterator_next_n(list_begin(lst), op[i].num));break;
        }
    }

    delta = get_time() - delta;
    printf("CSTL List Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    list_destroy(lst);
    free(op);



    /* HEAP */
    num = 10000000;
    op = (op_t*)malloc(num * sizeof(op_t));

    size = 0;
    for(i = 0; i < num; i++) {
        op[i].type = i < num/3 ? 0 : rand()%2;
        switch(op[i].type) {
            // push
            case 0: op[i].num = (long long)rand()*rand()%1000000; break;
            // pop
            case 1: break;
        }
    }

    delta = get_time();
    priority_queue_t *q = create_priority_queue(int);
    priority_queue_init(q);

    for (i = 0; i < num; i++) {
        switch(op[i].type) {
            case 0: priority_queue_push(q, op[i].num); break;
            case 1: if(!priority_queue_empty(q))priority_queue_pop(q); break;
        }
    }

    delta = get_time() - delta;

    printf("CSTL Heap Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);
    
    free(op);
    priority_queue_destroy(q);

}
