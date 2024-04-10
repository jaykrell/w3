// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

//#include <vector> TODO: namespace

#include "w3StackValue.h"
#include "w3ExportInstance.h"

struct Module;
struct FuncAddr;
struct TableAddr;

struct ModuleInstance // work in progress
{
    ModuleInstance (Module* mod);
    ModuleInstance () : module(0) { }

    Module* module;
    std::vector <uint8_t> memory;
    std::vector <FuncAddr*> funcs;
    std::vector <TableAddr*> tables;
    //std::vector <MemAddr*> mem; // mem [0] => memory for now
    std::vector <StackValue> globals;
    std::vector <ExportInstance> exports;
};
