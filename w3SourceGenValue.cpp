// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

//#include <string> TODO: namespace
#include "w3Tag.h"

// TODO templatize on existance of string and possibly label
// so that w3SourceGen and Interp can share.
struct w3SourceGenValue
{
    w3Tag tag;
    std::string str;
    PCSTR cstr() { return str.c_str(); }
};
