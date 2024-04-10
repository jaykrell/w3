// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

struct w3DecodedInstruction;

// TODO once we have Validate, Interp, Jit, CppGen,
// we might invert this structure and have a class per instruction with those 4 virtual functions.
// Or we will token-paste those names on to the instruction names,
// in order to avoid virtual function call cost. Let's get Interp working first.
struct Wasm
{
    w3DecodedInstruction* instr; // TODO make local variable

    Wasm () : instr (0) { }

    virtual ~Wasm () { }

    virtual void Reserved () = 0;
#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) void name () { abort (); }
#include "w3Instructions.h"
};
