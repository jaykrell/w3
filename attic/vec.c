/*
This file is part of some C pipe dream.
The real code is C++.
*/
#include <stdlib.h>
#include "vec.h"

size_t Vec_SizeBytes(Vec* vec)
{
    return vec->end - vec->start;
}

size_t Vec_CapacityBytes(Vec* vec)
{
    return vec->allocated - vec->start;
}

int Vec_PushBackOne(Vec* vec, char* elem) // return is negative for error
{
    return Vec_PushBackMultiple(vec, elem, 1);
}

void Vec_PopBack(Vec* vec, size_t count)
{
    if (count > 0)
    {
        Type* type = vec->type;
        size_t type_size = type->size;
        type->cleanup(type, vec->end -= count * type_size, count);
    }
}

int Vec_ResizeBytes(Vec* vec, size_t new_size)
{
    Type* type = vec->type;
    size_t type_size = type->size;
    size_t new_count = new_size / type_size;
    size_t old_size = Vec_SizeBytes(vec);
    size_t old_count = old_size / type_size;

    if (new_size == old_size)
        return 0;

    if (new_count < old_count)
    {
        Vec_PopBack(vec, old_count - new_count);
        return 0;
    }

    size_t allocated_size = Vec_CapacityBytes(vec);
    if (allocated_size >= new_size)
    {
        type->init(type, vec->start + old_size, new_count - old_count);
        vec->end += new_size - old_size;
        return 0;
    }

    size_t new_alloc = allocated_size * 2;
    if (new_alloc < new_size)
        new_alloc = new_size;

    char* n = (char*)malloc(new_alloc);
    if (!n)
        return -1;

    type->move(type, n, vec->start, old_count);
    type->init(type, n + old_size, new_count - old_count);

    char* old_start = vec->start;
    vec->start = n;
    vec->end = n + new_size;
    vec->allocated = n + new_alloc;
    free(old_start);
    return 0;
}

int
Vec_PushBackMultiple(Vec* vec, char* elem, size_t count) // return is negative for error
{
    Type* type = vec->type;
    size_t type_size = type->size;
    size_t old_size = Vec_SizeBytes(vec);

    // TODO alloc vs. inited

    int err = Vec_ResizeBytes(vec, old_size + type_size * count);
    if (err)
        return err;

    err = type->copy(type, vec->start + old_size * type_size, elem, count * type_size);
    if (!err)
        return 0;

    // TODO rollback
    return 0;
}

char* Vec_At(Vec* vec, size_t i);

int Vec_Resize(Vec* vec, size_t); // return is negative for error
size_t Vec_Reserve(Vec* vec);
