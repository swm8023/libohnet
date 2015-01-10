#include <string.h>
#include <stdio.h>

#include <ohutil/spcontainer.h>
#include <ohutil/memory.h>


typedef struct _tag_memdbg_rec {
    SPLST_DEFNEXT(struct _tag_memdbg_rec);
    const char* file;
    int line;
    char *block;
    char *ptr;
    size_t size;
}_memdbg_rec;
static _memdbg_rec *_memdbg_rec_head = NULL;



void *_oh_memdbg_alloc(void *ptr, size_t size , int init, const char* const file, int line) {
    /* allocate memory => |pack|memdbg_rec|using memory|pack| */
    int realsize = size + sizeof(_memdbg_rec) + _MEMDBG_ALIGNSIZE;
    char *block = (char*) (init ? calloc(1, realsize) : malloc(realsize));
    if (block == NULL) {
        return NULL;
    }

    /* let using memory align to ALIGNMENT */
    char *retptr = (char*)(((size_t)block + _MEMDBG_ALIGNSIZE +
        sizeof(_memdbg_rec)) & ~(_MEMDBG_ALIGNSIZE - 1));
    _memdbg_rec *rec = (_memdbg_rec*) ((char*)retptr - sizeof(_memdbg_rec));

    /* copy to new memory and free old memory if realloc */
    if (ptr != NULL) {
        _memdbg_rec* old_rec = (_memdbg_rec*)((char*)ptr - sizeof(_memdbg_rec));
        memcpy(retptr, old_rec->ptr, old_rec->size);
        _oh_memdbg_free(ptr);
    }

    /* init memdbg_rec */
    rec->file  = file;
    rec->line  = line;
    rec->block = block;
    rec->ptr   = retptr;
    rec->size  = size;
    splst_add(_memdbg_rec_head, rec);
    return retptr;
}

void _oh_memdbg_free(void *ptr) {
    _memdbg_rec* old_rec = (_memdbg_rec*)((char*)ptr - sizeof(_memdbg_rec));
    splst_del(_memdbg_rec_head, old_rec);
    free(old_rec->block);
}

int oh_memdbg_get_recnum() {
    return splst_len(_memdbg_rec_head);
}

void oh_memdbg_print_rec() {
    _memdbg_rec *rec = NULL;
    int tot = 0;
    FOR_SPLST_START(_memdbg_rec_head, rec)
        printf("memory leak at %p, size %zu. (%s:%d)\n", rec->ptr,
            rec->size, rec->file, rec->line);
    tot++;
    FOR_SPLST_END()
    printf("totol %d memory leak.\n", tot);
}