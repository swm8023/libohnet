#ifndef OHUTIL_UTIL_H
#define OHUTIL_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>

#include <ohutil/memory.h>
#include <ohutil/thread.h>
#include <ohutil/container.h>
#include <ohutil/time.h>
#include <ohutil/objpool.h>

#define STRING(x) #x
#define DEFINE_TEST(x) STRING(x)

#define fnswap swap
#define ohswap(a, b) do {   \
    typeof(a) _temp_ = (a); \
    (a) = (b);              \
    (b) = _temp_;           \
} while(0)

#define min(a, b) ({        \
    typeof(a) _ta = (a);    \
    typeof(b) _tb = (b);    \
    _ta < _tb ? _ta : _tb;  \
})

#define max(a, b) ({        \
    typeof(a) _ta = (a);    \
    typeof(b) _tb = (b);    \
    _ta > _tb ? _ta : _tb;  \
})

#define adj_between(param, minv, maxv) ({   \
    if ((param) < (minv)) (param) = (minv); \
    if ((param) > (maxv)) (param) = (maxv); \
    (param);                                \
})


typedef size_t bool_t;
#define true   1
#define false  0

#define RETURN_IF(x, arg...)\
    if ((x)) return arg;

#define RETURN_IF_NULL(x, arg...)   \
    if ((x) == NULL) return arg;

#define container_of(ptr, type, member) ({                      \
        (type *)((char *)(ptr) - offsetof(type,member) );})



#ifdef __cplusplus
}
#endif
#endif  //OHUTIL_UTIL_H