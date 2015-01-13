#include <oh.h>
#include <stdio.h>
#include <check.h>


START_TEST(test_time)
{
	fntime_t now = get_time();
	char buf1[25], buf2[25], buf3[25], buf4[25];

	ck_assert_int_eq(get_time_str(buf1, 24), -1);

	get_catime_str(buf1, 25);
	get_catime_str(buf2, 25);
	ck_assert_str_eq(buf1, buf2);

	ohtime_to_str(get_catime(), buf2, 25);
	ck_assert_str_eq(buf1, buf2);

	update_catime();
	get_catime_str(buf2, 25);
	ck_assert_str_ne(buf1, buf2);

	disable_time_cache();
	get_catime_str(buf1, 25);
	get_catime_str(buf2, 25);
	ck_assert_str_ne(buf1, buf2);

	enable_time_cache();
	get_catime_str(buf1, 25);
	ck_assert_str_eq(buf1, buf2);


	ck_assert_int_eq(ohusleep(500), 0);
}
END_TEST


START_TEST(test_malloc_debug)
{
// #ifdef DEBUG
// 	char *f1, *f2, *f3, *f4;
// 	f1 = (char*)oh_malloc(10);
// 	f2 = (char*)oh_malloc(20);
// 	ck_assert_int_eq(oh_memdbg_get_recnum(), 2);
// 	oh_memdbg_print_rec();

// 	f3 = (char*)oh_calloc(2, 20);
// 	f4 = (char*)oh_realloc(f1, 100);
// 	ck_assert_int_eq(oh_memdbg_get_recnum(), 3);
// 	oh_memdbg_print_rec();

// 	oh_free(f2);
// 	ck_assert_int_eq(oh_memdbg_get_recnum(), 2);
// 	oh_memdbg_print_rec();

// 	oh_free(f3);
// 	oh_free(f4);
// 	ck_assert_int_eq(oh_memdbg_get_recnum(), 0);
// 	oh_memdbg_print_rec();
// #else
// 	char *f1, *f2, *f3, *f4;
// 	f1 = (char*)oh_malloc(10);
// 	f2 = (char*)oh_malloc(20);

// 	f3 = (char*)oh_calloc(2, 20);
// 	f4 = (char*)oh_realloc(f1, 100);

// 	oh_free(f2);

// 	oh_free(f3);
// 	oh_free(f4);
// #endif

}
END_TEST


START_TEST(test_normal)
{
	ck_assert_int_eq(max(3, -3), 3);
	ck_assert_int_eq(max(3,  5), 5);
	ck_assert_int_eq(min(3, -3),-3);
	ck_assert_int_eq(min(3,  5), 3);
}
END_TEST


typedef struct _tag_test_lst {
	SPLST_DEFNEXT(struct _tag_test_lst);
	int val;
}test_lst;

START_TEST(test_sp_list)
{
	test_lst *head = SPLST_INITVAL;
	test_lst *elm1, *elm2, *elm3;

	ck_assert_int_eq(splst_len(head), 0);

	elm1 = (test_lst*)malloc(sizeof(test_lst));
	elm2 = (test_lst*)malloc(sizeof(test_lst));
	elm3 = (test_lst*)malloc(sizeof(test_lst));
	elm1->val = 1;
	elm2->val = 2;
	elm3->val = 3;

	splst_add(head, elm1);
	splst_add(head, elm2);
	splst_add(head, elm3);

	ck_assert_int_eq(splst_len(head), 3);
	ck_assert_int_eq(head->val, 3);

	splst_del(head, elm3);
	splst_del(head, elm1);

	ck_assert_int_eq(splst_len(head), 1);
	ck_assert_int_eq(head->val, 2);

	splst_del(head, elm1);
	ck_assert_int_eq(splst_len(head), 1);

	splst_del(head, elm2);
	ck_assert_int_eq(splst_len(head), 0);

	free(elm1);
	free(elm2);
	free(elm3);

}
END_TEST

START_TEST(test_thread_atomic)
{
	atomic8_t  ato8  = 0;
	atomic16_t ato16 = 0;
	atomic32_t ato32 = 0;
	atomic64_t ato64 = 0;
	atomic_increment(ato8);
	atomic_increment(ato16);
	atomic_increment(ato32);
	atomic_increment(ato64);
	ck_assert_int_eq(ato8,  1);
	ck_assert_int_eq(ato16, 1);
	ck_assert_int_eq(ato32, 1);
	ck_assert_int_eq(ato64, 1);

	atomic_decrement(ato8);
	atomic_decrement(ato16);
	atomic_decrement(ato32);
	atomic_decrement(ato64);
	ck_assert_int_eq(ato8,  0);
	ck_assert_int_eq(ato16, 0);
	ck_assert_int_eq(ato32, 0);
	ck_assert_int_eq(ato64, 0);

	ck_assert_int_eq(atomic_add_get(ato8,  35), 35);
	ck_assert_int_eq(atomic_add_get(ato16, 35), 35);
	ck_assert_int_eq(atomic_add_get(ato32, 35), 35);
	ck_assert_int_eq(atomic_add_get(ato64, 35), 35);

	ck_assert_int_eq(atomic_get_add(ato8,  -10), 35);
	ck_assert_int_eq(atomic_get_add(ato16, -10), 35);
	ck_assert_int_eq(atomic_get_add(ato32, -10), 35);
	ck_assert_int_eq(atomic_get_add(ato64, -10), 35);

	ck_assert_int_eq(atomic_get(ato8), 25);
	ck_assert_int_eq(atomic_get(ato16), 25);
	ck_assert_int_eq(atomic_get(ato32), 25);
	ck_assert_int_eq(atomic_get(ato64), 25);

}
END_TEST

char mutex_test_array[20];
int mutex_test_array_pos = 0;
THREAD_FUNC_START(mutex_thread, arg) {
	int i = 0;
	mutex_t mutex = (mutex_t)arg;
	mutex_lock(mutex);
	for (i = 0; i < 5; i++) {
		mutex_test_array[mutex_test_array_pos++] = (char)('a'+i);
		usleep(1000);
	}
	mutex_unlock(mutex);
}
THREAD_FUNC_END

START_TEST(test_thread_mutex)
{
#ifdef DEBUG
	mutex_t mutex1 = mutex_init(NULL);
	mutex_t mutex2 = mutex_init(NULL);
	oh_mtxdbg_print_rec();
	mutex_lock(mutex1);
	mutex_lock(mutex2);
	oh_mtxdbg_print_rec();
	mutex_unlock(mutex1);
	oh_mtxdbg_print_rec();
	mutex_unlock(mutex2);
	oh_mtxdbg_print_rec();
	oh_memdbg_print_rec();

	mutex_destroy(mutex1);
	mutex_destroy(mutex2);
	oh_mtxdbg_print_rec();

	mutex_t mutex = mutex_init(NULL);
	thread_t t1, t2;
	mutex_test_array_pos = 0;
	thread_start(t1, mutex_thread, (void*)mutex);
	thread_start(t2, mutex_thread, (void*)mutex);
	thread_join(t1);
	thread_join(t2);
	mutex_test_array[mutex_test_array_pos] = 0;
	ck_assert_str_eq("abcdeabcde", mutex_test_array);
	mutex_destroy(mutex);
	oh_memdbg_print_rec();

#else
	mutex_t mutex = mutex_init(NULL);
	thread_t t1, t2;
	mutex_test_array_pos = 0;
	thread_start(t1, mutex_thread, (void*)mutex);
	thread_start(t2, mutex_thread, (void*)mutex);
	thread_join(t1);
	thread_join(t2);
	mutex_test_array[mutex_test_array_pos] = 0;
	ck_assert_str_eq("abcdeabcde", mutex_test_array);
	mutex_destroy(mutex);
#endif

}
END_TEST

cond_t cm;
THREAD_FUNC_START(cond_thread, arg) {
	int i = 0, mod = (size_t)arg;
	for (i = 0; i < 5; i++) {
		cond_lock(cm);
		while (mutex_test_array_pos % 2 != mod)
			cond_wait(cm);
		mutex_test_array[mutex_test_array_pos++] = (char)('a'+i);
		cond_unlock(cm)
		cond_signal(cm);
	}
}
THREAD_FUNC_END

START_TEST(test_thread_cond)
{
	cm = cond_init(NULL);
	thread_t t1, t2;
	mutex_test_array_pos = 0;
	memset(mutex_test_array, 0, sizeof(mutex_test_array));
	thread_start(t1, cond_thread, (void*)0);
	thread_start(t2, cond_thread, (void*)1);
	thread_join(t1);
	thread_join(t2);
	oh_memdbg_print_rec();
	mutex_test_array[mutex_test_array_pos] = 0;
	ck_assert_str_eq("aabbccddee", mutex_test_array);
	cond_timedwait(cm, SECOND(0.3));
	cond_destroy(cm);
	oh_memdbg_print_rec();
}
END_TEST

Suite *util_test_suite() {
	Suite *s = suite_create("=== Util Test ===");
	TCase *tc_core = tcase_create("TC core");
	tcase_set_timeout(tc_core, 10);

	tcase_add_test(tc_core, test_time);
	tcase_add_test(tc_core, test_malloc_debug);
	tcase_add_test(tc_core, test_normal);
	tcase_add_test(tc_core, test_sp_list);
	tcase_add_test(tc_core, test_thread_atomic);
	tcase_add_test(tc_core, test_thread_mutex);
	tcase_add_test(tc_core, test_thread_cond);
	suite_add_tcase(s, tc_core);
	return s;
}