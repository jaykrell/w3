// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#include "w3Tag.h"

struct w3GlobalType
{
    w3Tag value_type {};
    bool is_mutable {};
};
