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

int main(int argc, char *argv[]) {
    
    srand((unsigned)time(NULL));
    int num = 100000, i;

    op_t *op;
    op = (op_t*)fn_malloc(num * sizeof(op_t));

    int size = 0;
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

    type_regist(op_t, _op_t_copy_func, NULL, NULL, NULL);
    // test c list
    fntime_t delta = get_time();
    list_t *lst = new_list(op_t);

    int ssize = 0;
    for (i = 0; i < num; i++) {
        switch(op[i].type) {
            case 0: list_push_front(lst, &op[i]); break;
            case 1: list_push_back(lst, &op[i]); break;
            case 2: if(!list_empty(lst))list_pop_front(lst); break;
            case 3: if(!list_empty(lst))list_pop_back(lst); break;
            case 4: list_insert(lst, iter_next_n(list_begin(lst), op[i].num), &op[i]);break;
            case 5: if(!list_empty(lst))
                list_erase(lst, iter_next_n(list_begin(lst), op[i].num));break;
        }
    }

    delta = get_time() - delta;
    printf("C List Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    //test c++ list
    delta = get_time();
    list<op_t> slist;
    for (i = 0; i < num; i++) {
        switch(op[i].type) {
            case 0: slist.push_front(op[i]); break;
            case 1: slist.push_back(op[i]); break;
            case 2: if(!slist.empty())slist.pop_front(); break;
            case 3: if(!slist.empty())slist.pop_back(); break;
            case 4: {
                list<op_t>::iterator it = slist.begin();
                advance(it, op[i].num);
                slist.insert(it, op[i]);
            }
            break;
            case 5: {
                list<op_t>::iterator it = slist.begin();
                advance(it, op[i].num);
                slist.erase(it);
            }
            break;
        }
    }
    delta = get_time() - delta;
    printf("STL List Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    // check
    iterator_t it0 = list_begin(lst);
    list<op_t>::iterator it1 = slist.begin();
    for (;iter_equal(list_end(lst), it0) && it1 != slist.end();
        it0 = iter_next(it0), it1++) {
        op_t opp;
        iter_get_value(it0, &opp);
    }

    // read time
    list_t *lst2 = new_list(int);
    list<int> sslist;
    list<int>::iterator it2;

    int pnum = 10000000;
    int *pnums = (int*)fn_malloc(sizeof(int) * pnum);
    int *xnums = (int*)fn_malloc(sizeof(int) * pnum);
    int *ynums = (int*)fn_malloc(sizeof(int) * pnum);

    for (i = 0; i < pnum; i++) pnums[i] = rand() % 1000;

    delta = get_time();
    for (i = 0; i < pnum; i++) {
        list_push_back(lst2, pnums[i]);
    }

    int ind = 0;
    for (it0 = list_begin(lst2); !iter_equal(list_end(lst2), it0); it0 = iter_next(it0)) {
        iter_get_value(it0, &xnums[ind++]);
    }
    delta = get_time() - delta;
    printf("C List Read Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    delta = get_time();
    for (i = 0; i < pnum; i++) {
        sslist.push_back(pnums[i]);
    }

    ind = 0;
    for (it2 = sslist.begin(); it2 != sslist.end(); it2++) {
        ynums[ind++] = *it2;
    }
    delta = get_time() - delta;
    printf("STL List Read Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);
    // for (i = 0 ;i < pnum; i++) {
    // }

	return 0;
}