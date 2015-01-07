#include <fn.h>
#include <stdio.h>
#include <stdarg.h>
#include <check.h>
#include "fntest.h"



START_TEST(test_pairfunc)
{
    pair_t *pair = pair_new(int, double);
    int vali;
    double vald;
    assert(_PAIR_STNODE(pair)->type == _get_type_bystr(_PAIR_TYPE_NAME));
    ck_assert_ptr_eq(_PAIR_CTYPE(pair, 0), _get_type_bystr("int"));
    ck_assert_ptr_eq(_PAIR_CTYPE(pair, 1), _get_type_bystr("double"));

    pair_make(pair, 1, 3.5);
    ck_assert_int_eq(*(int*)pair_first(pair), 1);
    ck_assert_int_eq(*(double*)pair_second(pair) == 3.5, true);

    pair_make(pair, 2, 4.5);
    ck_assert_int_eq(*(int*)pair_first(pair), 2);
    ck_assert_int_eq(*(double*)pair_second(pair) == 4.5, true);

    pair_first_value(pair, &vali);
    pair_second_value(pair, &vald);
    ck_assert_int_eq(vali, 2);
    ck_assert_int_eq(vald == 4.5, true);

    pair_t *pair2 = pair_new(int, double);
    pair_assign(pair2, pair);
    ck_assert_int_eq(*(int*)pair_first(pair2), 2);
    ck_assert_int_eq(*(double*)pair_second(pair2) == 4.5, true);

    ck_assert_int_eq(_type_less(_get_type_bystr("int"), &vali, &vali), false);
    ck_assert_int_eq(_type_equal(_get_type_bystr("int"), &vali, &vali), true);
    ck_assert_int_eq(_type_less(_get_type_bystr("double"), &vald, &vald), false);

    ck_assert_int_eq(_pair_less(pair, pair2), false);
    ck_assert_int_eq(_pair_less(pair2, pair), false);

    pair_make(pair2, 3, 4.5);
    ck_assert_int_eq(_pair_less(pair, pair2), true);
    pair_make(pair2, 2, 4.6);
    ck_assert_int_eq(_pair_less(pair, pair2), true);
    pair_make(pair2, 1, 4.6);
    ck_assert_int_eq(_pair_less(pair, pair2), false);
    pair_make(pair2, 2, 4.3);
    ck_assert_int_eq(_pair_less(pair, pair2), false);

    pair_delete(pair);
    pair_delete(pair2);

    ck_assert_int_eq(fn_memdbg_get_recnum(), 0);
}
END_TEST



Suite *cpair_test_suite() {
    Suite *s = suite_create("=== CPair Test ===");
    TCase *tc_core = tcase_create("TC core");
    tcase_set_timeout(tc_core, 10);

    tcase_add_test(tc_core, test_pairfunc);


    suite_add_tcase(s, tc_core);
    return s;
}