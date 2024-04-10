// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

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
    w3Handle file {};
#else
    Fd file {};
#endif

    ~MemoryMappedFile ();

    void read (PCSTR a);
};
