// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#include "w3Tag.h"
#include "w3Limits.h"

struct TableType
{
    Tag elementType {};
    Limits limits {};
};
