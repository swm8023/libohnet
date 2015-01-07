#include <fn.h>
#include <stdio.h>
#include <stdarg.h>
#include <check.h>
#include <string.h>
#include "fntest.h"



START_TEST(test_stringfunc)
{
    string_t *str0 = string_new();
    string_t *str1 = string_new();
    ck_assert_int_eq(string_length(str0), 0);

    string_from_cstr(str1, "world");
    string_from_cstr(str0, "hello");

    string_append_cstr(str0, " ");
    string_append(str0, str1);
    string_append_cstr(str0, "!");
    ck_assert_int_eq(strcmp(string_to_cstr(str0), "hello world!"), 0);

    char buf[4096];
    string_copy_cstr(str0, buf, 4096);
    ck_assert_int_eq(strcmp(buf, "hello world!"), 0);

    ck_assert_int_eq(string_at(str0, 1), 'e');
    ck_assert_int_eq(string_indexof(str0, 'o', 0), 4);

    string_update(str0, 4, 'w');
    ck_assert_int_eq(strcmp(string_to_cstr(str0), "hellw world!"), 0);

    string_from_int(str0, 64);
    ck_assert_int_eq(strcmp(string_to_cstr(str0), "64"), 0);
    ck_assert_int_eq(string_to_int(str0), 64);
    string_from_int64(str0, -123);
    ck_assert_int_eq(string_to_int64(str0) == -123, 1);
    ck_assert_int_eq(strcmp(string_to_cstr(str0), "-123"), 0);
    string_from_double(str0, -43.123);
    ck_assert_int_eq(string_to_double(str0) ==  -43.123, 1);
    ck_assert_int_eq(strcmp(string_to_cstr(str0), "-43.123000"), 0);

    string_delete(str0);
    string_delete(str1);
    ck_assert_int_eq(fn_memdbg_get_recnum(), 0);
}
END_TEST

START_TEST(test_stringaskey)
{
    map_t *mp = map_new(string_t, int);
    string_t *str = string_new();

    map_put(mp, string_from_cstr(str, "abc"), 1);
    map_put(mp, string_from_cstr(str, "dab"), 4);
    map_put(mp, string_from_cstr(str, "bcd"), 3);
    map_put(mp, string_from_cstr(str, "adc"), 2);
    map_put(mp, string_from_cstr(str, "adcd"), -1);
    map_put(mp, string_from_cstr(str, ""), 23);

    iterator_t it = map_begin(mp);
    while (!iter_equal(it, map_end(mp))) {
        pair_t *p = (pair_t*)iter_get_pointer(it);
        string_t *t = (string_t*)pair_first(p);
        int val = *(int*)pair_second(p);
        // printf("%s %d\n", string_to_cstr(t), val);
        it = iter_next(it);
    }

    map_erase(mp, map_find(mp, string_from_cstr(str, "")));

    it = map_begin(mp);
    while (!iter_equal(it, map_end(mp))) {
        pair_t *p = (pair_t*)iter_get_pointer(it);
        string_t *t = (string_t*)pair_first(p);
        int val = *(int*)pair_second(p);
        // printf("%s %d\n", string_to_cstr(t), val);
        it = iter_next(it);
    }

    string_delete(str);
    map_delete(mp);
}
END_TEST


Suite *cstring_test_suite() {
    Suite *s = suite_create("=== CString Test ===");
    TCase *tc_core = tcase_create("TC core");
    tcase_set_timeout(tc_core, 10);

    tcase_add_test(tc_core, test_stringfunc);
    tcase_add_test(tc_core, test_stringaskey);


    suite_add_tcase(s, tc_core);
    return s;
}