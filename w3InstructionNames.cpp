// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#include "w3.h"
#include "w3InstructionNames.h"

#if 0 // Split string up for old compiler.

extern const char instructionNames [ ] =
#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) #name "\0"
#include "w3instructions.h"
;
#else

extern const UInstructionNames instructionNames = { {
#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) #name,
#include "w3instructions.h"
} };

#endif

#define InstructionName(i) (&instructionNames.data [instructionEncode [i].string_offset])
