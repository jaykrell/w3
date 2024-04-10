// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#include "w3ExternalValue.h"
#include "w3WasmString.h"

struct w3ExportInstance // work in progress
{
    WasmString name;
    w3ExternalValue external_value;
};
