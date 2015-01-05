#include <fn.h>
#include <stdio.h>
#include <check.h>

#include "fntest.h"

#include <vector>
#include <list>
#include <queue>
using namespace std;


START_TEST(test_newheap)
{
    heap_t *hp, hp2;

    hp = new_heap(int);
    ck_assert_str_eq(_HEAP_CTYPE(hp)->name, "int32_t");
    delete_heap(hp);

    init_heap(&hp2, int);
    ck_assert_str_eq(_HEAP_CTYPE(&hp2)->name, "int32_t");
    destroy_heap(&hp2);

    ck_assert_int_eq(fn_memdbg_get_recnum(), 0);
}
END_TEST

START_TEST(test_heapop_cbuiltin)
{
    heap_t *hp;
    int i = 0, val;
    hp = new_heap(int);
    ck_assert_int_eq(heap_empty(hp), true);
    ck_assert_int_eq(heap_size(hp), 0);

    int arr0[] = {3, 2, 4, 1, 7, 9, 8, 5, 6, 0};
    int arr1[] = {3, 2, 2, 1, 1, 1, 1, 1, 1, 0};
    int arr2[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int arr3[] = {3, 3, 4, 4, 7, 9, 9, 9, 9, 9};
    int arr4[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

    for (i = 0; i < 10; i++) {
        heap_push(hp, arr0[i]);
        ck_assert_int_eq(*(int*)heap_top(hp), arr1[i]);
        heap_top_val(hp, &val);
        ck_assert_int_eq(val, arr1[i]);
    }

    for (i = 0; i < 10; i++) {
        ck_assert_int_eq(*(int*)heap_top(hp), arr2[i]);
        heap_top_val(hp, &val);
        ck_assert_int_eq(val, arr2[i]);
        ck_assert_int_eq(heap_size(hp), 10 - i);
        heap_pop(hp);
    }

    ck_assert_int_eq(heap_empty(hp), true);
    ck_assert_int_eq(heap_size(hp), 0);
    delete_heap(hp);


    hp = new_heap_if(int, _int_t_greater_func);

    for (i = 0; i < 10; i++) {
        heap_push(hp, arr0[i]);
        ck_assert_int_eq(*(int*)heap_top(hp), arr3[i]);
        heap_top_val(hp, &val);
        ck_assert_int_eq(val, arr3[i]);
    }

    for (i = 0; i < 10; i++) {
        ck_assert_int_eq(*(int*)heap_top(hp), arr4[i]);
        heap_top_val(hp, &val);
        ck_assert_int_eq(val, arr4[i]);
        ck_assert_int_eq(heap_size(hp), 10 - i);
        heap_pop(hp);
    }

    ck_assert_int_eq(heap_empty(hp), true);
    ck_assert_int_eq(heap_size(hp), 0);
    delete_heap(hp);

    ck_assert_int_eq(fn_memdbg_get_recnum(), 0);
}
END_TEST

START_TEST(test_heaop_userdef)
{
    type_unregist(op_t);
    type_regist(op_t, _op_t_copy_func, _op_t_less_func, NULL, NULL);
    heap_t *hp;
    int i = 0;
    hp = new_heap(op_t);
    ck_assert_int_eq(heap_empty(hp), true);
    ck_assert_int_eq(heap_size(hp), 0);


    op_t rec0[8] = {{3,7},{15,32},{3,5},{6,13},{13,6},{21,4},{14,3},{13,1}};
    op_t rec1[8] = {{21,4},{15,32},{14,3},{13,6},{13,1},{6,13},{3,7},{3,5}};
    op_t val;

    for (i = 0; i < 8; i++) {
        heap_push(hp, &rec0[i]);
    }
    for (i = 0; i < 8; i++) {
        heap_top_val(hp, &val);
        ck_assert_int_eq(val.type, rec1[i].type);
        ck_assert_int_eq(val.num,  rec1[i].num);
        heap_pop(hp);
    }

    delete_heap(hp);

    type_unregist(op_t);

    ck_assert_int_eq(fn_memdbg_get_recnum(), 0);
}
END_TEST



START_TEST(test_heapeffec)
{
    srand((unsigned)time(NULL));
    int num = 10000000, i, val;

    op_t *op;
    op = (op_t*)fn_malloc(num * sizeof(op_t));

    int size = 0;
    for(i = 0; i < num; i++) {
        op[i].type = i < num/3 ? 0 : rand()%2;
        switch(op[i].type) {
            // push
            case 0: op[i].num = (long long)rand()*rand()%1000000; break;
            // pop
            case 1: break;
        }
    }

    // test c heap
    fntime_t delta = get_time();
    heap_t hp;
    init_heap(&hp, int);

    for (i = 0; i < num; i++) {
        switch(op[i].type) {
            case 0: heap_push(&hp, op[i].num); break;
            case 1: if(!heap_empty(&hp))heap_pop(&hp); break;
        }
    }

    delta = get_time() - delta;

    printf("C Heap Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    //test c++ priority queue
    delta = get_time();
    priority_queue<int, vector<int>, greater<int> > shp;
    for (i = 0; i < num; i++) {
        switch(op[i].type) {
            case 0: shp.push(op[i].num); break;
            case 1: if (!shp.empty()) shp.pop(); break;
        }
    }
    delta = get_time() - delta;
    printf("STL Heap Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    // check
    ck_assert_int_eq(shp.size(), heap_size(&hp));

    while(!heap_empty(&hp) && !shp.empty()) {
        heap_top_val(&hp, &val);
        ck_assert_int_eq(*(int*)heap_top(&hp), shp.top());
        ck_assert_int_eq(val, shp.top());
        heap_pop(&hp);
        shp.pop();
    }
    ck_assert_int_eq(heap_empty(&hp), true);

    fn_free(op);
    destroy_heap(&hp);
    // read time
    // list_t *lst2 = new_list(int);
    // list<int> sslist;
    // list<int>::iterator it2;

    // int pnum = 5000000;
    // int *pnums = (int*)fn_malloc(sizeof(int) * pnum);
    // int *xnums = (int*)fn_malloc(sizeof(int) * pnum);
    // int *ynums = (int*)fn_malloc(sizeof(int) * pnum);

    // for (i = 0; i < pnum; i++) pnums[i] = rand() % 1000;

    // delta = get_time();
    // for (i = 0; i < pnum; i++) {
    //     list_push_back(lst2, pnums[i]);
    // }

    // int ind = 0;
    // for (it0 = list_begin(lst2); !iter_equal(list_end(lst2), it0); it0 = iter_next(it0)) {
    //     iter_get_value(it0, &xnums[ind++]);
    // }
    // delta = get_time() - delta;
    // printf("C Vector Read Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    // delta = get_time();
    // for (i = 0; i < pnum; i++) {
    //     sslist.push_back(pnums[i]);
    // }

    // ind = 0;
    // for (it2 = sslist.begin(); it2 != sslist.end(); it2++) {
    //     ynums[ind++] = *it2;
    // }
    // delta = get_time() - delta;
    // printf("STL Vector Read Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);
    // for (i = 0 ;i < pnum; i++) {
    //     ck_assert_int_eq(xnums[i], ynums[i]);
    // }
    ck_assert_int_eq(fn_memdbg_get_recnum(), 0);
}
END_TEST



Suite *cheap_test_suite() {
    Suite *s = suite_create("=== CHeap Test ===");
    TCase *tc_core = tcase_create("TC core");
    tcase_set_timeout(tc_core, 1000);

    tcase_add_test(tc_core, test_newheap);
    tcase_add_test(tc_core, test_heapop_cbuiltin);
    tcase_add_test(tc_core, test_heaop_userdef);
#ifdef ENEFFTEST
    //tcase_add_test(tc_core, test_heapeffec);
#endif
    suite_add_tcase(s, tc_core);
    return s;
}