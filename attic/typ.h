/*
This file is part of a C pipe dream.
The real code is C++.
*/
#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CONTAINER(addr, type, field) (((char*)(addr)) - offsetof(type, field))

struct Type;
typedef struct Type Type;

struct Type
{
    int64_t size;
    char* name;
    int (*init)(Type* type, void* data, size_t count);
    void (*cleanup)(Type* type, void* data, size_t count);
    int (*copy)(Type*, void* to, void* from, size_t count);
    void (*move)(Type*, void* to, void* from, size_t count);
};

void
Type_Init(Type* type, void* t);

void
Type_Copy(Type* type, void* to, const void* from);

#ifdef __cplusplus
} // extern "C"
#endif
