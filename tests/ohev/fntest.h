#ifndef FNEV_FNTEST_H
#define FNEV_FNTEST_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <unistd.h>
#include <oh.h>
#include <check.h>


Suite *log_test_suite();
Suite *evt_test_suite();
Suite *evtpool_test_suite();


#ifdef __cplusplus
}
#endif
#endif  //FNSTL_FNTEST_H