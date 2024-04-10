// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

//#include <vector> TODO: namespace

#include "w3StackValue.h"
#include "w3ExportInstance.h"

struct w3Module;
struct w3FuncAddr;
struct w3TableAddr;

struct w3ModuleInstance // work in progress
{
    w3ModuleInstance (w3Module* mod);
    w3ModuleInstance () : module(0) { }

    w3Module* module;
    std::vector <uint8_t> memory;
    std::vector <w3FuncAddr*> funcs;
    std::vector <w3TableAddr*> tables;
    //std::vector <w3MemAddr*> mem; // mem [0] => memory for now
    std::vector <w3StackValue> globals;
    std::vector <w3ExportInstance> exports;
};
