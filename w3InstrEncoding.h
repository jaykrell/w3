// A WebAssembly implementation and experimentation platform.
// portable
// simple? Always striving for the right level of complexity -- not too simple.
// efficient? (not yet)

// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#include "w3InstrNames.h"

enum InstructionEnum;
typedef struct Module Module;

struct InstructionEncoding
{
    uint8_t byte0;
    //uint8_t byte1;              // FIXME always 0 if fixed_size > 1
    uint8_t fixed_size    : 2;    // 0, 1, 2
    Immediate immediate;
    uint8_t pop           : 2;    // required minimum stack in
    uint8_t push          : 1;
    InstructionEnum name;
    uint32_t string_offset : bits_for_uint (sizeof (instructionNames));
    Tag stack_in0  ; // type of stack [0] upon input, if pop >= 1
    Tag stack_in1  ; // type of stack [1] upon input, if pop >= 2
    Tag stack_in2  ; // type of stack [2] upon input, if pop == 3
    Tag stack_out0 ; // type of stack [1] upon input, if push == 1
    void (*interp) (Module*); // Module* wrong
};

// index by byte, first byte in instruction (too bad it is bytecode not shortcode)
extern const InstructionEncoding instructionEncode [];
