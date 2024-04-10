// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

union Value
{
    int32_t i32;
    uint32_t u32;
    uint64_t u64;
    int64_t i64;
    float f32;
    double f64;
};
