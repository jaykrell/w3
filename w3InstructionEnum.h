// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) name,

typedef enum InstructionEnum
{
#include "w3Instructions.h"
} InstructionEnum;

#undef INSTRUCTION
