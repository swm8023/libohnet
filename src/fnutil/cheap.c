#include <assert.h>
#include <fnutil/cheap.h>

heap_t* _new_heap(const char* typestr, _heap_less lessfunc) {
    assert(typestr != NULL);

    heap_t* heap = (heap_t*)fn_malloc(sizeof(heap_t));
    RETURN_IF_NULL(heap, NULL);

    if (_init_heap(heap, typestr, lessfunc) < 0) {
        fn_free(heap);
        return NULL;
    }
    return heap;
}

int _init_heap(heap_t* heap, const char* typestr, _heap_less lessfunc) {
    assert(heap != NULL);
    assert(typestr != NULL);

    /* container base attr */
    heap->_less = lessfunc;

    /* use default type less function if lessfunc is NULL */
    if (lessfunc == NULL) {
        heap->_less = _get_type_bystr(typestr)->less;
    }

    /* less function must be set */
    RETURN_IF_NULL(heap->_less, -1);

    /* init contain vector */
    RETURN_IF(_init_vector(_HEAP_CVEC(heap), typestr) < 0, -1);

    return 0;
}

void destroy_heap(heap_t* heap) {
    assert(heap != NULL);

    destroy_vector(_HEAP_CVEC(heap));
}

void delete_heap(heap_t* heap) {
    assert(heap != NULL);

    destroy_vector(_HEAP_CVEC(heap));
    fn_free(heap);
}

void* heap_top(heap_t* heap) {
    assert(heap != NULL);
    assert(!heap_empty(heap));

    return vector_front(_HEAP_CVEC(heap));
}

void heap_top_val(heap_t* heap, void *pelm) {
    assert(heap != NULL);
    assert(!heap_empty(heap));

    _type_copy(_HEAP_CTYPE(heap), pelm, heap_top(heap));
}

void heap_push(heap_t* heap , ...) {
    va_list elm_arg;

    assert(heap != NULL);

    /* append new element to the last, then fix the heap */
    va_start(elm_arg, heap);
    _vector_push_back_varg(_HEAP_CVEC(heap), elm_arg);
    va_end(elm_arg);

    _heap_fix_up(heap, heap_size(heap) - 1);

}

void heap_pop(heap_t* heap) {
    assert(heap != NULL);
    assert(!heap_empty(heap));

    /* swap the first and last element,
       then remove last element and fix the heap*/
    _heap_swap(heap, 0, heap_size(heap) - 1);
    vector_pop_back(_HEAP_CVEC(heap));

    if (!heap_empty(heap)) {
        _heap_fix_down(heap, 0);
    }
}

/* private function */
void _heap_fix_up(heap_t* heap, int pos) {
    assert(heap != NULL);
    assert(heap_size(heap) > pos);
    while (pos > 0 && heap->_less(_HEAP_AT(heap, pos),
        _HEAP_AT(heap, _HEAP_PARENT(pos)))) {
        _heap_swap(heap, pos, _HEAP_PARENT(pos));
        pos = _HEAP_PARENT(pos);
    }
}


void _heap_fix_down(heap_t* heap, int pos) {
    assert(heap != NULL);
    assert(heap_size(heap) > pos);

    while (_HEAP_LCHILD(pos) < heap_size(heap)) {
        int npos = _HEAP_LCHILD(pos);
        npos += (npos + 1 < heap_size(heap) &&
            heap->_less(_HEAP_AT(heap, npos + 1), _HEAP_AT(heap, npos)) ? 1 : 0);
        if (heap->_less(_HEAP_AT(heap, pos), _HEAP_AT(heap, npos))) {
            break;
        }
        _heap_swap(heap, pos, npos);
        pos = npos;
    }
}

void _heap_swap(heap_t* heap, int p1, int p2) {
    assert(heap != NULL);
    /* the max index can be swap is heap size */
    assert(heap_size(heap) >= p1);
    assert(heap_size(heap) >= p2);

    _vector_swap_elem(_HEAP_CVEC(heap), p1, p2);
}

