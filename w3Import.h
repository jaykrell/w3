// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#include "w3WasmString.h"
#include "w3ImportTag.h"
#include "w3TableType.h"
#include "w3Memorytype.h"
#include "w3GlobalType.h"

struct w3Import
{
    WasmString module {};
    WasmString name {};
    w3ImportTag tag {(ImportTag)-1};
    // TODO virtual functions to model union
    //union
    //{
        w3TableType table {};
        uint32_t function {};
        w3MemoryType memory {};
        w3GlobalType global {};
    //};
};
