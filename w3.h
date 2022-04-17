// A WebAssembly implementation and experimentation platform.
// portable
// simple? Always striving for the right level of complexity -- not too simple.
// efficient? (not yet)

// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#define _CRT_SECURE_NO_WARNINGS 1

#if _MSC_VER
#pragma warning (disable:4127) // conditional expression is constant
#pragma warning (disable:4365) // integer type mixups
#pragma warning (disable:4480) // non-standard extension
#pragma warning (disable:4571) // catch(...)
#pragma warning (disable:4616) // disable unknown warning (for older compiler)
#pragma warning (disable:4619) // disable unknown warning (for older compiler)
#pragma warning (disable:4820) // padding added
#pragma warning (disable:5045) // compiler will/did insert Spectre mitigation
#endif

#include <math.h>
#include <assert.h>
#include <errno.h>
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

typedef ptrdiff_t ssize_t;

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

#if _MSC_VER
#pragma warning(push)
#pragma warning (disable:4668) // #if not_defined is #if 0
#endif

#include <limits.h>

#if _MSC_VER
#pragma warning(pop)
#endif

// Win32 ZeroMemory
#define ZeroMem(p, n) memset((p), 0, (n))

#if _MSC_VER
// Older compiler.
typedef signed __int8     int8_t;
typedef signed __int16    int16_t;
typedef signed __int32    int32_t;
typedef signed __int64    int64_t;
typedef unsigned __int8  uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#define __func__ __FUNCTION__
#else
#include <stdint.h>
#endif

#define _DARWIN_USE_64_BIT_INODE 1
//#define __DARWIN_ONLY_64_BIT_INO_T 1
// TODO cmake
// TODO big endian and packed I/O
//#define _LARGEFILE_SOURCE
//#define _LARGEFILE64_SOURCE

#if _MSC_VER
#pragma warning (disable:4201) // nameless struct/union
#pragma warning (disable:4355) // this used in base member initializer list
#pragma warning (disable:4100) // unused parameter
#pragma warning (disable:4371) // layout change from previous compiler version
#pragma warning (disable:4505) // unused static function
#pragma warning (disable:4514) // unused function
#pragma warning (disable:4668) // #if not_defined is #if 0
#pragma warning (disable:4710) // function not inlined
#pragma warning (disable:4820) // padding
#pragma warning (disable:5032) // pragma warning push is balanced elsewhere
#pragma warning (push)
#pragma warning (disable:4571) // catch(...)
#pragma warning (disable:4626) // assignment implicitly deleted
#pragma warning (disable:4625) // copy constructor implicitly deleted
#pragma warning (disable:4668) // #if not_defined as #if 0
#pragma warning (disable:4774) // printf used without constant format
#pragma warning (disable:4820) // ucrt\malloc.h(45): warning C4820: '_heapinfo': '4' bytes padding added after data member '_heapinfo::_useflag'
#pragma warning (disable:5039) // exception handling and function pointers
#include <intrin.h>
#endif

#if __GNUC__ || __clang__
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#ifndef _ISOC99_SOURCE
#define _ISOC99_SOURCE
#endif

#if _MSC_VER
#include <malloc.h>
#define alloca _alloca
#endif
