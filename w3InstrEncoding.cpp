// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#include "w3.h"
#include "w3InstructionEnum.h"
#include "w3InstrEncoding.h"
#include "w3InstrNames.h"

#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, pop, push, in0, in1, in2, out0) { byte0, fixed_size, imm, pop, push, name, offsetof (InstructionNames, name), in0, in1, in2, out0 },

extern const InstructionEncoding instructionEncode [ ] =
{
#include "w3instructions.h"
};

static_assert (sizeof (instructionEncode) / sizeof (instructionEncode [0]) == 256, "not 256 instructions");
