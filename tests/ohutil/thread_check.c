#include <oh.h>
#include <stdio.h>
#include <check.h>

void outp(void *t) {
    printf("%p\n", t);
}

START_TEST(test_pool)
{
    thread_pool *pool = thread_pool_create(10, 10);
    thread_pool_run(pool);
    int i = 0;
    for (i = 0; i < 1000; i++) {
        while (thread_pool_push(pool, outp, NULL, (void*)i) == -1) {};
    }
    thread_pool_destroy(pool);
}
END_TEST

Suite *thread_test_suite() {
    Suite *s = suite_create("=== Thread Test ===");
    TCase *tc_core = tcase_create("TC core");
    tcase_set_timeout(tc_core, 1000);

    tcase_add_test(tc_core, test_pool);
    suite_add_tcase(s, tc_core);
    return s;
}