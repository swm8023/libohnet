#include <assert.h>
#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <fnstl/cstring.h>

string_t* _string_new(size_t size) {
    string_t *str = (string_t*)fn_malloc(sizeof(string_t));
    RETURN_IF_NULL(str, NULL);

    if (_string_init(str, size) < 0) {
        fn_free(str);
        return NULL;
    }
    return str;
}

int _string_init(string_t* str, size_t size) {
    assert(str != NULL);

    str->aif      = _default_cmem_if;
    str->length   = 0;
    str->capacity = size ? size : _STR_INIT_CAPACITY;
    str->data     = (char*)str->aif->alloc(str->capacity);

    return str->data ? 0 : -1;
}

void string_destroy(string_t* str) {
    assert(str != NULL);

    str->aif->free(str->data);
}

void string_delete(string_t* str) {
    assert(str != NULL);

    string_destroy(str);
    fn_free(str);
}

void string_assign(string_t* dest, const string_t* src) {
    assert(dest != NULL);
    assert(src != NULL);

    if (dest == src) return;

    string_clear(dest);
    string_append(dest, src);
}

bool_t string_less(const string_t* str0, const string_t* str1) {
    assert(str0 != NULL);
    assert(str1 != NULL);

    int i, minlen = min(str0->length, str1->length);
    for (i = 0; i < minlen; i++) {
        int delta = string_at(str0, i) - string_at(str1, i);
        if (delta != 0) {
            return delta < 0 ? true : false;
        }
    }
    return str1->length > str0->length ;
}

bool_t string_equal(const string_t* str0, const string_t* str1) {
    assert(str0 != NULL);
    assert(str1 != NULL);

    if (str0->length != str1->length) {
        return false;
    }
    int i;
    for (i = 0; i < str0->length; i++) {
        if (string_at(str0, i) != string_at(str1, i)) {
            return false;
        }
    }
    return true;
}

void string_reserve(string_t* str, size_t size) {
    assert (str != NULL);

    while (_STR_CAPACITY(str) < size)
        str->capacity <<= 1;

    str->data = str->aif->realloc(str->data, str->capacity);
}

string_t* string_from_cstr(string_t* str, const char* cstr) {
    assert(str != NULL);
    assert(cstr != NULL);

    size_t len = strlen(cstr);
    string_reserve(str, len + 1);

    memcpy(_STR_DATA(str), cstr, len);
    str->length = len;
    str->data[len] = 0;

    return str;
}

const char* string_to_cstr(string_t* str) {
    assert(str != NULL);

    str->data[str->length] = 0;
    return _STR_DATA(str);
}

void string_copy_cstr(string_t* str, char* buf, size_t size) {
    assert(str != NULL);
    assert(buf != NULL);

    int cpsize = string_length(str);
    if (cpsize > size - 1) {
        cpsize = size - 1;
    }
    memcpy(buf, _STR_DATA(str), cpsize);
    buf[cpsize] = 0;
}


void string_from_int(string_t* str, int val) {
    assert(str != NULL);

    string_reserve(str, 15);
    snprintf(str->data, str->capacity, "%d", val);
    str->length = strlen(str->data);
}

void string_from_int64(string_t* str, int64_t val) {
    assert(str != NULL);

    string_reserve(str, 25);
    snprintf(str->data, str->capacity, "%"PRId64, val);
    str->length = strlen(str->data);
}

void string_from_double(string_t* str, double val) {
    assert(str != NULL);

    string_reserve(str, 50);
    snprintf(str->data, str->capacity, "%f", val);
    str->length = strlen(str->data);
}

int string_to_int(string_t* str) {
    assert(str != NULL);

    int val = 0;
    sscanf(str->data, "%d", &val);
    return val;
}

int64_t string_to_int64(string_t* str) {
    assert(str != NULL);

    int64_t val = 0;
    sscanf(str->data, "%"PRId64, &val);
    return val;
}

double string_to_double(string_t* str) {
    assert(str != NULL);

    double val = 0;
    sscanf(str->data, "%lf", &val);
    return val;
}

void string_append(string_t* str0, const string_t* str1) {
    assert(str0 != NULL);
    assert(str1 != NULL);

    string_reserve(str0, str0->length + str1->length + 1);

    memcpy(_STR_DATA(str0) + str0->length, _STR_DATA(str1), str1->length);
    str0->length += str1->length;
    str0->data[str0->length] = 0;
}

void string_append_cstr(string_t* str, const char* cstr) {
    assert(str != NULL);
    assert(cstr != NULL);

    size_t len = strlen(cstr);
    string_reserve(str, str->length + len + 1);

    memcpy(_STR_DATA(str) + str->length, cstr, len);
    str->length += len;
    str->data[str->length] = 0;
}


char string_at(const string_t* str, int pos) {
    assert(str != NULL);
    assert(pos < str->length);

    return str->data[pos];
}

void string_update(string_t* str, int pos, char ch) {
    assert(str != NULL);
    assert(pos < str->length);

    str->data[pos] = ch;
}

int string_indexof(const string_t* str, char find, int pos) {
    assert(str != NULL);

    int nowp;
    for (nowp = pos; nowp < str->length; nowp++) {
        RETURN_IF(string_at(str, nowp) == find, nowp);
    }
    return STRING_NPOS;
}
