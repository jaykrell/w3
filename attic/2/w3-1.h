// A WebAssembly codebase by Jay Krell
//
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

/* __DARWIN_UNIX03 defaults to 1 on older and newer headers,
 * but older headers still have context "ss" instead of "__ss"
 * and such, so we have to force 0.
 * That is -- the defaults vary, the behavior of the newer
 * default is not available in older headers, so we must
 * force the older behavior, so that we can write one compatible source.
 */
#if defined(__APPLE__) && !defined(__DARWIN_UNIX03)
#define __DARWIN_UNIX03 0
#endif

#ifdef __osf__
/* To get struct tm.tm_gmtoff, tm_zone. Would be good to autoconf this? */
#ifndef _OSF_SOURCE
#define _OSF_SOURCE
#endif
/* For socklen_t. Would be good to autoconf this.
 * This also gives us "uin-len".
 */
#ifndef _POSIX_PII_SOCKET
#define _POSIX_PII_SOCKET
#endif
/* More clearly get "uin-len". */
#ifndef _SOCKADDR_LEN
#define _SOCKADDR_LEN
#endif
/* Request 64bit time_t. Not available on v4. Would be good to autoconf this.
 * We later check for TIMEVAL64TO32/TIMEVAL32TO64 to see if this works.
 */
#ifndef _TIME64_T
#define _TIME64_T
#endif
#endif /* osf */

/* Autoconf: AC_SYS_LARGEFILE
 */
#define _FILE_OFFSET_BITS 64

/* Autoconf: AC_USE_SYSTEM_EXTENSIONS */
/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
#define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
#define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
#define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
#define __EXTENSIONS__ 1
#endif

#ifndef _REENTRANT
#define _REENTRANT
#endif

/* AC_SYS_LARGEFILE */
#ifndef _LARGE_FILES
#define _LARGE_FILES 1
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

/*#ifdef __vms*/
/* Enable support for files larger than 2GB.
 * Autoconf: AC_SYS_LARGEFILE?
 */
#ifndef _LARGEFILE
#define _LARGEFILE
#endif
/* Enable 32bit gids and reveal setreuids. */
#ifndef __USE_LONG_GID_T
#define __USE_LONG_GID_T 1
#endif
/* st_ino has three forms that all fit in the
 * same space; pick the one we want.
 */
#ifndef __USE_INO64
#define __USE_INO64 1
#endif
/*#endif*/

#if defined(__arm__) && defined(__APPLE__)
/* Reveal the correct struct stat? */
#ifndef _DARWIN_FEATURE_64_ONLY_BIT_INODE
#define _DARWIN_FEATURE_64_ONLY_BIT_INODE
#endif
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
#include <stddef.h>

// Win32 ZeroMemory
#define ZeroMem(p, n) memset((p), 0, (n))

#include "w3StdInt.h"
#include "w3StdFunc.h"
#include "ieee.h"
#include "math_private.h"

#if UCHAR_MAX == 0x0FFUL
typedef   signed char        INT8; // TODO: C99
typedef unsigned char       UINT8;
#else
#error unable to find 8bit integer
#endif
#if USHRT_MAX == 0x0FFFFUL
typedef          short      INT16;
typedef unsigned short     UINT16;
#else
#error unable to find 16bit integer
#endif
#if UINT_MAX == 0x0FFFFFFFFUL
typedef          int        INT32;
typedef unsigned int       UINT32;
#elif ULONG_MAX == 0x0FFFFFFFFUL
typedef          long       INT32;
typedef unsigned long      UINT32;
#else
#error unable to find 32bit integer
#endif

// Support pre-C99 for Windows and VMS and any system with an __int64 macro.
#if !defined(_LONGLONG) && (defined(_MSC_VER) || defined(__DECC) || defined(__DECCXX) || defined(__int64))
typedef          __int64    INT64;
typedef unsigned __int64   UINT64;
#else
typedef          long long  INT64;
typedef unsigned long long UINT64;
#endif

typedef char* PCH;
typedef const char* PCSTR;
typedef ptrdiff_t ssize_t;

uint32_t GetUint16LE (const void* a);
uint32_t GetUint32LE (const void* a);

const uint32_t PageSize = (1UL << 16);
const uint32_t PageShift = 16;

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

#define FORMAT32 ""

#ifdef _WIN64
#define FORMAT_SIZE "I64"
#define long_t __int64 // aka ptrdiff, aka Unix long but not Windows long
#define FORMAT64 "I64"
#else
#define FORMAT64 "ll"
#define FORMAT_SIZE "l"
#define long_t long // aka ptrdiff, aka Unix long but not Windows long
#endif
