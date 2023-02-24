// A WebAssembly implementation and experimentation platform.
// portable
// simple? Always striving for the right level of complexity -- not too simple.
// efficient? (not yet)

// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#define _CRT_SECURE_NO_WARNINGS 1

#ifdef _MSC_VER
#pragma warning (disable:4100) // unused parameter
#pragma warning (disable:4127) // conditional expression is constant
#pragma warning (disable:4201) // nameless struct/union
#pragma warning (disable:4355) // this used in base member initializer list
#pragma warning (disable:4365) // integer type mixups
#pragma warning (disable:4371) // layout change from previous compiler version
#pragma warning (disable:4480) // non-standard extension
#pragma warning (disable:4505) // unused static function
#pragma warning (disable:4514) // unused function
#pragma warning (disable:4571) // catch(...)
#pragma warning (disable:4616) // disable unknown warning (for older compiler)
#pragma warning (disable:4619) // disable unknown warning (for older compiler)
#pragma warning (disable:4625) // copy constructor implicitly deleted
#pragma warning (disable:4626) // assignment implicitly deleted
#pragma warning (disable:4668) // #if not_defined vs. #if 0
#pragma warning (disable:4710) // function not inlined
#pragma warning (disable:4774) // printf used without constant format
#pragma warning (disable:4820) // padding added
#pragma warning (disable:5032) // pragma warning push is balanced elsewhere
#pragma warning (disable:5039) // exception handling and function pointers
#pragma warning (disable:5045) // compiler will/did insert Spectre mitigation
#pragma warning (disable:5264) // const variable not used
#endif

#define _WASI_EMULATED_MMAN
#define WIN32_LEAN_AND_MEAN 1

#if _WIN32
#define BIG_ENDIAN      2
#define LITTLE_ENDIAN   1
#define BYTE_ORDER      LITTLE_ENDIAN
#else
#include <endian.h>
#define BIG_ENDIAN      __BIG_ENDIAN
#define LITTLE_ENDIAN   __LITTLE_ENDIAN
#define BYTE_ORDER      __BYTE_ORDER
#endif

#include <math.h>
#include <limits.h>

// Win32 ZeroMemory
#define ZeroMem(p, n) memset((p), 0, (n))

#include "w3StdInt.h"
#include "w3StdFunc.h"
#include "ieee.h"
#include "math_private.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern const float wasm_hugef;
extern const double wasm_huged;

float wasm_truncf (float x);
double wasm_truncd (double x);
float wasm_roundf (float x);
double wasm_roundd (double x);
float wasm_floorf (float x);
//double wasm_floord (double x);
int wasm_isinff (float x);
int wasm_isinfd (double x);
int wasm_isnanf (float x);
int wasm_isnand (double x);

#ifdef __cplusplus
} // extern "C"
#endif
