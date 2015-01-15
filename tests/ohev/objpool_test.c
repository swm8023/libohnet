#include <oh.h>
#include <stdio.h>
#include <check.h>
#include <unistd.h>
#include <sys/socket.h>



START_TEST(test_objpool)
{
    objpool_base *op = (objpool_base*)ohmalloc(sizeof(objpool_base));
    objpool_init(op, 8, sizeof(objpool_obj_base));

    int num = 30, i;
    objpool_obj_base **base = (objpool_obj_base**)ohmalloc(num * sizeof(objpool_obj_base*));
    for (i = 0; i < num; i++) {
        base[i] = objpool_get_obj(op);
    }

    for (i = 0; i < 10; i++) {
        base[i] = objpool_free_obj(op, base[i]);
    }

    for (i = 0; i < 13; i++) {
        base[i] = objpool_get_obj(op);
    }

    objpool_destroy(op);
    ohfree(base);
    ohfree(op);

}
END_TEST



Suite *objpool_test_suite() {
    Suite *s = suite_create("=== Objpool Test ===");
    TCase *tc_core = tcase_create("TC core");
    tcase_set_timeout(tc_core, 1000);

    tcase_add_test(tc_core, test_objpool);
    suite_add_tcase(s, tc_core);
    return s;
}