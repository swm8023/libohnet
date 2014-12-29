#include <fn.h>
#include <list>
#include <stdio.h>
#include <assert.h>
using namespace std;

typedef struct _op_t {long long type; int num;} op_t;

bool_t _op_t_copy_func(const void* t1, const void* t2) {
    memcpy((void*)t1, (void*)t2, sizeof(op_t));
    return true;
}

bool_t _op_t_less_func(const void *t1, const void* t2) {
    op_t *o1 = (op_t*)t1;
    op_t *o2 = (op_t*)t2;
    return o1->type > o2->type || o1->type == o2->type && o1->num > o2->num;
}

#define ggg(x) #x
#define fff(x) ggg(x)
int main(int argc, char *argv[]) {
    
    printf("%s\n", fff(_CBUILTIN_TYEP_DECLARE(int8_t)));
	return 0;
}