// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#include "w3WasmString.h"

struct Section
{
    uint32_t id {};
    WasmString name {};
    size_t payload_size {};
    uint8_t* payload {};
};
