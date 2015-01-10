#include <oh.h>
#include <stdio.h>
#include <stdarg.h>
#include <check.h>
#include "fntest.h"

#include <set>
using namespace std;

START_TEST(test_setcreate_cbuiltin)
{
    int i;
    set_t *st0 = set_new(int);
    ck_assert_int_eq(set_size(st0),  0);
    ck_assert_int_eq(set_empty(st0), 1);

    int arr0[] = {3, 1, 2, 0, 7, 5, 6, 9 ,8, 4};
    int arr1[] = {2, 0, 3, 1, 4, 5, 7, 8, 9, 6};
    int arr2[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    /* test insert */
    for (i = 0; i < 10; i++) {
        set_insert(st0, arr0[i]);
        ck_assert_int_eq(set_size(st0), i + 1);
        ck_assert_int_eq(set_empty(st0), false);
    }
    set_insert(st0, 3);
    set_insert(st0, 0);
    set_insert(st0, 4);
    ck_assert_int_eq(set_size(st0), 10);

    /* test count */
    for (i = 0;i < 10; i++) {
        ck_assert_int_eq(set_count(st0, arr1[i]), 1);
        ck_assert_int_eq(set_count(st0, arr1[i] + 10), 0);
    }

    /* test iterator */
    iterator_t iter = set_find(st0, 3);
    int ind = 0;
    for (; !iter_equal(iter, set_end(st0)); iter = iter_next(iter)) {
        int val;
        iter_get_value(iter, &val);
        ck_assert_int_eq(val, ind++ + 3);
    }
    ind = 0;
    for (iter = set_begin(st0); !iter_equal(iter, set_end(st0)); iter = iter_next(iter)) {
        int val;
        iter_get_value(iter, &val);
        ck_assert_int_eq(val, ind++);
    }
    ck_assert_int_eq(iter_equal(set_end(st0), set_find(st0, 11)), true);

    iter = set_insert(st0, -1);
    ck_assert_int_eq(iter_equal(set_begin(st0), iter), true);
    set_erase_val(st0, -1);

    /* test erase */
    for (i = 0; i < 10; i++) {
        set_erase_val(st0, arr1[i]);
        set_erase_val(st0, arr1[i] + 10);
        ck_assert_int_eq(set_count(st0, arr1[i]), 0);
        ck_assert_int_eq(set_size(st0), 9 - i);
    }

    ck_assert_int_eq(set_size(st0),  0);
    ck_assert_int_eq(set_empty(st0), 1);

    set_delete(st0);

}
END_TEST

START_TEST(test_setcreate_fnbuiltin)
{
    int i;
    set_t *st0 = set_new(pair_t<int, int>);
    pair_t *pr = pair_new(int, int);
    ck_assert_int_eq(set_size(st0),  0);
    ck_assert_int_eq(set_empty(st0), 1);

    int arr0[][2] = {{3, 1}, {1, 2}, {1, 3}, {0, 4}, {7, 5}, {6, 9} ,{8, 4}, {6, 6}, {7, 7}, {-1, 2}};
    int arr1[][2] = {{6, 9}, {6, 6}, {7, 7}, {1, 3}, {8, 4}, {3, 1}, {0, 4}, {7, 5}, {-1, 2}, {1, 2}};
    int arr2[][2] = {{-1, 2}, {0, 4}, {1, 2}, {1, 3}, {3, 1}, {6, 6}, {6, 9}, {7, 5}, {7, 7}, {8, 4}};
    // /* test insert */
    for (i = 0; i < 10; i++) {
        pair_make(pr, arr0[i][0], arr0[i][1]);
        set_insert(st0, pr);
        ck_assert_int_eq(set_size(st0), i + 1);
        ck_assert_int_eq(set_empty(st0), false);
    }
    pair_make(pr, arr0[1][0], arr0[1][1]);
    set_insert(st0, pr);
    pair_make(pr, arr0[3][0], arr0[3][1]);
    set_insert(st0, pr);
    pair_make(pr, arr0[7][0], arr0[7][1]);
    set_insert(st0, pr);
    ck_assert_int_eq(set_size(st0), 10);

    /* test count */
    for (i = 0;i < 10; i++) {
        pair_make(pr, arr1[i][0], arr1[i][1]);
        ck_assert_int_eq(set_count(st0, pr), 1);

        pair_make(pr, arr1[i][0] + 10, arr1[i][1] - 1);
        ck_assert_int_eq(set_count(st0, pr), 0);
    }

    /* test iterator */
    pair_make(pr, arr2[3][0], arr2[3][1]);

    iterator_t iter = set_find(st0, pr);
    set_erase(st0, iter);

    ck_assert_int_eq(set_count(st0, pr), 0);
    ck_assert_int_eq(set_size(st0), 9);

    iter = set_insert(st0, pr);

    int ind = 0;
    for (; !iter_equal(iter, set_end(st0)); iter = iter_next(iter)) {
        int val1, val2;
        iter_get_value(iter, pr);
        pair_value(pr, &val1, &val2);
        ck_assert_int_eq(val1, arr2[ind + 3][0]);
        ck_assert_int_eq(val2, arr2[ind + 3][1]);
        ind++;
    }
    ind = 0;
    for (iter = set_begin(st0); !iter_equal(iter, set_end(st0)); iter = iter_next(iter)) {
        int val1, val2;
        iter_get_value(iter, pr);
        pair_value(pr, &val1, &val2);
        ck_assert_int_eq(val1, arr2[ind][0]);
        ck_assert_int_eq(val2, arr2[ind][1]);
        ind ++;
    }
    pair_make(pr, 0, 5);
    ck_assert_int_eq(iter_equal(set_end(st0), set_find(st0, pr)), true);

    pair_make(pr, -2, 3);
    iter = set_insert(st0, pr);
    ck_assert_int_eq(iter_equal(set_begin(st0), iter), true);
    set_erase_val(st0, pr);

    /* test erase */
    for (i = 0; i < 10; i++) {
        pair_make(pr, arr0[i][0], arr0[i][1]);
        set_erase_val(st0, pr);
        ck_assert_int_eq(set_count(st0, pr), 0);
        ck_assert_int_eq(set_size(st0), 9 - i);
    }

    ck_assert_int_eq(set_size(st0),  0);
    ck_assert_int_eq(set_empty(st0), 1);

    pair_delete(pr);
    set_delete(st0);
}
END_TEST

START_TEST(test_setrandomdata)
{
    set_t *st = set_new(int);
    int i, j;
    srand((unsigned)time(NULL));

    /* check insert */
    for (i = 0; i < 100; i++) {
        ck_assert_int_eq(set_size(st),  0);
        ck_assert_int_eq(set_empty(st), 1);
        for (j = 0; j < 1000; j++) {
            if (rand() % 3) {
                set_insert(st, rand() % 10000);
            } else {
                set_erase_val(st, rand() % 10000);
            }
        }
        set_clear(st);
    }
    set_delete(st);
}
END_TEST

START_TEST(test_seteffect)
{
    int i;
    int num = 5000000, base = 1000;
    int *x = (int*)malloc(num*sizeof(4));
    for (i = 0; i < base; i++) x[i] = i;
    for (i = base - 1; i >= 0; i--) {
        int pos = rand() % (i + 1);
        fnswap(x[i], x[pos]);
    }
    for (i = base; i < num; i ++) {
        x[i] = x[i-base] + base;
    }
    // test c set insert
    fntime_t delta = get_time();
    set_t *st0 = set_new(int);
    for (int i = 0; i < num; i++) {
        set_insert(st0, x[i]);
    }
    delta = get_time() - delta;
    printf("C Set Insert Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    // test stl set insert
    delta = get_time();
    set<int> st;
    for (int i = 0; i < num; i++) {
        st.insert(x[i]);
    }
    delta = get_time() - delta;
    printf("STL Set Insert Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    ck_assert_int_eq(set_size(st0), st.size());
    ck_assert_int_eq(set_size(st0), num);
    ck_assert_int_eq(st.size(), num);

    // test c set erase
    delta = get_time();
    for (int i = 0; i < num; i++) {
        set_erase_val(st0, x[i]);
    }
    delta = get_time() - delta;
    printf("C Set erase Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    // test c++ set erase
    delta = get_time();
    for (int i = 0; i < num; i++) {
        st.erase(x[i]);
    }
    delta = get_time() - delta;
    printf("STL Set erase Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    ck_assert_int_eq(set_size(st0), 0);
    ck_assert_int_eq(st.size(), 0);
    free(x);
}
END_TEST


Suite *cset_test_suite() {
    Suite *s = suite_create("=== CSet Test ===");
    TCase *tc_core = tcase_create("TC core");
    tcase_set_timeout(tc_core, 10);

    tcase_add_test(tc_core, test_setcreate_cbuiltin);
    tcase_add_test(tc_core, test_setcreate_fnbuiltin);
    tcase_add_test(tc_core, test_setrandomdata);
#ifdef ENEFFTEST
    tcase_add_test(tc_core, test_seteffect);
#endif
    suite_add_tcase(s, tc_core);
    return s;
}