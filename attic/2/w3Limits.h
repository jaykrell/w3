// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

struct Limits
{
    // TODO size_t? null?
    uint32_t min {};
    uint32_t max {};
    bool hasMax {};
};
