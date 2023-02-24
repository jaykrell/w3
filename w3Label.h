// A WebAssembly implementation and experimentation platform.
// portable
// simple? Always striving for the right level of complexity -- not too simple.
// efficient? (not yet)

// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

struct Label
{
    Label() : arity(0), continuation(0)
    {
    }

    size_t arity;
    size_t continuation;
};
