/*
This file is part of a C pipe dream.
The real code is C++.
*/
#pragma once

#include "typ.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Vec;
typedef struct Vec Vec;

struct Vec
{
    void* allocator; // future
    Type* type;
    char* start;
    char* end;
    char* allocated;
};

int
Vec_PushBack(Vec* v, const void* elem, size_t count); // return is negative for error

void
Vec_PopBack(Vec* v, size_t count);

char*
Vec_At(Vec* v, size_t index);

size_t
Vec_Size(Vec* v);

size_t
Vec_ByteSize(Vec* v);

int
Vec_Resize(Vec* v, size_t); // return is negative for error

size_t
Vec_Reserve(Vec* v);

#ifdef __cplusplus
} // extern "C"
#endif
