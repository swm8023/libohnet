#include <oh.h>
#include <stdio.h>
#include <check.h>


START_TEST(test_buffer)
{
    char rbuf[20];
    ohbuffer buf;
    buf_init(&buf, 1, NULL, 0);
    int wsz = buf_write(&buf, "hello world", 11);
    ck_assert_int_eq(wsz, 11);

    wsz = buf_read(&buf, 5, rbuf, 20);
    ck_assert_int_eq(wsz, 5);
    rbuf[wsz] = 0;
    ck_assert_str_eq("hello", rbuf);

    wsz = buf_read(&buf, 20, rbuf, 20);
    ck_assert_int_eq(wsz, 6);
    rbuf[wsz] = 0;
    ck_assert_str_eq(" world", rbuf);

    buf_destroy(&buf);

}
END_TEST

START_TEST(test_buffer_pool)
{
    ohbuffer_unit_objpool *pool = (ohbuffer_unit_objpool*)ohmalloc(sizeof(ohbuffer_unit_objpool));

    bufunit_pool_init(pool,
        32, 1 + sizeof(ohbuffer_unit));
    char rbuf[20];
    ohbuffer buf;
    buf_init(&buf, 1, pool, 1);
    int wsz = buf_write(&buf, "hello world", 11);
    ck_assert_int_eq(wsz, 11);

    wsz = buf_read(&buf, 5, rbuf, 20);
    ck_assert_int_eq(wsz, 5);
    rbuf[wsz] = 0;
    ck_assert_str_eq("hello", rbuf);

    wsz = buf_read(&buf, 20, rbuf, 20);
    ck_assert_int_eq(wsz, 6);
    rbuf[wsz] = 0;
    ck_assert_str_eq(" world", rbuf);

    buf_destroy(&buf);
    bufunit_pool_destroy(pool);
    ohfree(pool);
}
END_TEST



Suite *buffer_test_suite() {
    Suite *s = suite_create("=== Buffer Test ===");
    TCase *tc_core = tcase_create("TC core");
    tcase_set_timeout(tc_core, 10);

    tcase_add_test(tc_core, test_buffer);
    tcase_add_test(tc_core, test_buffer_pool);
    suite_add_tcase(s, tc_core);
    return s;
}