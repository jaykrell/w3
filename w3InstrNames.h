// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) char name [ sizeof (#name) ];
struct InstructionNames
{
#include "w3instructions.h"
};

#if 0 // Split string up for old compiler.
const char instructionNames [ ] =
#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) #name "\0"
#include "w3instructions.h"
;
#else

union UInstructionNames
{
    InstructionNames x;
    char data [1];
};

extern const UInstructionNames instructionNames;

#endif

#define InstructionName(i) (&instructionNames.data [instructionEncode [i].string_offset])
