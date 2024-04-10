// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#include "w3.h"

#undef PopulationCount64
#undef PopulationCount32

#if _MSC_VER
extern __forceinline
#endif
uint32_t PopulationCount64 (uint64_t a)
{
    //todo intrinsics and portable performance and constant time

    uint64_t c = 0;
    const uint64_t b = 1;

    while (a)
    {
        c += (a & b);
        a >>= 1;
    }

    return (uint32_t)c;
}

#if _MSC_VER
extern __forceinline
#endif
uint32_t PopulationCount32 (uint32_t a)
{
    return PopulationCount64 (a);
}

#if _MSC_VER
extern __forceinline
#endif
uint32_t CountTrailingZeros64 (uint64_t a)
{
    //todo intrinsics and portable performance and constant time

    uint64_t c = 0;

    while ((a & 1) == 0 && c < 64)
    {
        a >>= 1;
        ++c;
    }
    return (uint32_t)c;
}

#if _MSC_VER
extern __forceinline
#endif
uint32_t CountTrailingZeros32 (uint32_t a)
{
    uint64_t b = a;
    return CountTrailingZeros64 (b) & 31;
}

#if _MSC_VER
extern __forceinline
#endif
uint32_t CountLeadingZeros64 (uint64_t a)
{
    //todo intrinsics and portable performance and constant time

    uint64_t c = 0;
    const uint64_t b = ((uint64_t)1) << 63;

    while ((a & b) == 0 && c < 64)
    {
        a <<= 1;
        ++c;
    }
    return (uint32_t)c;
}

#if _MSC_VER
extern __forceinline
#endif
uint32_t CountLeadingZeros32 (uint32_t a)
{
    uint64_t b = a;
    return CountLeadingZeros64 (b << 32) & 31;
}

#if 0

int main()
{
#define X2(x) printf("%s:%d\n", #x, x);
#define X(x) X2(x(1)); \
    X2(x(0)); X2(x(2)); X2(x(3)); X2(x(4)); X2(x(((int64_t)-1))); \
    X2(x(((int64_t)1) << 30)); \
    X2(x(((int64_t)1) << 33)); \
    X2(x(((int64_t)7) << 30)); \
    X2(x(((int64_t)7) << 33)); \
    X2(x(((int64_t)~0) << 30)); \
    X2(x(((int64_t)~0) << 33)); \
    X2(x(0x80000000)); \
    X2(x(0x70000000)); \
    X2(x(0xF0000000)); \
    X2(x(0x08000000)); \
    X2(x(0x07000000)); \
    X2(x(0x0F000000)); \
    X2(x(0xF80000000)); \
    X2(x(0xF70000000)); \
    X2(x(0xFF0000000)); \
    X2(x(0xF08000000)); \
    X2(x(0xF07000000)); \
    X2(x(0xF0F000000)); \
    X2(x(((int64_t)-2))); \

    X(CountLeadingZeros32);
    X(CountLeadingZeros64);
    X(CountTrailingZeros32);
    X(CountTrailingZeros64);
    X(PopulationCount32);
    X(PopulationCount64);
}

#endif
