#ifndef OHUTIL_MEMORY_H
#define OHUTIL_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>


/* debug mode */
#ifdef DEBUG
#define ohmalloc(size)          _oh_memdbg_alloc(NULL, (size), 0, __FILE__, __LINE__)
#define ohcalloc(cnt, size)     _oh_memdbg_alloc(NULL, (cnt) * (size), 1,  __FILE__, __LINE__)
#define ohrealloc(ptr, size)    _oh_memdbg_alloc((ptr), (size), 0,  __FILE__, __LINE__)
#define ohfree(ptr)             _oh_memdbg_free((ptr))
/* realease mode */
#else
#define ohmalloc  malloc
#define ohcalloc  calloc
#define ohrealloc realloc
#define ohfree    free
#endif

#define _MEMDBG_ALIGNSIZE sizeof(size_t)

void* _oh_memdbg_alloc(void*, size_t, int, const char* const file, int line);
void _oh_memdbg_free(void*);

int  oh_memdbg_get_recnum();
void oh_memdbg_print_rec();


/* compatibility with old version*/
#define fn_malloc   ohmalloc
#define fn_calloc   ohcalloc
#define fn_realloc  ohrealloc
#define fn_free     ohfree
#define fn_memdbg_print_rec()  oh_memdbg_print_rec()
#define fn_memdbg_get_recnum() oh_memdbg_get_recnum()


#ifdef __cplusplus
}
#endif
#endif  //OHUTIL_MEMORY_H