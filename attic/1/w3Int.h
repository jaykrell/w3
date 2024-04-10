// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

uint64_t SignExtend (uint64_t value, uint32_t bits);

size_t IntMagnitude (ssize_t i);

uint32_t UIntGetPrecision (uint64_t a);

uint32_t IntGetPrecision (int64_t a);

uint32_t UIntToDec_GetLength (uint64_t b);

uint32_t UIntToDec (uint64_t a, PCH buf);

uint32_t IntToDec (int64_t a, PCH buf);

uint32_t IntToDec_GetLength (int64_t a);

uint32_t UIntToHex_GetLength (uint64_t b);

uint32_t IntToHex_GetLength (int64_t a);

void UIntToHexLength (uint64_t a, uint32_t len, PCH buf);

void IntToHexLength (int64_t a, uint32_t len, PCH buf);

uint32_t IntToHex (int64_t a, PCH buf);

uint32_t IntToHex8 (int64_t a, PCH buf);

uint32_t IntToHex_GetLength_AtLeast8 (int64_t a);

uint32_t UIntToHex_GetLength_AtLeast8 (uint64_t a);

uint32_t IntToHex_AtLeast8 (int64_t a, PCH buf);

uint32_t UIntToHex_AtLeast8 (uint64_t a, PCH buf);
