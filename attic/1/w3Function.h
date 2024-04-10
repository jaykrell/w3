// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

struct Function // section3
{
    // Functions are split between two sections: types in section3, locals/body in section10
    size_t function_index {}; // TODO needed?
    size_t function_type_index {};
    size_t local_only_count {};
    size_t param_count {};
    bool import {}; // TODO needed?
};
