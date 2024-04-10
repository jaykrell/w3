// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

struct SectionTraits
{
    void (Module::*read)(uint8_t** cursor);
    PCSTR name;
};

extern const SectionTraits section_traits [ ];
