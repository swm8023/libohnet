#ifndef FNSTL_FNTEST_H
#define FNSTL_FNTEST_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <unistd.h>
#include <oh.h>
#include <check.h>


Suite *cvector_test_suite();
Suite *clist_test_suite();
Suite *cheap_test_suite();
Suite *cpair_test_suite();
Suite *cset_test_suite();
Suite *cmap_test_suite();
Suite *ctypes_test_suite();
Suite *util_test_suite();
Suite *cstring_test_suite();


typedef struct _op_t {long long type; int num;} op_t;
bool_t _op_t_copy_func(void* t1, const void* t2);

bool_t _op_t_less_func(const void *t1, const void* t2);
bool_t _int_t_greater_func(const void *t1, const void *t2);

#ifdef __cplusplus
}
#endif
#endif  //FNSTL_FNTEST_H