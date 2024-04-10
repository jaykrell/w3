// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

struct ModuleInstance;
struct FunctionType;
struct w3Code;

struct FunctionInstance // work in progress
{
    ModuleInstance* module_instance;
    FunctionType* function_type;
    void* host_code; // TODO
    w3Code* code; // TODO
};
