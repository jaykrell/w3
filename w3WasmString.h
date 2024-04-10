// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#include "w3BuiltinString.h"

//todo eliminate this? remove std::string?
struct WasmString
{
    PCH data {};
    size_t size {};
    std::string storage {};
    w3BuiltinString builtin {};
    bool builtinStorage {};

    PCH c_str ()
    {
        if (!data)
        {
            data = (PCH)storage.c_str ();
        }
        else if (data != storage.c_str ())
        {
            storage = std::string (data, size);
            data = (PCH)storage.c_str ();
        }
        return data;
    }
};
