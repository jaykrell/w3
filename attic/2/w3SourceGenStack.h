// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

//#include <stack> TODO: namespace
//#include <string>
#include "w3SourceGenValue.h"

struct SourceGenStack : std::stack<SourceGenValue>
{
    typedef std::stack<SourceGenValue> base;

    void clear()
    {
        while (size()) pop();
    }

    PCSTR cstr() { return top ().str.c_str (); }

    std::string pop ()
    {
        std::string str = top ().str;
        base::pop();
        return str;
    }

    void push_label (...) { } //todo
};
