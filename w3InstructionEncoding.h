// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#include "w3InstructionNames.h"
#include "w3InstructionEnum.h"

typedef struct w3Module w3Module;

struct w3InstructionEncoding
{
    uint8_t byte0;
    //uint8_t byte1;              // FIXME always 0 if fixed_size > 1
    uint8_t fixed_size    : 2;    // 0, 1, 2
    Immediate immediate;
    uint8_t pop           : 2;    // required minimum stack in
    uint8_t push          : 1;
    w3InstructionEnum name;
    uint32_t string_offset : bits_for_uint (sizeof (instructionNames));
    w3Tag stack_in0  ; // type of stack [0] upon input, if pop >= 1
    w3Tag stack_in1  ; // type of stack [1] upon input, if pop >= 2
    w3Tag stack_in2  ; // type of stack [2] upon input, if pop == 3
    w3Tag stack_out0 ; // type of stack [1] upon input, if push == 1
    void (*interp) (w3Module*); // w3Module* wrong
};

// index by byte, first byte in instruction (too bad it is bytecode not shortcode)
extern const w3InstructionEncoding instructionEncode [];
