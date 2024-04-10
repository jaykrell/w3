// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

struct w3FuncAddr;
struct w3TableAddr;
struct w3MemAddr;
struct w3GlobalAddr;

struct w3ExternalValue // external to a module, an export instance
{
    union
    {
        w3FuncAddr* func;
        w3TableAddr* table;
        w3MemAddr* mem;
        w3GlobalAddr* global;
    };
};

