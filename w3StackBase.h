// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#include "w3StackBaseBase.h"
#include "w3StackValue.h"

struct w3StackBase : private w3StackBaseBase
{
    typedef w3StackBaseBase base;
    typedef base::iterator iterator;
    w3StackValue& back () { return base::back (); }
    w3StackValue& front () { return base::front (); }
    iterator begin () { return base::begin (); }
    iterator end () { return base::end (); }
    bool empty () const { return base::empty (); }
    void resize (size_t newsize) { base::resize (newsize); }
    size_t size () { return base::size (); }
    w3StackValue& operator [ ] (size_t index) { return base::operator [ ] (index); }

    void push (const w3StackValue& a)
    {   // While ultimately a stack of values, labels, and frames, values dominate,
        // so the usage is made convenient for them.
        push_back (a);
    }

    void push_i32(...) // todo
    {
    }

    void push_i64(...) // todo
    {
    }

    void push_f32(...) // todo
    {
    }

    void push_f64(...) // todo
    {
    }

    void pop ()
    {
        pop_back ();
    }

    w3StackValue& top ()
    {   // While ultimately a stack of values, labels, and frames, values dominate,
        // so the usage is made convenient for them.
        return back ();
    }
};
