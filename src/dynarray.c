#include <stdlib.h>

#include "dynarray.h"

struct dynarray {
    void** data;
    int size;
    int capacity;
};

struct dynarray* dynarray_create() {
    struct dynarray* da = (struct dynarray*) malloc(sizeof(struct dynarray));
    da->data = (void*) malloc(sizeof(void*) * 2);
    da->size = 0;
    da->capacity = 2;

    return da;
}

void dynarray_free(struct dynarray* da) {
    free(da->data);
    free(da);
    return;
}

int dynarray_size(struct dynarray* da) {
    return da->size;
}

void dynarray_insert(struct dynarray* da, void* val) {
    if (da != NULL)
    {
        if (da->size >= da->capacity)
        {
            void** temparr = (void*) malloc(da->size * sizeof(void*) * 2);
            for (int i = 0; i < da->size; i++)
                temparr[i] = da->data[i];
            free(da->data);
            da->data = temparr;
            da->capacity *= 2;
        }
        da->data[da->size] = val;
        da->size++;
    }
    return;
}

void dynarray_remove(struct dynarray* da, int idx) {
    if (idx < da->size && idx >= 0)
    {
        for (int i = idx + 1; i < da->size; ++i)
            da->data[i - 1] = da->data[i];
        da->size--;
    }
    return;
}

void* dynarray_get(struct dynarray* da, int idx) {
    if (idx < da->size && idx >= 0)
        return da->data[idx];
    return NULL;
}

void dynarray_set(struct dynarray* da, int idx, void* val) {
    if (idx < da->size && idx >= 0)
        da->data[idx] = val;
    return;
}

void** dynarray_raw(struct dynarray* da)
{
    void** raw = malloc(sizeof(void*) * (da->size + 1));
    int i;
    for (i = 0; i < dynarray_size(da); ++i)
        raw[i] = dynarray_get(da, i);
    raw[da->size] = NULL;
    return raw;
}
