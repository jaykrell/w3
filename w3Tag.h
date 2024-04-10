// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

// Wasm has several notions of type tags.
// Some are in the file format, some are useful
// to interpreters or codegenerators (probably
// none are useful to generated code).
//
// Since there are not all that many, and there should
// be contradictory requirements on their values,
// unify them for a sort of simplicity (sort of complexity,
// since it is confusing which to deal with).
//
// WebAssembly file format defines at least the values 0x7C-0x7F.
// We desire simplicity and clarity, so we shall endeavor
// to not do much translation or compression.
typedef enum w3Tag : uint8_t
{
    Tag_none = 0,   // allow for zero-init
    Tag_bool = 1,   // aka i32
    Tag_any  = 2,   // often has constraints

    // These are from the file format. These are the primary
    // types in WebAssembly, prior to the addition of SIMD.
    Tag_i32 = 0x7F,
    Tag_i64 = 0x7E,
    Tag_f32 = 0x7D,
    Tag_f64 = 0x7C,

    //todo: comment
    Tag_empty = 0x40, // ResultType, void

    //internal, any value works..not clearly needed
    // A heterogenous conceptual WebAssembly stack contains Values, Frames, and Labels.
    Tag_Value = 0x80,   // i32, i64, f32, f64
    Tag_Label = 0x81,   // branch target
    Tag_Frame = 0x82,   // return address + locals + params

    //todo: comment
    Tag_FuncRef = 0x70,

    //codegen temp
    //Tag_intptr = 3,
    //Tag_uintptr = 4,
    //Tag_pch = 5,
} w3Tag;

char TagChar(w3Tag t); //todo: short string?
