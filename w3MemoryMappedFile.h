// A WebAssembly codebase by Jay Krell

#pragma once

#include "w3.h"
#include "w3Handle.h"
#include "w3Fd.h"

struct MemoryMappedFile
{
// TODO allow for redirection to built-in data (i.e. filesystem emulation with builtin BCL)
// TODO allow for systems that must read, not mmap
    void* base {};
    size_t size {};
#ifdef _WIN32
    Handle file {};
#else
    Fd file {};
#endif

    ~MemoryMappedFile ();

    void read (PCSTR a);
};
