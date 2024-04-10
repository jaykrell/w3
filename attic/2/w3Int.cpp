// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#include "w3.h"
#include "w3Int.h"

uint64_t SignExtend (uint64_t value, uint32_t bits)
{
    // Extract lower bits from value and signextend.
    // From detour_sign_extend.
    const uint32_t left = 64 - bits;
    const uint64_t m1 = (uint64_t)(int64_t)-1;
    const int64_t wide = (int64_t)(value << left);
    const uint64_t sign = (wide < 0) ? (m1 << left) : 0;
    return value | sign;
}

size_t IntMagnitude (ssize_t i)
{
    // Avoid negating the most negative number.
    return 1 + (size_t)-(i + 1);
}

struct IntSplitSignMagnitude_t
{
    IntSplitSignMagnitude_t (int64_t a)
    : is_negative ((a < 0) ? 1u : 0u),
        magnitude ((a < 0) ? (1 + (uint64_t)-(a + 1)) // Avoid negating most negative number.
                  : (uint64_t)a) { }

    uint32_t is_negative;
    uint64_t magnitude;
};

uint32_t UIntGetPrecision (uint64_t a)
{
    // How many bits needed to represent.
    uint32_t len = 1;
    while ((len <= 64) && (a >>= 1)) ++len;
    return len;
}

uint32_t IntGetPrecision (int64_t a)
{
    // How many bits needed to represent.
    // i.e. so leading bit is extendible sign bit, or 64
    return std::min (64u, 1 + UIntGetPrecision (IntSplitSignMagnitude_t (a).magnitude));
}

uint32_t UIntToDec_GetLength (uint64_t b)
{
    uint32_t len = 0;
    do ++len;
    while (b /= 10);
    return len;
}

uint32_t UIntToDec (uint64_t a, PCH buf)
{
    uint32_t const len = UIntToDec_GetLength (a);
    for (uint32_t i = 0; i != len; ++i, a /= 10)
        buf [i] = "0123456789" [a % 10];
    return len;
}

uint32_t IntToDec (int64_t a, PCH buf)
{
    const IntSplitSignMagnitude_t split (a);
    if (split.is_negative)
        *buf++ = '-';
    return split.is_negative + UIntToDec (split.magnitude, buf);
}

uint32_t IntToDec_GetLength (int64_t a)
{
    const IntSplitSignMagnitude_t split (a);
    return split.is_negative + UIntToDec_GetLength (split.magnitude);
}

uint32_t UIntToHex_GetLength (uint64_t b)
{
    uint32_t len = 0;
    do ++len;
    while (b >>= 4);
    return len;
}

uint32_t IntToHex_GetLength (int64_t a)
{
    // If negative and first digit is <8, add one to induce leading 8-F
    // so that sign extension of most significant bit will work.
    // This might be a bad idea. TODO.
    uint64_t b = (uint64_t)a;
    uint32_t len = 0;
    uint64_t most_significant;
    do ++len;
    while ((most_significant = b), b >>= 4);
    return len + (a < 0 && most_significant < 8);
}

void UIntToHexLength (uint64_t a, uint32_t len, PCH buf)
{
    buf += len;
    for (uint32_t i = 0; i != len; ++i, a >>= 4)
        *--buf = "0123456789ABCDEF" [a & 0xF];
}

void IntToHexLength (int64_t a, uint32_t len, PCH buf)
{
    UIntToHexLength ((uint64_t)a, len, buf);
}

uint32_t IntToHex (int64_t a, PCH buf)
{
    uint32_t const len = IntToHex_GetLength (a);
    IntToHexLength (a, len, buf);
    return len;
}

uint32_t IntToHex8 (int64_t a, PCH buf)
{
    IntToHexLength (a, 8, buf);
    return 8;
}

uint32_t IntToHex_GetLength_AtLeast8 (int64_t a)
{
    uint32_t len = IntToHex_GetLength (a);
    return std::max (len, 8u);
}

uint32_t UIntToHex_GetLength_AtLeast8 (uint64_t a)
{
    uint32_t const len = UIntToHex_GetLength (a);
    return std::max (len, 8u);
}

uint32_t IntToHex_AtLeast8 (int64_t a, PCH buf)
{
    uint32_t const len = IntToHex_GetLength_AtLeast8 (a);
    IntToHexLength (a, len, buf);
    return len;
}

uint32_t UIntToHex_AtLeast8 (uint64_t a, PCH buf)
{
    uint32_t const len = UIntToHex_GetLength_AtLeast8 (a);
    UIntToHexLength (a, len, buf);
    return len;
}
