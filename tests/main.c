#include <stdio.h>
#include <fn.h>
#include <assert.h>
#include "fntest.h"

#include <check.h>

Suite *ctypes_test_suite();
Suite *util_test_suite();

int main(int argc, char* argv[]) {
	SRunner *sr = srunner_create (util_test_suite());
    srunner_add_suite(sr, ctypes_test_suite());
    srunner_add_suite(sr, cvector_test_suite());
    srunner_add_suite(sr, clist_test_suite());
    srunner_add_suite(sr, cheap_test_suite());
    srunner_add_suite(sr, cpair_test_suite());


    srunner_set_fork_status(sr, CK_NOFORK);
	srunner_run_all (sr, CK_NORMAL);
	int number_failed = srunner_ntests_failed (sr);
  	srunner_free (sr);

    assert(fn_memdbg_get_recnum() ==  0);

	return number_failed;
}