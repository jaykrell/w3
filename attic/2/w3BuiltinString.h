// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

typedef enum BuiltinString {
    BuiltinString_none = 0,
    BuiltinString_main,
    BuiltinString_start,
} BuiltinString;
