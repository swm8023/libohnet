#include <fn.h>
#include <stdio.h>
#include <check.h>


START_TEST(test_xxx)
{

}
END_TEST


Suite *cvector_test_suite() {
	Suite *s = suite_create("CVector Test");
	TCase *tc_core = tcase_create("TC core");
	tcase_set_timeout(tc_core, 10);

	tcase_add_test(tc_core, test_xxx);
	suite_add_tcase(s, tc_core);
	return s;
}