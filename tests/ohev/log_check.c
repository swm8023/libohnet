#include <oh.h>
#include <stdio.h>
#include <check.h>


START_TEST(test_console_log)
{
    set_default_logif_level(LOG_TRACE);
    log_info("hello world");
}
END_TEST



Suite *log_test_suite() {
    Suite *s = suite_create("=== Log Test ===");
    TCase *tc_core = tcase_create("TC core");
    tcase_set_timeout(tc_core, 10);

   // tcase_add_test(tc_core, test_console_log);
    suite_add_tcase(s, tc_core);
    return s;
}