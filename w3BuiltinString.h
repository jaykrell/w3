// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

typedef enum w3BuiltinString {
    w3BuiltinString_none = 0,
    w3BuiltinString_main,
    w3BuiltinString_start,
} w3BuiltinString;
