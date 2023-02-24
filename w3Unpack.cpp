// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#include "w3-1.h"

uint32_t Unpack2 (const void* a)
{
    uint8_t* b = (uint8_t*)a;
    return ((b [1]) << 8) | (uint32_t)b [0];
}

uint32_t Unpack4 (const void* a)
{
    return (Unpack2 ((PCH)a + 2) << 16) | Unpack2 (a);
}

