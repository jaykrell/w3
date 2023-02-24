// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#include <string>
#include "w3Tag.h"

// TODO templatize on existance of string and possibly label
// so that SourceGen and Interp can share.
struct SourceGenValue
{
    Tag tag;
    std::string str;
    PCSTR cstr() { return str.c_str(); }
};
