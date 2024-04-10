// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#include "w3Frame.h"
#include "w3Label.h"
#include "w3Tag.h"
#include "w3TaggedValue.h"

struct StackValue
{
    // TODO remove two level tagging
    Tag tag;
    //union {
        TaggedValue value; // TODO: change to Value or otherwise remove redundant tag
        Frame* frame; // TODO by value? Probably not. This was changed
                      // to resolve circular types, and for the initial frame that seemed
                      // wrong, but now that call/ret being implemented, seems right
        //DecodedInstruction* instr;
        Label label;
    //};

    StackValue() : tag (Tag_none), frame (0)
    {
    }

    StackValue (Tag t) : tag (t), frame (0)
    {
        if ((t & 0x78) == 0x78)
        {
            tag = Tag_Value;
            value.tag = t; //todo: remove?
        }
    }

    StackValue (TaggedValue t) : tag (Tag_Value), frame (0)
    {
        value = t;
    }

    StackValue (Frame* f) : tag (Tag_Frame), frame (0)
    {
 //     frame = f;
    }
};
