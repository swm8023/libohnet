#include <fn.h>
#include <stdio.h>
#include <stdarg.h>
#include <check.h>



START_TEST(test_gettype)
{
    ck_assert_str_eq(_get_type_bystr("char")->name, "int8_t");
    ck_assert_str_eq(_get_type_bystr("unsigned char")->name, "uint8_t");
    ck_assert_str_eq(_get_type_bystr("int8_t")->name, "int8_t");
    ck_assert_str_eq(_get_type_bystr("uint8_t")->name, "uint8_t");
    ck_assert_str_eq(_get_type_bystr("short")->name, "int16_t");
    ck_assert_str_eq(_get_type_bystr("unsigned short")->name, "uint16_t");
    ck_assert_str_eq(_get_type_bystr("int16_t")->name, "int16_t");
    ck_assert_str_eq(_get_type_bystr("uint16_t")->name, "uint16_t");
    ck_assert_str_eq(_get_type_bystr("int")->name, "int32_t");
    ck_assert_str_eq(_get_type_bystr("unsigned int")->name, "uint32_t");
    ck_assert_str_eq(_get_type_bystr("int32_t")->name, "int32_t");
    ck_assert_str_eq(_get_type_bystr("uint32_t")->name, "uint32_t");
    ck_assert_str_eq(_get_type_bystr("long long")->name, "int64_t");
    ck_assert_str_eq(_get_type_bystr("unsigned long long")->name, "uint64_t");
    ck_assert_str_eq(_get_type_bystr("int64_t")->name, "int64_t");
    ck_assert_str_eq(_get_type_bystr("uint64_t")->name, "uint64_t");
    ck_assert_str_eq(_get_type_bystr("float")->name, "float");
    ck_assert_str_eq(_get_type_bystr("double")->name, "double");
    ck_assert_str_eq(_get_type_bystr("double*")->name, "pointer");
    ck_assert_str_eq(_get_type_bystr("vector_t<int>")->name, "vector_t");
    ck_assert_str_eq(_get_type_bystr("vector_t<int>*")->name, "pointer");

    int a1 = 1, a2 = 2, a3 = 1;
    ck_assert_int_eq(_get_type_bystr("int")->less(&a1, &a2), true);
    ck_assert_int_eq(_get_type_bystr("int")->less(&a2, &a1), false);
    ck_assert_int_eq(_get_type_bystr("int")->less(&a1, &a3), false);

    typedef int test;
    ck_assert_ptr_eq(_get_type_bystr("test"), NULL);
    type_regist(test, NULL, NULL, NULL, NULL);

    ck_assert_str_eq(_get_type_bystr("test")->name, "test");
    type_regist(test, NULL, NULL, NULL, NULL);
    ck_assert_int_eq(splst_len(_userdef_types), 1);

    type_unregist(test);
    ck_assert_ptr_eq(_get_type_bystr("test"), NULL);
    ck_assert_int_eq(splst_len(_userdef_types), 0);
}
END_TEST

void test_getvargvalue_func(_type_t *type, ...) {
    va_list p;
    va_start(p, type);
    void *val = fn_calloc(1, 20);
    _get_varg_value_bytype(type, p, val);
    ck_assert_int_eq(*(int32_t*)val, 12);

    va_end(p);
    fn_free(val);
}

START_TEST(test_getvargvalue)
{
    _type_t *type;
    int i;
    type = &_builtin_types[0];
    for (i = 0; i < 11; i++) {
        test_getvargvalue_func(type, 12);
    }
}
END_TEST


Suite *ctypes_test_suite() {
    Suite *s = suite_create("=== Ctypes Test ===");
    TCase *tc_core = tcase_create("TC core");
    tcase_set_timeout(tc_core, 10);

    tcase_add_test(tc_core, test_gettype);
    tcase_add_test(tc_core, test_getvargvalue);
    suite_add_tcase(s, tc_core);
    return s;
}