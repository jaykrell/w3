// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

struct TaggedValue
{
    w3Tag tag;
    Value value;

    TaggedValue()
    {
        memset (this, 0, sizeof(*this));
    }
};
