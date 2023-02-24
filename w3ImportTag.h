// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

typedef enum ImportTag {    // aka desc
    ImportTag_Function = 0, // aka type
    ImportTag_Table = 1,
    ImportTag_Memory = 2,
    ImportTag_Global = 3,
} ImportTag, ExportTag;
