// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#define BITS_FOR_UINT_HELPER(a, x) (a) >= (1u << x) ? (x) + 1 :

#define BITS_FOR_UINT(a)                                                                                \
  (BITS_FOR_UINT_HELPER (a, 31) BITS_FOR_UINT_HELPER (a, 30)                                                          \
   BITS_FOR_UINT_HELPER (a, 29) BITS_FOR_UINT_HELPER (a, 28) BITS_FOR_UINT_HELPER (a, 27) BITS_FOR_UINT_HELPER (a, 26) BITS_FOR_UINT_HELPER (a, 25) \
   BITS_FOR_UINT_HELPER (a, 24) BITS_FOR_UINT_HELPER (a, 23) BITS_FOR_UINT_HELPER (a, 22) BITS_FOR_UINT_HELPER (a, 21) BITS_FOR_UINT_HELPER (a, 20) \
   BITS_FOR_UINT_HELPER (a, 19) BITS_FOR_UINT_HELPER (a, 18) BITS_FOR_UINT_HELPER (a, 17) BITS_FOR_UINT_HELPER (a, 16) BITS_FOR_UINT_HELPER (a, 15) \
   BITS_FOR_UINT_HELPER (a, 14) BITS_FOR_UINT_HELPER (a, 13) BITS_FOR_UINT_HELPER (a, 12) BITS_FOR_UINT_HELPER (a, 11) BITS_FOR_UINT_HELPER (a, 10) \
   BITS_FOR_UINT_HELPER (a,  9) BITS_FOR_UINT_HELPER (a,  8) BITS_FOR_UINT_HELPER (a,  7) BITS_FOR_UINT_HELPER (a,  6) BITS_FOR_UINT_HELPER (a,  5) \
   BITS_FOR_UINT_HELPER (a,  4) BITS_FOR_UINT_HELPER (a,  3) BITS_FOR_UINT_HELPER (a,  2) BITS_FOR_UINT_HELPER (a,  1) BITS_FOR_UINT_HELPER (a,  0) 1)

#if 1 // (_MSC_VER && _MSC_VER <= 1700) || defined(__WATCOMC__)

#define bits_for_uint(x) BITS_FOR_UINT (x)

#else

constexpr uint32_t bits_for_uint (uint32_t a)
{
    return
#define X(x) (a < (1u << x)) ? x :
    X( 1) X( 2) X( 3) X( 4)
    X( 5) X( 6) X( 7) X( 8)
    X( 9) X(10) X(11) X(12)
    X(13) X(14) X(15) X(16)
    X(17) X(18) X(19) X(20)
    X(21) X(22) X(23) X(24)
    X(25) X(26) X(27) X(28)
    X(29) X(30) X(31)
#undef X
    32;
}

#endif
