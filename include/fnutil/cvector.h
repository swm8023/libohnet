#ifndef FNUTIL_CVECTOR_H
#define FNUTIL_CVECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fnutil/util.h>
typedef struct _tag_vector_t vector_t;

typedef struct _tag_vector_if {
	void* (*at)(vector_t*, int);
	void (*push_back)(vector_t*, ...);
	void (*pop_back)(vector_t*);
	void (*clear)(vector_t*);
	void (*insert)(vector_t*, int, ...);
} vector_if;


typedef struct _tag_vector_t {
	int size;
	int capacity;
	void *data;
	vector_if *vif;
} vector_t;

#define new_vector(x) _new_vector(#x)
vector_t* _new_vector(const char* typestr);
void free_vector();


#ifdef __cplusplus
}
#endif
#endif  //FNUTIL_CVECTOR_H