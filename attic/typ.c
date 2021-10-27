/*
This file is part of some C pipe dream.
The real code is C++.
*/
#include "typ.h"

void
Type_Init(Type* type, void* data, size_t count)
{
    memset(t, 0, type->size * count);
}

void
Type_Copy(void* to, const void* from, size_t count, Type* type)
{
    if (type->copy)
        type->copy(type, to, from, count);
    else
        memcpy(to, from, type->size * count);
}
