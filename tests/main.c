#include <stdio.h>
#include <oh.h>
#include <assert.h>

#include <check.h>

#include "ohstl/fntest.h"
#include "ohev/fntest.h"
#include "ohutil/fntest.h"



int main(int argc, char* argv[]) {
	SRunner *sr = srunner_create (util_test_suite());
    srunner_add_suite(sr, ctypes_test_suite());
    srunner_add_suite(sr, cvector_test_suite());
    srunner_add_suite(sr, clist_test_suite());
    srunner_add_suite(sr, cheap_test_suite());
    srunner_add_suite(sr, cpair_test_suite());
    srunner_add_suite(sr, cset_test_suite());
    srunner_add_suite(sr, cmap_test_suite());
    srunner_add_suite(sr, cstring_test_suite());

    srunner_add_suite(sr, log_test_suite());
    // srunner_add_suite(sr, evt_test_suite());
    // srunner_add_suite(sr, evtpool_test_suite());
    srunner_add_suite(sr, objpool_test_suite());
    srunner_add_suite(sr, buffer_test_suite());
    srunner_add_suite(sr, thread_test_suite());

    srunner_set_fork_status(sr, CK_NOFORK);
	srunner_run_all (sr, CK_NORMAL);
	int number_failed = srunner_ntests_failed (sr);
  	srunner_free (sr);

    assert(fn_memdbg_get_recnum() ==  0);

	return number_failed;
}