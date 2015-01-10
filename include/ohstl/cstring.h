#ifndef OHSTL_CSTRING_H
#define OHSTL_CSTRING_H

#ifdef __cplusplus
extern "C" {
#endif


#include <ohstl/citerator.h>


typedef struct _tag_string_t {
    _alloc_if *aif;
    size_t length;
    size_t capacity;
    char *data;
} string_t;


#define STRING_NPOS -1

#define _STR_INIT_CAPACITY   32

#define _STR_LENGTH(st)     ((st)->length)
#define _STR_CAPACITY(st)   ((st)->capacity)
#define _STR_DATA(st)       ((st)->data)

#define string_new()            _string_new(0)
#define string_new_sz(len)      _string_new(len)
#define string_init(st)         _string_init(st, 0)
#define string_init_sz(st, len) _string_init(st, len)


#define string_size(st)     ((st)->length)
#define string_length(st)   string_size(st)
#define string_empty(st)    (string_size(st) == 0)
#define string_clear(st)    ((st)->length = 0)


string_t* _string_new(size_t);
int _string_init(string_t*, size_t);

void string_destroy(string_t*);
void string_delete(string_t*);

void string_reserve(string_t*, size_t);

string_t* string_from_cstr(string_t*, const char*);
const char* string_to_cstr(string_t*);
void string_copy_cstr(string_t*, char*, size_t);

void string_from_int(string_t*, int);
void string_from_int64(string_t*, int64_t);
void string_from_double(string_t*, double);

int string_to_int(string_t*);
int64_t string_to_int64(string_t*);
double string_to_double(string_t*);

void string_append(string_t*, const string_t*);
void string_append_cstr(string_t*, const char*);

void string_update(string_t*, int, char);
char string_at(const string_t*, int);
int string_indexof(const string_t*, char, int);

void string_assign(string_t*, const string_t*);
bool_t string_less(const string_t*, const string_t*);
bool_t string_equal(const string_t*, const string_t*);
bool_t string_equal_cstr(const string_t*, const char*);





#ifdef __cplusplus
}
#endif
#endif  //OHSTL_CSTRING_H

