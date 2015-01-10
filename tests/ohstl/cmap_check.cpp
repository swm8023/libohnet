#include <oh.h>
#include <stdio.h>
#include <stdarg.h>
#include <check.h>
#include "fntest.h"

#include <map>
using namespace std;



START_TEST(test_mapcreate_cbuiltin)
{
    int i;
    map_t *st0 = map_new(int, int);
    pair_t *pr = pair_new(int, int);
    ck_assert_int_eq(map_size(st0),  0);
    ck_assert_int_eq(map_empty(st0), 1);

    int arr0[][2] = {{3, 1}, {1, 2}, {2, 3}, {0, 4}, {7, 5}, {6, 9} ,{8, 4}, {5, 6}, {9, 7}, {4, 2}};
    int arr1[][2] = {{6, 8}, {5, 6}, {9, 7}, {2, 9}, {8, 4}, {3, 1}, {0, 4}, {7, 5}, {4, 2}, {1, 3}};
    int arr2[][2] = {{0, 4}, {1, 3}, {2, 9}, {3, 1}, {4, 2}, {5, 6}, {6, 8}, {7, 5}, {8, 4}, {9, 7}};
    // /* test insert */
    for (i = 0; i < 10; i++) {
        map_put(st0, arr0[i][0], arr0[i][1]);
        ck_assert_int_eq(map_size(st0), i + 1);
        ck_assert_int_eq(map_empty(st0), false);
    }
    map_put(st0, 1, 3);
    map_put(st0, 2, 9);
    pair_make(pr, 6, 8);
    map_insert(st0, pr);
    ck_assert_int_eq(map_size(st0), 10);

    // /* test count */
    for (i = 0;i < 10; i++) {
        ck_assert_int_eq(map_count(st0, arr1[i][0]), 1);

        ck_assert_int_eq(map_count(st0, arr1[i][0] + 10, pr), 0);
    }

    // /* test iterator */

    iterator_t iter = map_find(st0, 3);
    map_erase(st0, iter);

    ck_assert_int_eq(map_count(st0, 3), 0);
    ck_assert_int_eq(map_size(st0), 9);

    iter = map_put(st0, 3, 1);

    int ind = 0;
    for (; !iter_equal(iter, map_end(st0)); iter = iter_next(iter)) {
        pair_t *pr = (pair_t*)iter_get_pointer(iter);

        ck_assert_int_eq(*(int*)pair_first(pr), arr2[ind + 3][0]);
        ck_assert_int_eq(*(int*)pair_second(pr), arr2[ind + 3][1]);
        ind++;
    }
    ind = 0;
    for (iter = map_begin(st0); !iter_equal(iter, map_end(st0)); iter = iter_next(iter)) {
        pair_t *pr = (pair_t*)iter_get_pointer(iter);

        ck_assert_int_eq(*(int*)pair_first(pr), arr2[ind][0]);
        ck_assert_int_eq(*(int*)pair_second(pr), arr2[ind][1]);
        ind ++;
    }
    ck_assert_int_eq(iter_equal(map_end(st0), map_find(st0, -2)), true);

    iter = map_put(st0, -3, 4);
    ck_assert_int_eq(iter_equal(map_begin(st0), iter), true);
    map_erase(st0, map_find(st0, -3));

    /* test erase */
    for (i = 0; i < 10; i++) {
        map_erase(st0, map_find(st0, i));
        ck_assert_int_eq(map_count(st0), 0);
        ck_assert_int_eq(map_size(st0), 9 - i);
    }

    ck_assert_int_eq(map_size(st0),  0);
    ck_assert_int_eq(map_empty(st0), 1);

    pair_delete(pr);
    map_delete(st0);

    ck_assert_int_eq(fn_memdbg_get_recnum(), 0);
}
END_TEST

START_TEST(test_maprandomdata)
{
    map_t *st = map_new(int, int);
    int i, j;
    srand((unsigned)time(NULL));

    /* check insert */
    for (i = 0; i < 100; i++) {
        ck_assert_int_eq(map_size(st),  0);
        ck_assert_int_eq(map_empty(st), 1);
        for (j = 0; j < 1000; j++) {
            if (rand() % 3) {
                map_put(st, rand() % 10000, rand() % 10000);
            } else {
                map_erase(st, map_find(st, rand() % 10000));
            }
        }
        map_clear(st);
    }
    map_delete(st);
}
END_TEST

START_TEST(test_mapeffect)
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
    // test c map insert
    fntime_t delta = get_time();
    map_t *st0 = map_new(int, int);
    pair_t *pr = pair_new(int, int);
    for (int i = 0; i < num; i++) {
        pair_make(pr, x[i], x[i]);
        map_insert(st0, pr);
    }
    delta = get_time() - delta;
    printf("C Map Insert Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    // test stl map insert
    delta = get_time();
    map<int, int> st;
    for (int i = 0; i < num; i++) {
        st.insert(std::pair<int,int>(x[i], x[i]));
    }
    delta = get_time() - delta;
    printf("STL Map Insert Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    ck_assert_int_eq(map_size(st0), st.size());
    ck_assert_int_eq(map_size(st0), num);
    ck_assert_int_eq(st.size(), num);

    // test c map erase
    delta = get_time();
    for (int i = 0; i < num; i++) {
        map_erase_val(st0, x[i]);
    }
    delta = get_time() - delta;
    printf("C Map erase Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    // test c++ map erase
    delta = get_time();
    for (int i = 0; i < num; i++) {
        st.erase(x[i]);
    }
    delta = get_time() - delta;
    printf("STL Map erase Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    ck_assert_int_eq(map_size(st0), 0);
    ck_assert_int_eq(st.size(), 0);

    free(x);
}
END_TEST


Suite *cmap_test_suite() {
    Suite *s = suite_create("=== CMap Test ===");
    TCase *tc_core = tcase_create("TC core");
    tcase_set_timeout(tc_core, 10);

    tcase_add_test(tc_core, test_mapcreate_cbuiltin);
    tcase_add_test(tc_core, test_maprandomdata);
#ifdef ENEFFTEST
    tcase_add_test(tc_core, test_mapeffect);
#endif
    suite_add_tcase(s, tc_core);
    return s;
}