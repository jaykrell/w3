// 2-clause BSD license unless that does not suffice
// else MIT. Need to research licenses and maybe develop a business plan.
//
// Implementation language is C ?or? C++. At least C++11?
// The following features of C++ are desirable:
//   RAII (destructors, C++98)
//   enum class (C++11)
//   std::size (C++17)
//   std::string::data (direct sprintf into std::string) (C++17)
//   non-static member initialization (C++11) (no longer in use)
//   thread safe static initializers, maybe (C++11)
//   char16_t (C++11, but could use C++98 unsigned short) (more of a concern for .NET, not WebAssembly, and minor there)
//   explicit operator bool (C++11 but easy to emulate in C++98) (emulated)
//   variadic template (C++11, really needed?)
//   variadic macros (really needed?)
//   std::vector (write our own?)
//   std::string (write our own?)
//   std::stack (write our own?)
//   Probably more of STL.(write our own?)
//
// C++ library dependencies are likely to be removed, but we'll see.

// Goals: clarity, simplicity, portability, size, interpreter, compile to C++, and maybe
// later some JIT

// Fix for circa Visual C++ 2.0 Win32 SDK. // C:\msdev\MSVC20\INCLUDE\objbase.h(8934) : error C2065: '_fmemcmp' : undeclared identifier
#if defined (_WIN32) && !defined (WIN32)
#define WIN32 1
#endif

#if _MSC_VER && _MSC_VER <= 1500
//#error This version of Visual C++ is too old. Known bad versions include 5.0 and 2008. Known good includes 1900/2017.
#endif

#define _CRT_SECURE_NO_WARNINGS 1

//#include "config.h"
#define _DARWIN_USE_64_BIT_INODE 1
//#define __DARWIN_ONLY_64_BIT_INO_T 1
// TODO cmake
// TODO big endian and packed I/O
//#define _LARGEFILE_SOURCE
//#define _LARGEFILE64_SOURCE

#ifndef HAS_TYPED_ENUM
#if 1 // __cplusplus >= 201103L || _MSC_VER >= 1500 // TODO test more compilers
#define HAS_TYPED_ENUM 1
#else
#define HAS_TYPED_ENUM 0
#endif
#endif

#if _MSC_VER

#if _MSC_VER <= 1500 // TODO which version?
float truncf (float);
double trunc (double);
float roundf (float);
double round (double);
#endif

#if _MSC_VER <= 1100
#pragma warning (disable:4018) // unsigned/signed
#pragma warning (disable:4146) // negated unsigned is unsigned
#pragma warning (disable:4238) // <utility> nonstandard extension used : class rvalue used as lvalue
#pragma warning (disable:4244) // int to char conversion
#pragma warning (disable:4511) // copy construct could not be generated
#pragma warning (disable:4512) // assignment operator could not be generated
#pragma warning (disable:4663) // C++ language change: to explicitly specialize class template..
#endif
#pragma warning (disable:4201) // nameless struct/union
#pragma warning (disable:4355) // this used in base member initializer list
#if _MSC_VER <= 1500
#pragma warning (disable:4127) // while (true) constant conditional
#pragma warning (disable:4201) // nonstandard extension used: nameless struct/union
#pragma warning (disable:4296) // always false
#pragma warning (disable:4480) // enum base type was non-standard
#pragma warning (disable:4616) // unknown warning disabled
//#pragma warning (disable:4706) // assignment within conditional
#endif
#pragma warning (disable:4100) // unused parameter
#pragma warning (disable:4371) // layout change from previous compiler version
#pragma warning (disable:4505) // unused static function
#pragma warning (disable:4514) // unused function
#pragma warning (disable:4619) // invalid pragma warning disable
#pragma warning (disable:4668) // #if not_defined is #if 0
#pragma warning (disable:4710) // function not inlined
#pragma warning (disable:4820) // padding
#if _MSC_VER > 1100 //TODO which version?
#pragma warning (push)
#endif
#pragma warning (disable:4571) // catch(...)
#pragma warning (disable:4626) // assignment implicitly deleted
#pragma warning (disable:4625) // copy constructor implicitly deleted
#pragma warning (disable:4668) // #if not_defined as #if 0
#pragma warning (disable:4774) // printf used without constant format
#pragma warning (disable:4820) // ucrt\malloc.h(45): warning C4820: '_heapinfo': '4' bytes padding added after data member '_heapinfo::_useflag'
#pragma warning (disable:5026) // move constructor implicitly deleted
#pragma warning (disable:5027) // move assignment implicitly deleted
#pragma warning (disable:5039) // exception handling and function pointers
#pragma warning (disable:5045) // compiler will insert Spectre mitigation
#endif
#if __GNUC__ || __clang__
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#if _MSC_VER
#include <intrin.h>
#endif
#define _ISOC99_SOURCE
#include <math.h>
#include <stack>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <memory.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#if _WIN32
#define NOMINMAX 1
#include <io.h>
#include <windows.h>
__declspec(dllimport) int __stdcall IsDebuggerPresent(void);
#if _MSC_VER <= 1100 // TODO which version?
#define __debugbreak DebugBreak
#endif
#else
#define IsDebuggerPresent() (0)
#define __debugbreak() ((void)0)
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif
#if _MSC_VER
#include <malloc.h> // for _alloca
#if _MSC_VER > 1100 //TODO which version?
#pragma warning (pop)
#endif
#endif

#if _MSC_VER && _MSC_VER <= 1500
// TODO find out what other pre-C99 platforms have these?
typedef signed __int8 int8;
typedef signed __int16 int16;
typedef __int64 int64;
typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int64 uint64;
typedef unsigned __int32 uint;

#else

#if UCHAR_MAX == 0x0FFUL
typedef   signed char        int8;
typedef unsigned char       uint8;
#else
typedef  int8_t  int8;
typedef uint8_t uint8;
//#error unable to find 8bit integer
#endif
#if USHRT_MAX == 0x0FFFFUL
typedef          short      int16;
typedef unsigned short     uint16;
#else
typedef  int16_t  int16;
typedef uint16_t uint16;
//#error unable to find 16bit integer
#endif

#if UINT_MAX != 0x0FFFFFFFFUL
#error Change int to int32 where needed (or everywhere to be safe)
#endif

#if UINT_MAX == 0x0FFFFFFFFUL
typedef          int        int32;
typedef unsigned int       uint32;
typedef unsigned int       uint;
#elif ULONG_MAX == 0x0FFFFFFFFUL
typedef          long       int32;
typedef unsigned long      uint32;
#else
typedef  int32_t  int32; // TODO we just use int
typedef uint32_t uint;
#error unable to find 32bit integer
#endif
#if _MSC_VER || __DECC || __DECCXX || defined (__int64)
typedef          __int64    int64;
typedef unsigned __int64   uint64;
#else
typedef          long long  int64;
typedef unsigned long long uint64;
#endif
// todo
// C99 / C++?
//typedef int64_t int64;
//typedef uint64_t uint64;

#endif

namespace w3 // TODO Visual C++ 2.0 lacks namespaces
{

#if 0
static
void
AssertFailed (const char* file, int line, const char* expr)
{
    fprintf (stderr, "Assert failed: %s(%d):%s\n", file, line, expr);
#if _WIN32
    DebugBreak();
#else
    abort ();
#endif
}

#define Assert(expr) ((void)((expr) || AssertFailed (__FILE__, __LINE__, #expr)))
#endif

template <typename T>
T Min(const T& a, const T& b)
{
    return (a <= b) ? a : b;
}

template <typename T>
T Max(const T& a, const T& b)
{
    return (a >= b) ? a : b;
}

#if _WIN64
#define FORMAT_SIZE "I64"
#else
#define FORMAT_SIZE "l"
#endif
#if _MSC_VER
#pragma warning (disable:4777) // printf maybe wrong for other platforms
#endif

#if _MSC_VER
#pragma warning (push)
#pragma warning (disable:4996) // _vsnprintf dangerous
#endif

// Portable to old (and new) Visual C++ runtime.
uint
string_vformat_length (const char* format, va_list va)
{
#if _MSC_VER
    // newer runtime: _vscprintf (format, va);
    // else loop until it fits, getting -1 while it does not.
    uint n = 0;
    for (;;)
    {
        uint inc = n ? n : 64;
        if (_vsnprintf ((char*)_alloca (inc), n += inc, format, va) != -1)
            return n + 2;
    }
#else
    return 2 + vsnprintf (0, 0, format, va);
#endif
}

std::string
StringFormatVa (const char* format, va_list va)
{
    // Some systems, including Linux/amd64, cannot consume a
    // va_list multiple times. It must be copied first.
    // Passing the parameter twice does not work.
#if !_WIN32
    va_list va2;
#ifdef __va_copy
    __va_copy (va2, va);
#else
    va_copy (va2, va); // C99
#endif
#endif

    std::vector<char> s ((size_t)string_vformat_length (format, va));

#if _WIN32
    _vsnprintf (&s [0], s.size (), format, va);
#else
    vsnprintf (&s [0], s.size (), format, va2);
#endif
    return &s [0];
}

#if _MSC_VER
#pragma warning (pop)
#endif

std::string
StringFormat (const char* format, ...)
{
    va_list va;
    va_start (va, format);
    std::string a = StringFormatVa (format, va);
    va_end (va);
    return a;
}

#define NotImplementedYed() (AssertFormat (0, ("not yet implemented %s 0x%08X ", __func__, __LINE__)))

void
ThrowString (const std::string& a)
{
    //fprintf (stderr, "%s\n", a.c_str ());
    throw a + "\n";
    //abort ();
}

void
ThrowInt (int i, const char* a = "")
{
    ThrowString (StringFormat ("error 0x%08X %s", i, a));
}

void
ThrowErrno (const char* a = "")
{
    ThrowInt (errno, a);
}

#if _WIN32
void
throw_Win32Error (int err, const char* a = "")
{
    ThrowInt (err, a);

}
void
throw_GetLastError (const char* a = "")
{
    ThrowInt ((int)GetLastError (), a);

}
#endif

void
AssertFailedFormat (const char* condition, const std::string& extra)
{
    fputs (("AssertFailed:" + std::string (condition) + ":" + extra + "\n").c_str (), stderr);
    //Assert (0);
    //abort ();
#if _WIN32 // TODO
    if (IsDebuggerPresent ()) __debugbreak ();
#endif
    ThrowString ("AssertFailed:" + std::string (condition) + ":" + extra);
}

void
AssertFailed (const char* expr)
{
    fprintf (stderr, "AssertFailed:%s\n", expr);
#if _WIN32 // TODO
    if (IsDebuggerPresent ()) __debugbreak ();
#endif
    assert (0);
    abort ();
}

#define Assert(x)         ((x) || ( AssertFailed (#x), (int)0))
#define AssertFormat(x, extra) ((x) || (AssertFailedFormat (#x, StringFormat extra), 0))

static
uint
Unpack2 (const void* a)
{
    uint8* b = (uint8*)a;
    return ((b [1]) << 8) | (uint)b [0];
}

static
uint
Unpack4 (const void* a)
{
    return (Unpack2 ((char*)a + 2) << 16) | Unpack2 (a);
}

static
uint
Unpack (const void* a, uint size)
{
    switch (size)
    {
    case 2: return Unpack2 (a);
    case 4: return Unpack4 (a);
    }
    AssertFormat (size == 2 || size == 4, ("%X", size));
    return ~0u;
}

template <uint N> struct uintLEn_to_native_exact;
template <uint N> struct uintLEn_to_native_fast;

#if !_MSC_VER || _MSC_VER > 1000
template <> struct uintLEn_to_native_exact<16> { typedef uint16 T; };
template <> struct uintLEn_to_native_exact<32> { typedef uint T; };
template <> struct uintLEn_to_native_exact<64> { typedef uint64 T; };
template <> struct uintLEn_to_native_fast<16> { typedef uint T; };
template <> struct uintLEn_to_native_fast<32> { typedef uint T; };
template <> struct uintLEn_to_native_fast<64> { typedef uint64 T; };
#else
struct uintLEn_to_native_exact<16> { typedef uint16 T; };
struct uintLEn_to_native_exact<32> { typedef uint T; };
struct uintLEn_to_native_exact<64> { typedef uint64 T; };
struct uintLEn_to_native_fast<16> { typedef uint T; };
struct uintLEn_to_native_fast<32> { typedef uint T; };
struct uintLEn_to_native_fast<64> { typedef uint64 T; };
#endif

template <uint N>
struct uintLEn // unsigned little endian integer, size n bits
{
    union {
        typename uintLEn_to_native_exact<N>::T debug_n;
        unsigned char data [N / 8];
    };

    operator
#if !_MSC_VER || _MSC_VER > 1000
    typename
#endif
    uintLEn_to_native_fast<N>::T ()
    {
#if !_MSC_VER || _MSC_VER > 1000
        typename
#endif
        uintLEn_to_native_fast<N>::T a = 0;
        for (uint i = N / 8; i; )
            a = (a << 8) | data [--i];
        return a;
    }
    void operator= (uint);
};

typedef uintLEn<16> uintLE16;
typedef uintLEn<32> uintLE;
typedef uintLEn<64> uintLE64;

uint
Unpack (uintLE16& a)
{
    return (uint)a;
}

uint
Unpack (uintLE16* a)
{
    return (uint)*a;
}

uint
Unpack (uintLE& a)
{
    return (uint)a;
}

uint
Unpack (uintLE* a)
{
    return (uint)*a;
}

// C++98 workaround for what C++11 offers.
struct explicit_operator_bool
{
    typedef void (explicit_operator_bool::*T) () const;
    void True () const;
};

typedef void (explicit_operator_bool::*bool_type) () const;

#if _WIN32
struct Handle
{
    // TODO Handle vs. win32file_t, etc.

    uint64 get_file_size (const char* file_name = "")
    {
        DWORD hi = 0;
        DWORD lo = GetFileSize (h, &hi);
        if (lo == INVALID_FILE_SIZE)
        {
            DWORD err = GetLastError ();
            if (err != NO_ERROR)
                throw_Win32Error ((int)err, StringFormat ("GetFileSize (%s)", file_name).c_str ());
        }
        return (((uint64)hi) << 32) | lo;
    }

    void* h;

    Handle (void* a) : h (a) { }
    Handle () : h (0) { }

    void* get () { return h; }

    bool valid () const { return static_valid (h); }

    static bool static_valid (void* h) { return h && h != INVALID_HANDLE_VALUE; }

    operator void* () { return get (); }

    static void static_cleanup (void* h)
    {
        if (!static_valid (h)) return;
        CloseHandle (h);
    }

    void* detach ()
    {
        void* const a = h;
        h = 0;
        return a;
    }

    void cleanup ()
    {
        static_cleanup (detach ());
    }

    Handle& operator= (void* a)
    {
        if (h == a) return *this;
        cleanup ();
        h = a;
        return *this;
    }

#if 0 // C++11
    explicit operator bool () { return valid (); } // C++11
#else
    operator explicit_operator_bool::T () const
    {
        return valid () ? &explicit_operator_bool::True : NULL;
    }
#endif

    bool operator ! () { return !valid (); }

    ~Handle ()
    {
        if (valid ()) CloseHandle (h);
        h = 0;
    }
};
#endif

struct Fd
{
    int fd;

#ifndef _WIN32
    uint64 get_file_size (const char* file_name = "")
    {
#if __CYGWIN__
        struct stat st = { 0 }; // TODO test more systems
        if (fstat (fd, &st))
#else
        struct stat64 st = { 0 }; // TODO test more systems
        if (fstat64 (fd, &st))
#endif
            ThrowErrno (StringFormat ("fstat (%s)", file_name).c_str ());
        return st.st_size;
    }
#endif

#if 0 // C++11
    explicit operator bool () { return valid (); } // C++11
#else
    operator explicit_operator_bool::T () const
    {
        return valid () ? &explicit_operator_bool::True : NULL;
    }
#endif

    bool operator ! () { return !valid (); }

    operator int () { return get (); }
    static bool static_valid (int fd) { return fd != -1; }
    int get () const { return fd; }
    bool valid () const { return static_valid (fd); }

    static void static_cleanup (int fd)
    {
        if (!static_valid (fd)) return;
#if _WIN32
        _close (fd);
#else
        close (fd);
#endif
    }

    int detach ()
    {
        int const a = fd;
        fd = -1;
        return a;
    }

    void cleanup ()
    {
        static_cleanup (detach ());
    }

    Fd (int a = -1) : fd (a) { }

    Fd& operator= (int a)
    {
        if (fd == a) return *this;
        cleanup ();
        fd = a;
        return *this;
    }

    ~Fd ()
    {
        cleanup ();
    }
};

struct MemoryMappedFile
{
// TODO allow for redirection to built-in data (i.e. filesystem emulation with builtin BCL)
// TODO allow for systems that must read, not mmap
    void* base;
    size_t size;
#if _WIN32
    Handle file;
#else
    Fd file;
#endif
    MemoryMappedFile () : base (0), size (0) { }

    ~MemoryMappedFile ()
    {
        if (!base)
            return;
#if _WIN32
        UnmapViewOfFile (base);
#else
        munmap (base, size);
#endif
        base = 0;
    }
    void read (const char* a)
    {
#if _WIN32
        file = CreateFileA (a, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (!file) throw_GetLastError (StringFormat ("CreateFileA (%s)", a).c_str ());
        // FIXME check for size==0 and >4GB.
        size = (size_t)file.get_file_size (a);
        Handle h2 = CreateFileMappingW (file, 0, PAGE_READONLY, 0, 0, 0);
        if (!h2) throw_GetLastError (StringFormat ("CreateFileMapping (%s)", a).c_str ());
        base = MapViewOfFile (h2, FILE_MAP_READ, 0, 0, 0);
        if (!base)
            throw_GetLastError (StringFormat ("MapViewOfFile (%s)", a).c_str ());
#else
        file = open (a, O_RDONLY);
        if (!file) ThrowErrno (StringFormat ("open (%s)", a).c_str ());
        // FIXME check for size==0 and >4GB.
        size = (size_t)file.get_file_size (a);
        base = mmap (0, size, PROT_READ, MAP_PRIVATE, file, 0);
        if (base == MAP_FAILED)
            ThrowErrno (StringFormat ("mmap (%s)", a).c_str ());
#endif
    }
};

#if HAS_TYPED_ENUM
#define BEGIN_ENUM(name, type) enum name : type
#define END_ENUM(name, type) ;
#else
#define BEGIN_ENUM(name, type) enum _ ## name
#define END_ENUM(name, type) ; typedef type name;
#endif

#define NOTHING /* nothing */

static
uint64
SignExtend (uint64 value, uint bits)
{
    // Extract lower bits from value and signextend.
    // From detour_sign_extend.
    const uint left = 64 - bits;
    const uint64 m1 = (uint64)(int64)-1;
    const int64 wide = (int64)(value << left);
    const uint64 sign = (wide < 0) ? (m1 << left) : 0;
    return value | sign;
}

static
uint
int_magnitude (int i)
{
    // Avoid negating the most negative number.
    return 1 + (uint)-(i + 1);
}

struct int_split_sign_magnitude_t
{
    int_split_sign_magnitude_t (int64 a)
    : neg ((a < 0) ? 1u : 0u),
        u ((a < 0) ? (1 + (uint64)-(a + 1)) // Avoid negating most negative number.
                  : (uint64)a) { }
    uint neg;
    uint64 u;
};

static
uint
UIntGetPrecision (uint64 a)
{
    // How many bits needed to represent.
    uint len = 1;
    while ((len <= 64) && (a >>= 1)) ++len;
    return len;
}

static
uint
IntGetPrecision (int64 a)
{
    // How many bits needed to represent.
    // i.e. so leading bit is extendible sign bit, or 64
    return Min (64u, 1 + UIntGetPrecision (int_split_sign_magnitude_t (a).u));
}

static
uint
UIntToDec_GetLength (uint64 b)
{
    uint len = 0;
    do ++len;
    while (b /= 10);
    return len;
}

static
uint
UIntToDec (uint64 a, char* buf)
{
    uint const len = UIntToDec_GetLength (a);
    for (uint i = 0; i < len; ++i, a /= 10)
        buf [i] = "0123456789" [a % 10];
    return len;
}

static
uint
IntToDec (int64 a, char* buf)
{
    const int_split_sign_magnitude_t split (a);
    if (split.neg)
        *buf++ = '-';
    return split.neg + UIntToDec (split.u, buf);
}

static
uint
IntToDec_GetLength (int64 a)
{
    const int_split_sign_magnitude_t split (a);
    return split.neg + UIntToDec_GetLength (split.u);
}

static
uint
UIntToHex_GetLength (uint64 b)
{
    uint len = 0;
    do ++len;
    while (b >>= 4);
    return len;
}

static
uint
IntToHex_GetLength (int64 a)
{
    // If negative and first digit is <8, add one to induce leading 8-F
    // so that sign extension of most significant bit will work.
    // This might be a bad idea. TODO.
    uint64 b = (uint64)a;
    uint len = 0;
    uint64 most_significant;
    do ++len;
    while ((most_significant = b), b >>= 4);
    return len + (a < 0 && most_significant < 8);
}

static
void
UIntToHexLength (uint64 a, uint len, char* buf)
{
    buf += len;
    for (uint i = 0; i < len; ++i, a >>= 4)
        *--buf = "0123456789ABCDEF" [a & 0xF];
}

static
void
IntToHexLength (int64 a, uint len, char* buf)
{
    UIntToHexLength ((uint64)a, len, buf);
}

static
uint
IntToHex (int64 a, char* buf)
{
    uint const len = IntToHex_GetLength (a);
    IntToHexLength (a, len, buf);
    return len;
}

static
uint
IntToHex8 (int64 a, char* buf)
{
    IntToHexLength (a, 8, buf);
    return 8;
}

static
uint
IntToHex_GetLength_AtLeast8 (int64 a)
{
    uint const len = IntToHex_GetLength (a);
    return Max (len, 8u);
}

static
uint
UIntToHex_GetLength_AtLeast8 (uint64 a)
{
    uint const len = UIntToHex_GetLength (a);
    return Max (len, 8u);
}

static
uint
IntToHex_AtLeast8 (int64 a, char* buf)
{
    uint const len = IntToHex_GetLength_AtLeast8 (a);
    IntToHexLength (a, len, buf);
    return len;
}

static
uint
UIntToHex_AtLeast8 (uint64 a, char* buf)
{
    uint const len = UIntToHex_GetLength_AtLeast8 (a);
    UIntToHexLength (a, len, buf);
    return len;
}

struct stream
{
    virtual void write (const void* bytes, size_t size) = 0;
    void prints (const char* a) { write (a, strlen (a)); }
    void prints (const std::string& a) { prints (a.c_str ()); }
    void printc (char a) { write (&a, 1); }
    void printf (const char* format, ...)
    {
        va_list va;
        va_start (va, format);
        printv (format, va);
        va_end (va);
    }

    void
    printv (const char* format, va_list va)
    {
        prints (StringFormatVa (format, va));
    }
};

struct stdout_stream : stream
{
    virtual void write (const void* bytes, size_t size)
    {
        fflush (stdout);
        const char* pc = (const char*)bytes;
        while (size > 0)
        {
            uint const n = (uint)Min (size, ((size_t)1024) * 1024 * 1024);
#if _MSC_VER
            ::_write (_fileno (stdout), pc, n);
#else
            ::write (fileno (stdout), pc, n);
#endif
            size -= n;
            pc += n;
        }
    }
};

struct stderr_stream : stream
{
    virtual void write (const void* bytes, size_t size)
    {
        fflush (stderr);
        const char* pc = (const char*)bytes;
        while (size > 0)
        {
            uint const n = (uint)Min (size, ((size_t)1024) * 1024 * 1024);
#if _MSC_VER
            ::_write (_fileno (stderr), pc, n);
#else
            ::write (fileno (stderr), pc, n);
#endif
            size -= n;
            pc += n;
        }
    }
};

static
uint
read_byte (uint8*& cursor, const uint8* end)
{
    if (cursor >= end)
        ThrowString (StringFormat ("malformed %d", __LINE__)); // UNDONE context (move to module or section)
    return *cursor++;
}

static
uint64
read_varuint64 (uint8*& cursor, const uint8* end)
{
    uint64 result = 0;
    uint shift = 0;
    while (true)
    {
        const uint byte = read_byte (cursor, end);
        result |= (byte & 0x7F) << shift;
        if ((byte & 0x80) == 0)
            break;
        shift += 7;
    }
    return result;
}

static
uint
read_varuint32 (uint8*& cursor, const uint8* end)
{
    uint result = 0;
    uint shift = 0;
    while (true)
    {
        const uint byte = read_byte (cursor, end);
        result |= (byte & 0x7F) << shift;
        if ((byte & 0x80) == 0)
            break;
        shift += 7;
    }
    return result;
}

static
uint
read_varuint7 (uint8*& cursor, const uint8* end)
{
    const uint result = read_byte (cursor, end);
    if (result & 0x80)
        ThrowString (StringFormat ("malformed %d", __LINE__)); // UNDONE context (move to module or section)
    return result;
}

static
int64
read_varint64 (uint8*& cursor, const uint8* end)
{
    int64 result = 0;
    uint shift = 0;
    uint size = 64;
    uint byte = 0;
    do
    {
        byte = read_byte (cursor, end);
        result |= (byte & 0x7F) << shift;
        shift += 7;
    } while ((byte & 0x80) == 0);

    // sign bit of byte is second high order bit (0x40)
    if ((shift < size) && (byte & 0x40))
        result |= (~0 << shift); // sign extend

    return result;
}

static
int
read_varint32 (uint8*& cursor, const uint8* end)
{
    int result = 0;
    uint shift = 0;
    uint size = 32;
    uint byte = 0;
    do
    {
        byte = read_byte (cursor, end);
        result |= (byte & 0x7F) << shift;
        shift += 7;
    } while ((byte & 0x80) == 0);

    // sign bit of byte is second high order bit (0x40)
    if ((shift < size) && (byte & 0x40))
        result |= (~0 << shift); // sign extend

    return result;
}

typedef enum Type : uint8
{
    Type_none,
    Type_bool, // i32
    Type_any, // often has some constraints
    Type_i32 = 0x7F,
    Type_i64 = 0x7E,
    Type_f32 = 0x7D,
    Type_f64 = 0x7C,
} Type;

// This should probabably be combined with ResultType, and called Tag.
typedef enum ValueType : uint8
{
    ValueType_i32 = 0x7F,
    ValueType_i64 = 0x7E,
    ValueType_f32 = 0x7D,
    ValueType_f64 = 0x7C,
} ValueType;

typedef union Value
{
    int i32;
    uint u32;
    uint64 u64;
    int64 i64;
    float f32;
    double f64;
} Value;

typedef struct TaggedValue
{
    ValueType tag;
    Value value;
} TaggedValue;

// This should probabably be combined with ValueType, and called Tag.
typedef enum ResultType : uint8
{
    ResultType_i32 = 0x7F,
    ResultType_i64 = 0x7E,
    ResultType_f32 = 0x7D,
    ResultType_f64 = 0x7C,
    ResultType_empty = 0x40
} ResultType, BlockType;

typedef enum TableElementType : uint
{
    TableElementType_funcRef = 0x70,
} TableElementType;

typedef enum LimitsTag // specific to tabletype?
{
    LimitsTag_min = 0,
    Limits_minMax = 1,
} LimitsTag;

struct Limits
{
    uint min;
    uint max;
    bool hasMax;
};

const uint FunctionTypeTag = 0x60;

struct TableType
{
    TableElementType elementType;
    Limits limits;
};

// Table types have an value type, funcref
const uint TableTypeFuncRef = 0x70;

// Globals are mutable or constant.
typedef enum Mutable
{
    Mutable_constant = 0, // aka false
    Mutable_variable = 1, // aka true
} Mutable;

struct Runtime;
struct Stack;
struct StackValue;
struct ModuleInstance;
struct Module;
struct Section;

// The stack shall use _alloca in a non-recursive interpreter loop.
// This requires some care and macros. Macros that reference locals.
// The interpreter is likely to mostly dispatch to function pointers.
// Some pieces must not be in those functions. They can return values
// to the calling loop and the loop can do some of the work.
//
// In time, the function pointers might be case labels instead.
// However decomposition into separate functions is more elegant, if not less efficient.
//
// Such decomposition will also be good for conversion to JIT, LLVM, C++, etc.
// It is only interpreter, perhaps, that has overwhelming efficiency concern.
//
// StackValue initial_stack[1];
// int stack_depth;
// StackValue* stack = initial_stack;
// StackValue* min_stack = initial_stack;

#if 0 // probably will not do it this way, std::stack and loop instead
// FIXME for grow up stack
#define ALLOC_STACK(n)                                                                  \
do {                                                                                    \
    if (stack - n < min_stack)                                                          \
        min_stack = (StackValue*)alloca((min_stack - (stack - n)) * sizeof (*stack)); \
    stack -= n;                                                                         \
} while (0)

// probably will not do it this way, std::stack and loop instead
#define STACK_POP_CHECK(n) \
do {                                                                                    \
    Assert (n <= stack_depth);  \
} while (0)

#define STACK_POP_UNSAFE(n) \
do {                                                                                    \
    stack_depth -= n;           \
    stack += n;                 \
} while (0)

// probably will not do it this way, std::stack and loop instead
// FIXME for grow up stack
#define STACK_POP(n) \
do {                                                                                    \
    Assert (n <= stack_depth);  \
    stack_depth -= n;           \
    stack += n;                 \
} while (0)

// probably will not do it this way, std::stack and loop instead
#define STACK_PUSH(v)           \
do {                            \                                                       \
    ALLOC_STACK (1);            \
    stack [0] = (v);            \
} while (0)                     \

// probably will not do it this way, std::stack and loop instead
#define FRAME_PUSH(callee)                      \
do {                                            \
    ALLOC_STACK (function->locals_size + 1);    \
    stack [0].frame = frame;                    \
} while (0)                                     \

// probably will not do it this way, std::stack and loop instead
#define FRAME_POP()                         \
do {                                        \
    STACK_POP (function->locals_size);      \
    frame = stack [0].frame;                \
    STACK_POP (1);                          \
} while (0)                                 \

#endif

typedef enum StackTag
{
    StackTag_Value = 1, // i32, i64, f32, f64
    StackTag_Label,     // branch target
    StackTag_Frame,     // return address + locals + params
} StackTag;

typedef struct LabelValue
{
    // FUTURE spec arity
    uint value; // presumably an index into decoded_instructions within implied Code.
} LabelValue;


typedef struct FunctionType FunctionType;
typedef struct Function Function;
typedef struct Code Code;
typedef struct Frame Frame; // work in progress

// work in progress
typedef struct StackValue
{
    StackTag type : 8;
    union
    {
        TaggedValue value;
        LabelValue label;
        Frame* frame; // TODO by value?
    };
} StackValue;

// TODO consider a vector instead, but it affects frame.locals staying valid across push/pop
typedef std::deque <StackValue> StackBaseBase;

struct StackBase : private StackBaseBase
{
    typedef StackBaseBase base;
    using base::iterator;
    using base::end;

    void push (const StackValue& a)
    {
        push_back (a);
    }

    void pop ()
    {
        pop_back ();
    }

    StackValue& top ()
    {
        return back ();
    }
};

struct Frame
{
    // FUTURE spec return_arity
    size_t function_index; // replace with pointer?
    ModuleInstance* module_instance;
    Module* module;
    Frame* next;
    StackBase::iterator locals;
    Code* code;
    size_t local_count; // includes params
    // TODO locals/params
    // This should just be stack pointer, to another stack,
    // along with type information (module->module->locals_types[])
};

// work in progress
struct Stack : private StackBase
{
    Stack () : value_depth (0)
    {
    }

    typedef StackBase base;
    using base::top;
    using base::iterator;
    using base::end;

    void reserve (size_t n)
    {
        // TODO
    }

    int value_depth; // excluding labels and frames

    // TODO labels and frames on stack

    ValueType& tag (ValueType tag)
    {
        Assert (value_depth >= 1);
        Assert (top ().type == StackTag_Value);
        Assert (top ().value.tag == tag);
        return top ().value.tag;
    }

    ValueType& tag ()
    {
        Assert (value_depth >= 1);
        Assert (top ().type == StackTag_Value);
        return top ().value.tag;
    }

    Value& value ()
    {
        Assert (value_depth >= 1);
        Assert (top ().type == StackTag_Value);
        return top ().value.value;
    }

    Value& value (ValueType tag)
    {
        Assert (value_depth >= 1);
        Assert (top ().type == StackTag_Value);
        Assert (top ().value.tag == tag);
        return top ().value.value;
    }

    void pop_value ()
    {
        Assert (value_depth >= 1);
        pop ();
        --value_depth;
    }

    void push_value (StackValue value)
    {
        push (value);
        value_depth += 1;
    }

    // type specific pushers

    void push_i32 (int i)
    {
        StackValue value = {StackTag_Value, { ValueType_i32 } };
        value.value.value.i32 = i;
        push_value (value);
    }

    void push_i64 (int64 i)
    {
        StackValue value = {StackTag_Value, { ValueType_i64 } };
        value.value.value.i64 = i;
        push_value (value);
    }

    void push_u32 (uint i)
    {
        push_i32 ((int)i);
    }

    void push_u64 (uint64 i)
    {
        push_i64 ((int64)i);
    }

    void push_f32 (float i)
    {
        StackValue value = {StackTag_Value, { ValueType_f32 } };
        value.value.value.f32 = i;
        push_value (value);
    }

    void push_f64 (double i)
    {
        StackValue value = {StackTag_Value, { ValueType_f64 } };
        value.value.value.f64 = i;
        push_value (value);
    }

    void push_bool (bool b)
    {
        push_i32 (b);
    }

    // accessors, check tag, return ref

    int& i32 ()
    {
        return value (ValueType_i32).i32;
    }

    int64& i64 ()
    {
        return value (ValueType_i64).i64;
    }

    uint& u32 ()
    {
        return value (ValueType_i32).u32;
    }

    uint64& u64 ()
    {
        return value (ValueType_i64).u64;
    }

    float& f32 ()
    {
        return value (ValueType_f32).f32;
    }

    double& f64 ()
    {
        return value (ValueType_f64).f64;
    }

    // setter, changes tag, returns ref

    Value& set (ValueType tag)
    {
        Assert (value_depth >= 1);
        StackValue& t = top ();
        TaggedValue& v = t.value;
        Assert (t.type == StackTag_Value);
        v.tag = tag;
        return v.value;
    }

    // type-specific setters

    void set_i32 (int a)
    {
        set (ValueType_i32).i32 = a;
    }

    void set_u32 (uint a)
    {
        set (ValueType_i32).u32 = a;
    }

    void set_bool (bool a)
    {
        set_i32 (a);
    }

    void set_i64 (int64 a)
    {
        set (ValueType_i64).i64 = a;
    }

    void set_u64 (uint64 a)
    {
        set (ValueType_i64).u64 = a;
    }

    void set_f32 (float a)
    {
        set (ValueType_f32).f32 = a;
    }

    void set_f64 (double a)
    {
        set (ValueType_f64).f64 = a;
    }

    // type specific poppers

    int pop_i32 ()
    {
        int a = i32 ();
        pop_value ();
        return a;
    }

    uint pop_u32 ()
    {
        uint a = u32 ();
        pop_value ();
        return a;
    }

    int64 pop_i64 ()
    {
        int64 a = i64 ();
        pop_value ();
        return a;
    }

    uint64 pop_u64 ()
    {
        uint64 a = u64 ();
        pop_value ();
        return a;
    }

    float pop_f32 ()
    {
        float a = f32 ();
        pop_value ();
        return a;
    }

    double pop_f64 ()
    {
        double a = f64 ();
        pop_value ();
        return a;
    }
};

typedef enum Immediate : uint8
{
    Imm_none = 0,
    Imm_i32,
    Imm_i64,
    Imm_f32,
    Imm_f64,
    Imm_sequence,
    Imm_vecLabel,
    //Imm_u32,
    Imm_memory      ,     // align:u32 offset:u32
    Imm_type        ,     // read_varuint32
    Imm_function    ,     // read_varuint32
    Imm_global      ,     // read_varuint32
    Imm_local       ,     // read_varuint32
    Imm_label       ,     // read_varuint32
} Immediate;

#define INTERP(x) void interp_ ## x ();

INTERP (Unreach)
INTERP (Nop)
INTERP (Block)
INTERP (Loop)
INTERP (If)
INTERP (Else)
INTERP (BlockEnd)
INTERP (Br)
INTERP (BrIf)
INTERP (BrTable)
INTERP (Ret)
INTERP (Call)
INTERP (Calli)
INTERP (Drop)
INTERP (Select)
INTERP (Local_get)
INTERP (Local_set)
INTERP (Local_tee)
INTERP (Global_get)
INTERP (Global_set)
INTERP (MemSize)
INTERP (MemGrow)
INTERP (Convert) // TODO templatize
INTERP (Reserved) // TODO templatize
INTERP (Load) // TODO templatize
INTERP (Store) // TODO templatize
INTERP (Const) // TODO templatize
INTERP (ITestOp)
INTERP (IRelOp)
INTERP (FRelOp)
INTERP (IUnOp)
INTERP (IBinOp)
INTERP (FUnOp)
INTERP (FBinOp)

#define INSTRUCTIONS \
INSTRUCTION (0x00, 1, 0, Unreach,   Imm_none,     0, 0, Type_none, Type_none, Type_none, Type_none) \
INSTRUCTION (0x01, 1, 0, Nop,       Imm_none,     0, 0, Type_none, Type_none, Type_none, Type_none) \
INSTRUCTION (0x02, 1, 0, Block,     Imm_sequence, 0, 0, Type_none, Type_none, Type_none, Type_none) \
INSTRUCTION (0x03, 1, 0, Loop,      Imm_sequence, 0, 0, Type_none, Type_none, Type_none, Type_none) \
INSTRUCTION (0x04, 1, 0, If,        Imm_sequence, 0, 0, Type_none, Type_none, Type_none, Type_none) \
INSTRUCTION (0x05, 1, 0, Else,      Imm_sequence, 0, 0, Type_none, Type_none, Type_none, Type_none) \
\
RESERVED (06) \
RESERVED (07) \
RESERVED (08) \
RESERVED (09) \
RESERVED (0A) \
\
INSTRUCTION (0x0B, 1, 0, BlockEnd,  Imm_none,       0, 0, Type_none, Type_none, Type_none, Type_none) \
INSTRUCTION (0x0C, 1, 0, Br,        Imm_label,      0, 0, Type_none, Type_none, Type_none, Type_none) \
INSTRUCTION (0x0D, 1, 0, BrIf,      Imm_label,      0, 0, Type_none, Type_none, Type_none, Type_none) \
INSTRUCTION (0x0E, 1, 0, BrTable,   Imm_vecLabel,   0, 0, Type_none, Type_none, Type_none, Type_none) \
INSTRUCTION (0x0F, 1, 0, Ret,       Imm_none,       0, 0, Type_none, Type_none, Type_none, Type_none) \
INSTRUCTION (0x10, 1, 0, Call,      Imm_function,   0, 0, Type_none, Type_none, Type_none, Type_none) \
INSTRUCTION (0x11, 1, 0, Calli,     Imm_type,       0, 0, Type_none, Type_none, Type_none, Type_none) \
\
RESERVED (12) \
RESERVED (13) \
RESERVED (14) \
RESERVED (15) \
RESERVED (16) \
RESERVED (17) \
RESERVED (18) \
RESERVED (19) \
\
INSTRUCTION (0x1A, 1, 0, Drop,       Imm_none, 1, 0, Type_any, Type_none, Type_none, Type_none) \
INSTRUCTION (0x1B, 1, 0, Select,     Imm_none, 3, 1, Type_any, Type_any, Type_bool, Type_any) \
\
RESERVED (1C) \
RESERVED (1D) \
RESERVED (1E) \
RESERVED (1F) \
\
INSTRUCTION (0x20, 1, 0, Local_get,  Imm_local,  0, 1, Type_none, Type_none, Type_none, Type_any) \
INSTRUCTION (0x21, 1, 0, Local_set,  Imm_local,  1, 0, Type_any,  Type_none, Type_none, Type_none) \
INSTRUCTION (0x22, 1, 0, Local_tee,  Imm_local,  1, 1, Type_any,  Type_none, Type_none, Type_any) \
INSTRUCTION (0x23, 1, 0, Global_get, Imm_global, 0, 1, Type_none, Type_none, Type_none, Type_any) \
INSTRUCTION (0x24, 1, 0, Global_set, Imm_global, 1, 0, Type_any,  Type_none, Type_none, Type_none) \
\
RESERVED (25) \
RESERVED (26) \
RESERVED (27) \
\
LOAD (0x28, i32, ) \
LOAD (0x29, i64, ) \
LOAD (0x2A, f32, ) \
LOAD (0x2B, f64, ) \
\
/* zero or sign extending load from memory to register */ \
LOAD (0x2C, i32, 8s) \
LOAD (0x2D, i32, 8u) \
LOAD (0x2E, i32, 16s) \
LOAD (0x2F, i32, 16u) \
LOAD (0x30, i64, 8s) \
LOAD (0x31, i64, 8u) \
LOAD (0x32, i64, 16s) \
LOAD (0x33, i64, 16u) \
LOAD (0x34, i64, 32s) \
LOAD (0x35, i64, 32u) \
\
STORE (0x36, i32, ) \
STORE (0x37, i64, ) \
STORE (0x38, f32, ) \
STORE (0x39, f64, ) \
\
/* truncating store from register to memory */ \
STORE (0x3A, i32, 8) \
STORE (0x3B, i32, 16) \
STORE (0x3C, i64, 8)  \
STORE (0x3D, i64, 16) \
STORE (0x3E, i64, 32) \
\
INSTRUCTION (0x3F, 2, 0, MemSize, Imm_none, 0, 0, Type_none, Type_none, Type_none, Type_none) \
INSTRUCTION (0x40, 2, 0, MemGrow, Imm_none, 0, 0, Type_none, Type_none, Type_none, Type_none) \
\
CONST (0x41, i32) \
CONST (0x42, i64) \
CONST (0x43, f32) \
CONST (0x44, f64) \
\
ITESTOP (0x45, Eqz, 32) \
IRELOP (0x46, Eq, 32, ) \
IRELOP (0x47, Ne, 32, ) \
IRELOP (0x48, Lt, 32, s) \
IRELOP (0x49, Lt, 32, u) \
IRELOP (0x4A, Gt, 32, s) \
IRELOP (0x4B, Gt, 32, u) \
IRELOP (0x4C, Le, 32, s) \
IRELOP (0x4D, Le, 32, u) \
IRELOP (0x4E, Ge, 32, s) \
IRELOP (0x4F, Ge, 32, u) \
\
ITESTOP (0x50, Eqz, 64) \
IRELOP (0x51, Eq, 64,  ) \
IRELOP (0x52, Ne, 64,  ) \
IRELOP (0x53, Lt, 64, s) \
IRELOP (0x54, Lt, 64, u) \
IRELOP (0x55, Gt, 64, s) \
IRELOP (0x56, Gt, 64, u) \
IRELOP (0x57, Le, 64, s) \
IRELOP (0x58, Le, 64, u) \
IRELOP (0x59, Ge, 64, s) \
IRELOP (0x5A, Ge, 64, u) \
\
FRELOP (0x5B, Eq, 32) \
FRELOP (0x5C, Ne, 32) \
FRELOP (0x5D, Lt, 32) \
FRELOP (0x5E, Gt, 32) \
FRELOP (0x5F, Le, 32) \
FRELOP (0x60, Ge, 32) \
\
FRELOP (0x61, Eq, 64) \
FRELOP (0x62, Ne, 64) \
FRELOP (0x63, Lt, 64) \
FRELOP (0x64, Gt, 64) \
FRELOP (0x65, Le, 64) \
FRELOP (0x66, Ge, 64) \
\
 IUNOP (0x67, Clz,     32) \
 IUNOP (0x68, Ctz,     32) \
 IUNOP (0x69, Popcnt,  32) \
IBINOP (0x6A, Add,     32) \
IBINOP (0x6B, Sub,     32) \
IBINOP (0x6C, Mul,     32) \
IBINOP (0x6D, Div_s,   32) \
IBINOP (0x6E, Div_u,   32) \
IBINOP (0x6F, Rem_s,   32) \
IBINOP (0x70, Rem_u,   32) \
IBINOP (0x71, And,     32) \
IBINOP (0x72, Or,      32) \
IBINOP (0x73, Xor,     32) \
IBINOP (0x74, Shl,     32) \
IBINOP (0x75, Shr_s,   32) \
IBINOP (0x76, Shr_u,   32) \
IBINOP (0x77, Rotl,    32) \
IBINOP (0x78, Rotr,    32) \
\
 IUNOP (0x79, Clz,     64) \
 IUNOP (0x7A, Ctz,     64) \
 IUNOP (0x7B, Popcnt,  64) \
IBINOP (0x7C, Add,     64) \
IBINOP (0x7D, Sub,     64) \
IBINOP (0x7E, Mul,     64) \
IBINOP (0x7F, Div_s,   64) \
IBINOP (0x80, Div_u,   64) \
IBINOP (0x81, Rem_s,   64) \
IBINOP (0x82, Rem_u,   64) \
IBINOP (0x83, And,     64) \
IBINOP (0x84, Or,      64) \
IBINOP (0x85, Xor,     64) \
IBINOP (0x86, Shl,     64) \
IBINOP (0x87, Shr_s,   64) \
IBINOP (0x88, Shr_u,   64) \
IBINOP (0x89, Rotl,    64) \
IBINOP (0x8A, Rotr,    64) \
\
 FUNOP (0x8B, Abs,      32) \
 FUNOP (0x8C, Neg,      32) \
 FUNOP (0x8D, Ceil,     32) \
 FUNOP (0x8E, Floor,    32) \
 FUNOP (0x8F, Trunc,    32) \
 FUNOP (0x90, Nearest,  32) \
 FUNOP (0x91, Sqrt,     32) \
FBINOP (0x92, Add,      32) \
FBINOP (0x93, Sub,      32) \
FBINOP (0x94, Mul,      32) \
FBINOP (0x95, Div,      32) \
FBINOP (0x96, Min,      32) \
FBINOP (0x97, Max,      32) \
FBINOP (0x98, Copysign, 32) \
\
 FUNOP (0x99, Abs,      64) \
 FUNOP (0x9A, Neg,      64) \
 FUNOP (0x9B, Ceil,     64) \
 FUNOP (0x9C, Floor,    64) \
 FUNOP (0x9D, Trunc,    64) \
 FUNOP (0x9E, Nearest,  64) \
 FUNOP (0x9F, Sqrt,     64) \
FBINOP (0xA0, Add,      64) \
FBINOP (0xA1, Sub,      64) \
FBINOP (0xA2, Mul,      64) \
FBINOP (0xA3, Div,      64) \
FBINOP (0xA4, Min,      64) \
FBINOP (0xA5, Max,      64) \
FBINOP (0xA6, Copysign, 64) \
\
CVTOP (0xA7,  Wrap, i32, i64,  )  \
CVTOP (0xA8, Trunc, i32, f32, s)  \
CVTOP (0xA9, Trunc, i32, f32, u)  \
CVTOP (0xAA, Trunc, i32, f64, s)  \
CVTOP (0xAB, Trunc, i32, f64, u)  \
CVTOP (0xAC, Extend, i64, i32, s) \
CVTOP (0xAD, Extend, i64, i32, u) \
CVTOP (0xAE, Trunc, i64, f32, s)  \
CVTOP (0xAF, Trunc, i64, f32, u)  \
CVTOP (0xB0, Trunc, i64, f64, s)  \
CVTOP (0xB1, Trunc, i64, f64, u)  \
\
CVTOP (0xB2, Convert, f32, i32, u) \
CVTOP (0xB3, Convert, f32, i32, s) \
CVTOP (0xB4, Convert, f32, i64, u) \
CVTOP (0xB5, Convert, f32, i64, s) \
CVTOP (0xB6, Demote,  f32, f64, ) \
CVTOP (0xB7, Convert, f64, i32, s) \
CVTOP (0xB8, Convert, f64, i32, u) \
CVTOP (0xB9, Convert, f64, i64, s) \
CVTOP (0xBA, Convert, f64, i64, u) \
CVTOP (0xBB, Promote, f64, f32, ) \
\
CVTOP (0xBC, Reinterpret, i32, f32, ) \
CVTOP (0xBD, Reinterpret, i64, f64, ) \
CVTOP (0xBE, Reinterpret, f32, i32, ) \
CVTOP (0xBF, Reinterpret, f64, i64, ) \
\
RESERVED (C0) \
RESERVED (C1) \
RESERVED (C2) \
RESERVED (C3) \
RESERVED (C4) \
RESERVED (C5) \
RESERVED (C6) \
RESERVED (C7) \
RESERVED (C8) \
RESERVED (C9) \
RESERVED (CA) \
RESERVED (CB) \
RESERVED (CC) \
RESERVED (CD) \
RESERVED (CE) \
RESERVED (CF) \
\
RESERVED (D0) \
RESERVED (D1) \
RESERVED (D2) \
RESERVED (D3) \
RESERVED (D4) \
RESERVED (D5) \
RESERVED (D6) \
RESERVED (D7) \
RESERVED (D8) \
RESERVED (D9) \
RESERVED (DA) \
RESERVED (DB) \
RESERVED (DC) \
RESERVED (DD) \
RESERVED (DE) \
RESERVED (DF) \
\
RESERVED (E0) \
RESERVED (E1) \
RESERVED (E2) \
RESERVED (E3) \
RESERVED (E4) \
RESERVED (E5) \
RESERVED (E6) \
RESERVED (E7) \
RESERVED (E8) \
RESERVED (E9) \
RESERVED (EA) \
RESERVED (EB) \
RESERVED (EC) \
RESERVED (ED) \
RESERVED (EE) \
RESERVED (EF) \
\
RESERVED (F0) \
RESERVED (F1) \
RESERVED (F2) \
RESERVED (F3) \
RESERVED (F4) \
RESERVED (F5) \
RESERVED (F6) \
RESERVED (F7) \
RESERVED (F8) \
RESERVED (F9) \
RESERVED (FA) \
RESERVED (FB) \
RESERVED (FC) \
RESERVED (FD) \
RESERVED (FE) \
RESERVED (FF) \

// integer | float, unary | binary | test | relation
#define IUNOP(b0, name, size)   INSTRUCTION (b0, 1, 0, name ## _i ## size, Imm_none, 1, 1, Type_i ## size, Type_none, Type_none, Type_i ## size)
#define FUNOP(b0, name, size)   INSTRUCTION (b0, 1, 0, name ## _f ## size, Imm_none, 1, 1, Type_f ## size, Type_none, Type_none, Type_f ## size)
#define IBINOP(b0, name, size)  INSTRUCTION (b0, 1, 0, name ## _i ## size, Imm_none, 2, 1, Type_i ## size, Type_i ## size, Type_none, Type_i ## size)
#define FBINOP(b0, name, size)  INSTRUCTION (b0, 1, 0, name ## _f ## size, Imm_none, 2, 1, Type_f ## size, Type_f ## size, Type_none, Type_f ## size)
#define ITESTOP(b0, name, size) INSTRUCTION (b0, 1, 0, name ## _i ## size, Imm_none, 1, 1, Type_i ## size, Type_none,     Type_none, Type_bool)
#define FRELOP(b0, name, size)  INSTRUCTION (b0, 1, 0, name ## _f ## size, Imm_none, 2, 1, Type_f ## size, Type_f ## size, Type_none, Type_bool)
#define IRELOP(b0, name, size, sign)  INSTRUCTION (b0, 1, 0, name ## _i ## size ## sign, Imm_none, 2, 1, Type_i ## size, Type_i ## size, Type_none, Type_bool)

// convert; TODO make ordering more sensible?
#define CVTOP(b0, name, to, from, sign) INSTRUCTION (b0, 1, 0, to ## _ ## name ## _ ## from ## sign, Imm_none, 1, 1, Type_ ## from, Type_none, Type_none, Type_ ## to)

#define RESERVED(b0) INSTRUCTION (0x ## b0, 0, 0, Reserved ## b0, Imm_none, 0, 0, Type_none, Type_none, Type_none, Type_none)

#undef CONST
#define CONST(b0, type) INSTRUCTION (b0, 1, 0, type ## _Const, Imm_ ## type, 0, 1, Type_none, Type_none, Type_none, Type_ ## type)

#define LOAD(b0, to, from) INSTRUCTION (b0, 1, 0, to ## _Load ## from, Imm_memory, 0, 1, Type_none, Type_none, Type_none, Type_ ## to)
#define STORE(b0, from, to) INSTRUCTION (b0, 1, 0, from ## _Store ## to, Imm_memory, 1, 0, Type_ ## from, Type_none, Type_none, Type_none)


#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) name,
enum InstructionEnum : uint16
{
    INSTRUCTIONS
};

#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) char name [ sizeof (#name) ];
typedef struct InstructionNames
{
INSTRUCTIONS
} InstructionNames;

const char instructionNames [ ] =
#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) #name "\0"
INSTRUCTIONS
;
#define BITS_FOR_UINT_HELPER(a, x) (a) >= (1u << x) ? (x) + 1 :
#define BITS_FOR_UINT(a)                                                                                \
  (BITS_FOR_UINT_HELPER (a, 31) BITS_FOR_UINT_HELPER (a, 30)                                                          \
   BITS_FOR_UINT_HELPER (a, 29) BITS_FOR_UINT_HELPER (a, 28) BITS_FOR_UINT_HELPER (a, 27) BITS_FOR_UINT_HELPER (a, 26) BITS_FOR_UINT_HELPER (a, 25) \
   BITS_FOR_UINT_HELPER (a, 24) BITS_FOR_UINT_HELPER (a, 23) BITS_FOR_UINT_HELPER (a, 22) BITS_FOR_UINT_HELPER (a, 21) BITS_FOR_UINT_HELPER (a, 20) \
   BITS_FOR_UINT_HELPER (a, 19) BITS_FOR_UINT_HELPER (a, 18) BITS_FOR_UINT_HELPER (a, 17) BITS_FOR_UINT_HELPER (a, 16) BITS_FOR_UINT_HELPER (a, 15) \
   BITS_FOR_UINT_HELPER (a, 14) BITS_FOR_UINT_HELPER (a, 13) BITS_FOR_UINT_HELPER (a, 12) BITS_FOR_UINT_HELPER (a, 11) BITS_FOR_UINT_HELPER (a, 10) \
   BITS_FOR_UINT_HELPER (a,  9) BITS_FOR_UINT_HELPER (a,  8) BITS_FOR_UINT_HELPER (a,  7) BITS_FOR_UINT_HELPER (a,  6) BITS_FOR_UINT_HELPER (a,  5) \
   BITS_FOR_UINT_HELPER (a,  4) BITS_FOR_UINT_HELPER (a,  3) BITS_FOR_UINT_HELPER (a,  2) BITS_FOR_UINT_HELPER (a,  1) BITS_FOR_UINT_HELPER (a,  0) 1)

#if _MSC_VER && _MSC_VER <= 1500

#define __func__ __FUNCTION__
#include <windows.h>
#define bits_for_uint(x) BITS_FOR_UINT (x)
#define static_assert(x, y) C_ASSERT (x)

#else

constexpr int bits_for_uint (uint a)
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

static_assert (BITS_FOR_UINT (0) == 1, "0");
static_assert (BITS_FOR_UINT (1) == 1, "1");
static_assert (BITS_FOR_UINT (2) == 2, "2");
static_assert (BITS_FOR_UINT (3) == 2, "3");
static_assert (BITS_FOR_UINT (4) == 3, "4");
static_assert (BITS_FOR_UINT (10) == 4, "");
static_assert (BITS_FOR_UINT (30) == 5, "");
static_assert (BITS_FOR_UINT (200) == 8, "");
static_assert (BITS_FOR_UINT (sizeof (instructionNames)) == 12, "");

static_assert (bits_for_uint (0) == 1, "0");
static_assert (bits_for_uint (1) == 1, "1");
static_assert (bits_for_uint (2) == 2, "2");
static_assert (bits_for_uint (3) == 2, "3");
static_assert (bits_for_uint (4) == 3, "4");
static_assert (bits_for_uint (10) == 4, "");
static_assert (bits_for_uint (30) == 5, "");
static_assert (bits_for_uint (200) == 8, "");
static_assert (bits_for_uint (sizeof (instructionNames)) == 12, "");

#define InstructionName(i) (&instructionNames [instructionEncode [i].string_offset])

struct InstructionEncoding
{
    uint8 byte0;
    //uint8 byte1;              // FIXME always 0 if fixed_size > 1
    uint8 fixed_size    : 2;    // 0, 1, 2
    Immediate immediate;
    uint8 pop           : 2;    // required minimum stack in
    uint8 push          : 1;
    InstructionEnum name : 16;
    uint string_offset : bits_for_uint (sizeof (instructionNames));
    Type stack_in0  ; // type of stack [0] upon input, if pop >= 1
    Type stack_in1  ; // type of stack [1] upon input, if pop >= 2
    Type stack_in2  ; // type of stack [2] upon input, if pop == 3
    Type stack_out0 ; // type of stack [1] upon input, if push == 1
    void (*interp) (Module*); // Module* wrong
};

struct DecodedInstruction
{
    DecodedInstruction ()
    {
        name = (InstructionEnum)-1;
        align = offset = (uint)-1;
    }

    union
    {

        uint  u32;
        uint64 u64;
        int   i32;
        int64 i64;
        float f32;
        double f64;
        struct // memory
        {
            uint align;
            uint offset;
        };
        // etc.
    };
    std::vector <DecodedInstruction> sequence;
    std::vector <uint> vecLabel;
    InstructionEnum name;
    BlockType blockType;
};

#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, pop, push, in0, in1, in2, out0) { byte0, fixed_size, imm, pop, push, name, offsetof (InstructionNames, name), in0, in1, in2, out0 },
const InstructionEncoding instructionEncode [ ] = {
    INSTRUCTIONS
};

static_assert (sizeof (instructionEncode) / sizeof (instructionEncode [0]) == 256, "not 256 instructions");

typedef enum BuiltinString {
    BuiltinString_none = 0,
    BuiltinString_main,
    BuiltinString_start,
} BuiltinString;

struct String
{
    String() :
        data (0),
        size (0),
        builtin (BuiltinString_none),
        builtinStorage (false)
    {
    }

    char* data;
    size_t size;
    std::string storage;
    BuiltinString builtin ;
    bool builtinStorage;

    char* c_str ()
    {
        if (!data)
        {
            data = (char*)storage.c_str ();
        }
        else if (data != storage.c_str ())
        {
            storage = std::string (data, size);
            data = (char*)storage.c_str ();
        }
        return data;
    }
};

struct Section
{
    uint id;
    String name;
    size_t payload_size;
    uint8* payload;
};

struct SectionTraits
{
    const char* name;
    void (Module::*read)(uint8*& cursor);
};

typedef enum ImportTag { // aka desc
    ImportTag_Function = 0, // aka type
    ImportTag_Table = 1,
    ImportTag_Memory = 2,
    ImportTag_Global = 3,
} ImportTag, ExportTag;

#define ExportTag_Function ImportTag_Function
#define ExportTag_Table ImportTag_Table
#define ExportTag_Memory ImportTag_Memory
#define ExportTag_Global ImportTag_Global

struct MemoryType
{
    Limits limits;
};

struct ImportFunction
{
};

struct ImportTable
{
};

struct ImportMemory
{
};

struct GlobalType
{
    ValueType value_type;
    bool is_mutable;
};

struct Import
{
    Import() : tag ((ImportTag)-1) { }

    String module;
    String name;
    ImportTag tag;
    // TODO virtual functions to model union
    union
    {
        uint function;
        MemoryType memory;
        GlobalType global;
        TableType table;
    };
};

struct FuncAddr // TODO
{
};

struct TableAddr // TODO
{
};

struct MemAddr // TODO
{
};

struct GlobalAddr // TODO
{
};

struct ExternalValue // external to a module, an export instance
{
    union
    {
        FuncAddr* func;
        TableAddr* table;
        MemAddr* mem;
        GlobalAddr* global;
    };
};

struct ExportInstance // work in progress
{
    String name;
    ExternalValue external_value;
};

struct ModuleInstance // work in progress
{
    ModuleInstance (Module* mod);

    Module* module;
    std::vector <uint8> memory;
    std::vector <FuncAddr*> funcs;
    std::vector <TableAddr*> tables;
    //std::vector <NenAddr*> mem; // mem [0] => memory for now
    std::vector <StackValue> globals;
    std::vector <ExportInstance> exports;
};

struct FunctionInstance // work in progress
{
    ModuleInstance* module_instance;
    FunctionType* function_type;
    void* host_code; // TODO
    Code* code; // TODO
};

struct Function // section3
{
    Function () : function_type_index (0), function_index (0), import (false) { }

    // Functions are split between two sections: types in section3, locals/body in section10
    uint function_type_index;
    size_t function_index; // TODO needed?
    bool import; // TODO needed?
};

struct Global
{
    GlobalType global_type;
    std::vector <DecodedInstruction> init;
};

struct Element
{
    uint table;
    std::vector <DecodedInstruction> offset_instructions;
    uint offset;
    std::vector <uint> functions;
};

struct Export
{
    Export () :
        tag ((ExportTag)-1), is_start (false), is_main (false) { }

    Export (const Export& e)
    {
        printf ("copy export %X %X %X %X\n", tag, is_main, is_start, table);
        memcpy (this, &e, sizeof (e));
    }

    void operator = (const Export& e);

    ExportTag tag;
    String name;
    bool is_start;
    bool is_main;
    union
    {
        uint function;
        uint memory;
        uint table;
        uint global;
    };
};

struct Data // section11
{
    Data () : memory (0), bytes (0) { }

    uint memory;
    std::vector <DecodedInstruction> expr;
    void* bytes;
};

struct Code // section3 and section10
{
    Code () : size (0), cursor (0), import (false)
    {
    }

    size_t size;
    uint8* cursor;
    std::vector <ValueType> locals;
    std::vector <DecodedInstruction> decoded_instructions; // section10
    bool import;
};

// Initial representation of X and XSection are the same.
// This might evolve, i.e. into separate TypesSection and Types,
// or just Types that is not Section.

struct FunctionType
{
    // CONSIDER pointer into mmf
    std::vector <ValueType> parameters;
    std::vector <ValueType> results;

    bool operator == (const FunctionType& other) const
    {
        return parameters == other.parameters &&
            results == other.results;
    }
};

struct Module
{
    Module () : base (0), file_size (0), end (0), start (0), main (0),
        import_function_count (0),
        import_table_count (0),
        import_memory_count (0),
        import_global_count (0)
    {
    }

    MemoryMappedFile mmf;
    uint8* base;
    uint64 file_size;
    uint8* end;
    Section sections [12];
    //std::vector<std::shared_ptr<Section>> custom_sections; // FIXME

    std::vector <FunctionType> function_types; // section1 function signatures
    std::vector <Import> imports; // section2
    std::vector <Function> functions; // section3 and section10 function declarations
    std::vector <TableType> tables; // section4 indirect tables
    std::vector <uint8> memory; // section5 memory configuration
    std::vector <Global> globals; // section6
    std::vector <Export> exports; // section7
    std::vector <Element> elements; // section9 table initialization
    std::vector <Code> code; // section10
    std::vector <Data> data; // section11 memory initialization

    Export* start;
    Export* main;

    size_t import_function_count;
    size_t import_table_count;
    size_t import_memory_count;
    size_t import_global_count;

    String read_string (uint8*& cursor);

    uint read_i32 (uint8*& cursor);
    uint64 read_i64 (uint8*& cursor);
    float read_f32 (uint8*& cursor);
    double read_f64 (uint8*& cursor);

    uint read_byte (uint8*& cursor);
    uint read_varuint7 (uint8*& cursor);
    uint read_varuint32 (uint8*& cursor);

    void read_vector_varuint32 (std::vector<uint>&, uint8*& cursor);
    Limits read_limits (uint8*& cursor);
    MemoryType read_memorytype (uint8*& cursor);
    GlobalType read_globaltype (uint8*& cursor);
    TableType read_tabletype (uint8*& cursor);
    ValueType read_valuetype (uint8*& cursor);
    BlockType read_blocktype(uint8*& cursor);
    TableElementType read_elementtype (uint8*& cursor);
    bool read_mutable (uint8*& cursor);
    void read_section (uint8*& cursor);
    void read_module (const char* file_name);
    void read_vector_ValueType (std::vector <ValueType>& result, uint8*& cursor);
    void read_function_type (FunctionType& functionType, uint8*& cursor);

    void read_types (uint8*& cursor);
    void read_imports (uint8*& cursor);
    void read_functions (uint8*& cursor);
    void read_tables (uint8*& cursor);
    void read_memory (uint8*& cursor);
    void read_globals (uint8*& cursor);
    void read_exports (uint8*& cursor);
    void read_start (uint8*& cursor)
    {
        ThrowString ("Start::read not yet implemented");
    }
    void read_elements (uint8*& cursor);
    void read_code (uint8*& cursor);
    void read_data (uint8*& cursor);
};

static
InstructionEnum
DecodeInstructions (Module* module, std::vector <DecodedInstruction>& instructions, uint8*& cursor);

void Module::read_data (uint8*& cursor)
{
    const uint size1 = read_varuint32 (cursor);
    printf ("reading data11 size:%X\n", size1);
    data.resize (size1);
    for (uint i = 0; i < size1; ++i)
    {
        Data& a = data [i];
        a.memory = read_varuint32 (cursor);
        DecodeInstructions (this, a.expr, cursor);
        const uint size2 = read_varuint32 (cursor);
        if (cursor + size2 > end)
            ThrowString ("data out of bounds");
        a.bytes = cursor;
        printf ("data [%X]:{%X}\n", i, cursor [0]);
        cursor += size2;
    }
    printf ("read data11 size:%X\n", size1);
}

void Module::read_code (uint8*& cursor)
{
    printf ("reading CodeSection10\n");
    const size_t size = read_varuint32 (cursor);
    printf ("reading CodeSection size:%" FORMAT_SIZE "X\n", size);
    if (cursor + size > end)
        ThrowString (StringFormat ("code out of bounds cursor:%p end:%p size:%" FORMAT_SIZE "X line:%X", cursor, end, size, __LINE__));
    const size_t old = code.size ();
    Assert (old == import_function_count);
    code.resize (old + size);
    for (size_t i = 0; i < size; ++i)
    {
        Code& a = code [old + i];
        a.import = false;
        a.size = read_varuint32 (cursor);
        if (cursor + a.size > end)
            ThrowString (StringFormat ("code out of bounds cursor:%p end:%p size:%" FORMAT_SIZE "X line:%X", cursor, end, a.size, __LINE__));
        a.cursor = cursor;
        printf ("code [%" FORMAT_SIZE "X]: %p/%" FORMAT_SIZE "X\n", i, a.cursor, a.size);
        if (a.size)
        {
            //printf (InstructionName (cursor [0]));
            cursor += a.size;
        }
    }
}

void Module::read_elements (uint8*& cursor)
{
    const uint size1 = read_varuint32 (cursor);
    printf ("reading section9 elements size1:%X\n", size1);
    elements.resize (size1);
    for (uint i = 0; i < size1; ++i)
    {
        Element& a = elements [i];
        a.table = read_varuint32 (cursor);
        DecodeInstructions (this, a.offset_instructions, cursor);
        const uint size2 = read_varuint32 (cursor);
        a.functions.resize (size2);
        for (uint j = 0; j < size2; ++j)
        {
            uint& b = a.functions [j];
            b = read_varuint32 (cursor);
            printf ("elem.function [%X/%X]:%X\n", j, size2, b);
        }
    }
    printf ("read elements9 size:%X\n", size1);
}

void Module::read_exports (uint8*& cursor)
{
    printf ("reading section 7\n");
    const uint size = read_varuint32 (cursor);
    printf ("reading exports7 count:%X\n", size);
    exports.resize (size);
    for (uint i = 0; i < size; ++i)
    {
        Export& a = exports [i];
        a.name = read_string (cursor);
        a.tag = (ExportTag)read_byte (cursor);
        a.function = read_varuint32 (cursor);
        a.is_main = a.name.builtin == BuiltinString_main;
        a.is_start = a.name.builtin == BuiltinString_start;
        printf ("read_export %X:%X %s tag:%X index:%X is_main:%X is_start:%X\n", i, size, a.name.c_str (), a.tag, a.function, a.is_main, a.is_start);

        if (a.is_start)
        {
            Assert (!start);
            start = &a;
        }
        else if (a.is_main)
        {
            Assert (!main);
            main = &a;
        }
    }
    printf ("read exports7 size:%X\n", size);
}

void Module::read_globals (uint8*& cursor)
{
    //printf ("reading section 6\n");
    const uint size = read_varuint32 (cursor);
    printf ("reading globals6 size:%X\n", size);
    globals.resize (size);
    for (uint i = 0; i < size; ++i)
    {
        Global& a = globals [i];
        a.global_type = read_globaltype (cursor);
        printf ("read_globals %X:%X value_type:%X  mutable:%X init:%p\n", i, size, a.global_type.value_type, a.global_type.is_mutable, cursor);
        DecodeInstructions (this, a.init, cursor);
        // Init points to code -- Instructions until end of block 0x0B Instruction.
    }
    printf ("read globals6 size:%X\n", size);
}

void Module::read_functions (uint8*& cursor)
{
    printf ("reading section 3\n");
    const size_t old = functions.size ();
    Assert (old == import_function_count);
    const size_t size = read_varuint32 (cursor);
    functions.resize (old + size);
    for (size_t i = 0; i < size; ++i)
    {
        printf ("read_function %" FORMAT_SIZE "X:%" FORMAT_SIZE "X\n", i, size);
        Function& a = functions [old + i];
        a.function_type_index = read_varuint32 (cursor);
        a.function_index = i + old; // TODO probably not needed
        a.import = false; // TODO probably not needed
    }
    printf ("read section 3\n");
}

void Module::read_imports (uint8*& cursor)
{
    printf ("reading section 2\n");
    const size_t size = read_varuint32 (cursor);
    imports.resize (size);
    // TODO two passes to limit realloc?
    for (uint i = 0; i < size; ++i)
    {
        Import& r = imports [i];
        r.module = read_string (cursor);
        r.name = read_string (cursor);
        ImportTag tag = r.tag = (ImportTag)read_byte (cursor);
        printf ("import %s.%s %X\n", r.module.c_str (), r.name.c_str (), (uint)tag);
        switch (tag)
        {
            // TODO more specific import type and vtable?
        case ImportTag_Function:
            r.function = read_varuint32 (cursor); // TODO probably not needed
            ++import_function_count;
            // TODO for each import type
            functions.resize (functions.size () + 1);
            functions.back ().function_index = functions.size () - 1; // TODO remove this field
            functions.back ().function_type_index = r.function;
            functions.back ().import = true; // TODO needed?
            break;
        case ImportTag_Table:
            r.table = read_tabletype (cursor);
            ++import_table_count;
            break;
        case ImportTag_Memory:
            r.memory = read_memorytype (cursor);
            ++import_memory_count;
            break;
        case ImportTag_Global:
            r.global = read_globaltype (cursor);
            ++import_global_count;
            break;
        default:
            ThrowString ("invalid ImportTag");
        }
    }
    printf ("read section 2 import_function_count:%" FORMAT_SIZE "X import_table_count:%" FORMAT_SIZE "X import_memory_count:%" FORMAT_SIZE "X import_global_count:%" FORMAT_SIZE "X\n",
        import_function_count,
        import_table_count,
        import_memory_count,
        import_global_count);

    // TODO fill in more about imports?
    Code imported_code;
    imported_code.import = true;
    code.resize (import_function_count, imported_code);
}

void Module::read_vector_ValueType (std::vector<ValueType>& result, uint8*& cursor)
{
    const uint size = read_varuint32 (cursor);
    result.resize (size);
    for (uint i = 0; i < size; ++i)
        result [i] = read_valuetype (cursor);
}

void Module::read_function_type (FunctionType& functionType, uint8*& cursor)
{
    read_vector_ValueType (functionType.parameters, cursor);
    read_vector_ValueType (functionType.results, cursor);
}

void Module::read_types (uint8*& cursor)
{
    printf ("reading section 1\n");
    const size_t size = read_varuint32 (cursor);
    function_types.resize (size);
    for (size_t i = 0; i < size; ++i)
    {
        const uint marker = read_byte (cursor);
        if (marker != 0x60)
            ThrowString ("malformed2 in Types::read");
        read_function_type (function_types [i], cursor);
    }
    printf ("read section 1\n");
}

static
InstructionEnum
DecodeInstructions (Module* module, std::vector <DecodedInstruction>& instructions, uint8*& cursor)
{
    uint b0;
    while (1)
    {
        InstructionEncoding e;
        DecodedInstruction i;
        switch (b0 = module->read_byte (cursor)) // TODO multi-byte instructions
        {
        case BlockEnd:
            return BlockEnd;
        default:
            e = instructionEncode [b0];
            if (e.fixed_size == 0)
            {
#if _WIN32
                if (IsDebuggerPresent ()) DebugBreak();
#endif
                ThrowString ("reserved");
            }
            i.name = e.name;
            if (e.fixed_size == 2) // TODO
                if (module->read_byte (cursor))
                    ThrowString ("second byte not 0");
            printf ("decode1 %s\n", InstructionName (i.name));
            switch (e.immediate)
            {
            case Imm_sequence:
                i.blockType = module->read_blocktype (cursor);
                InstructionEnum next;
                next = DecodeInstructions (module, i.sequence, cursor);
                Assert (next == BlockEnd || (i.name == If && next == Else));
                break;
            case Imm_memory:
                i.align = module->read_varuint32 (cursor);
                i.offset = module->read_varuint32 (cursor);
                break;
            case Imm_none:
                break;
            case Imm_global:
                i.u32 = module->read_varuint32 (cursor);
                break;
            case Imm_function:
            case Imm_local:
            case Imm_label:
            case Imm_type:
                i.u32 = module->read_varuint32 (cursor);
                break;
            default:
                ThrowString ("unknown immediate");
                break;
            case Imm_i32: // Spec is confusing here, signed or unsigned.
                i.u32 = module->read_i32 (cursor);
                break;
            case Imm_i64: // Spec is confusing here, signed or unsigned.
                i.u64 = module->read_i64 (cursor);
                break;
            case Imm_f32:
                i.f32 = module->read_f32 (cursor);
                break;
            case Imm_f64:
                i.f64 = module->read_f64 (cursor);
                break;
            case Imm_vecLabel:
                module->read_vector_varuint32 (i.vecLabel, cursor);
                break;
            }
            printf ("decode2 %s 0x%X %d\n", InstructionName (i.name), i.i32, i.i32);
            instructions.push_back (i);
        }
    }
    return (InstructionEnum)b0;
}

static
void
DecodeFunction (Module* module, Code& code, uint8*& cursor)
{
    uint local_type_count = module->read_varuint32 (cursor);
    printf ("local_type_count:%X\n", local_type_count);
    for (uint i = 0; i < local_type_count; ++i)
    {
        uint j = module->read_varuint32 (cursor);
        ValueType value_type = module->read_valuetype (cursor);
        printf ("local_type_count %X-of-%X count:%X type:%X\n", i, local_type_count, j, value_type);
        code.locals.resize (code.locals.size () + j, value_type);
    }
    DecodeInstructions (module, code.decoded_instructions, cursor);
}

const
SectionTraits section_traits [ ] =
{
    { 0 },
#define SECTIONS        \
    SECTION (TypesSection, read_types)     \
    SECTION (ImportsSection, read_imports)   \
    SECTION (FunctionsSection, read_functions) \
    SECTION (TablesSection, read_tables)    \
    SECTION (MemorySection, read_memory)    \
    SECTION (GlobalsSection, read_globals)   \
    SECTION (ExportsSection, read_exports)   \
    SECTION (StartSection, read_start)     \
    SECTION (ElementsSection, read_elements) \
    SECTION (CodeSection, read_code) \
    SECTION (DataSection, read_data) \

#undef SECTION
#define SECTION(x, read) {#x, &Module::read},
SECTIONS

};

uint Module::read_i32 (uint8*& cursor)
// Unspecified signedness is unsigned. Spec is unclear.
{
    return read_varuint32 (cursor);
}

uint64 Module::read_i64 (uint8*& cursor)
// Unspecified signedness is unsigned. Spec is unclear.
{
    return read_varuint64 (cursor, end);
}

float Module::read_f32 (uint8*& cursor)
// floats are not variably sized? Spec is unclear due to fancy notation
// getting in the way.
{
    union {
        uint8 bytes [4];
        float f32;
    } u;
    for (int i = 0; i < 4; ++i)
        u.bytes [i] = (uint8)read_byte (cursor);
    return u.f32;
}

double Module::read_f64 (uint8*& cursor)
// floats are not variably sized? Spec is unclear due to fancy notation
// getting in the way.
{
    union {
        uint8 bytes [8];
        double f64;
    } u;
    for (int i = 0; i < 8; ++i)
        u.bytes [i] = (uint8)read_byte (cursor);
    return u.f64;
}

uint Module::read_varuint7 (uint8*& cursor)
{
    // TODO move implementation here, i.e. for context, for errors
    return w3::read_varuint7 (cursor, end);
}

uint Module::read_byte (uint8*& cursor)
{
    // TODO move implementation here, i.e. for context, for errors
    return w3::read_byte (cursor, end);
}

// TODO efficiency
// i.e. string_view or such pointing right into the mmap
String Module::read_string (uint8*& cursor)
{
    const uint size = read_varuint32 (cursor);
    if (size + cursor > end)
        ThrowString ("malformed in read_string");
    // TODO UTF8 handling
    String a;
    a.data = (char*)cursor;
    a.size = size;

    // TODO string recognizer?
    printf ("read_string %X:%.*s\n", size, size, cursor);
    if (size == 7 && !memcmp (cursor, "$_start", 7))
    {
        a.builtin = BuiltinString_start;
    }
    else if (size == 5 && !memcmp (cursor, "_main", 5))
    {
        a.builtin = BuiltinString_main;
    }
    cursor += size;
    return a;
}

void Module::read_vector_varuint32 (std::vector<uint>& result, uint8*& cursor)
{
    const uint size = read_varuint32 (cursor);
    result.resize (size);
    for (uint i = 0; i < size; ++i)
        result [i] = read_varuint32 (cursor);
}

uint Module::read_varuint32 (uint8*& cursor)
{
    // TODO move implementation here, i.e. for context, for errors
    return w3::read_varuint32 (cursor, end);
}

Limits Module::read_limits (uint8*& cursor)
{
    Limits limits = { };
    const uint tag = read_byte (cursor);
    switch (tag)
    {
    case 0:
    case 1:
        break;
    default:
        ThrowString ("invalid limit tag");
        break;
    }
    limits.hasMax = (tag == 1);
    limits.min = read_varuint32 (cursor);
    if (limits.hasMax)
        limits.max = read_varuint32 (cursor);
    return limits;
}

MemoryType Module::read_memorytype (uint8*& cursor)
{
    const MemoryType m = { read_limits (cursor) };
    return m;
}

bool Module::read_mutable (uint8*& cursor)
{
    const uint m = read_byte (cursor);
    switch (m)
    {
    case 0:
    case 1: break;
    default:
        ThrowString ("invalid mutable");
    }
    return m == 1;
}

ValueType Module::read_valuetype (uint8*& cursor)
{
    const uint value_type = read_byte (cursor);
    switch (value_type)
    {
    default:
        ThrowString (StringFormat ("invalid ValueType:%X", value_type));
        break;
    case ValueType_i32:
    case ValueType_i64:
    case ValueType_f32:
    case ValueType_f64:
        break;
    }
    return (ValueType)value_type;
}

BlockType Module::read_blocktype(uint8*& cursor)
{
    const uint block_type = read_byte (cursor);
    switch (block_type)
    {
    default:
        ThrowString (StringFormat ("invalid BlockType:%X", block_type));
        break;
    case ValueType_i32:
    case ValueType_i64:
    case ValueType_f32:
    case ValueType_f64:
    case ResultType_empty:
        break;
    }
    return (BlockType)block_type;
}

GlobalType Module::read_globaltype (uint8*& cursor)
{
    GlobalType globalType = { };
    globalType.value_type = read_valuetype (cursor);
    globalType.is_mutable = read_mutable (cursor);
    return globalType;
}

TableElementType Module::read_elementtype (uint8*& cursor)
{
    TableElementType elementType = (TableElementType)read_byte (cursor);
    if (elementType != TableElementType_funcRef)
        ThrowString ("invalid elementType");
    return elementType;
}

TableType Module::read_tabletype (uint8*& cursor)
{
    TableType tableType = { };
    tableType.elementType = read_elementtype (cursor);
    tableType.limits = read_limits (cursor);
    printf ("read_tabletype:type:%X min:%X hasMax:%X max:%X\n", tableType.elementType, tableType.limits.min, tableType.limits.hasMax, tableType.limits.max);
    return tableType;
}

void Module::read_memory (uint8*& cursor)
{
    printf ("reading section 5\n");
    auto limits = read_limits (cursor);
    // Hello world does not have this section.
}

void Module::read_tables (uint8*& cursor)
{
    uint size = read_varuint32 (cursor);
    printf ("reading tables size:%X\n", size);
    AssertFormat (size == 1, ("%X", size));
    tables.resize (size);
    for (uint i = 0; i < size; ++i)
    {
        tables [0] = read_tabletype (cursor);
    }
}

void Module::read_section (uint8*& cursor)
{
    uint payload_size = 0;
    uint8* payload = 0;
    uint name_size = 0;

    const uint id = read_varuint7 (cursor);

    if (id > 11)
        ThrowString (StringFormat ("malformed line:%d id:%X payload:%p payload_size:%X base:%p end:%p", __LINE__, id, payload, payload_size, base, end)); // UNDONE context (move to module or section)

    payload_size = read_varuint32 (cursor);
    printf ("%s payload_size:%X\n", __func__, payload_size);
    payload = cursor;
    name_size = 0;
    const char* name = 0;
    if (id == 0)
    {
        name_size = read_varuint32 (cursor);
        name = (char*)cursor;
        if (cursor + name_size > end)
            ThrowString (StringFormat ("malformed %d", __LINE__)); // UNDONE context (move to module or section)
    }
    if (payload + payload_size > end)
        ThrowString (StringFormat ("malformed line:%d id:%X payload:%p payload_size:%X base:%p end:%p", __LINE__, id, payload, payload_size, base, end)); // UNDONE context (move to module or section)

    cursor = payload + payload_size;

    if (id == 0)
    {
        printf ("skipping custom section:.%.*s\n", name_size, name);
        // UNDONE custom sections
        return;
    }

    Section& section = sections [id];
    section.id = id;
    section.name.data = (char*)name;
    section.name.size = name_size;
    section.payload_size = payload_size;
    section.payload = payload;
    (this->*section_traits [id].read) (payload);
    if (payload != cursor)
        ThrowString (StringFormat ("failed to read section:%X payload:%p cursor:%p\n", id, payload, cursor));
}

void Module::read_module (const char* file_name)
{
    mmf.read (file_name);
    base = (uint8*)mmf.base;
    file_size = mmf.file.get_file_size ();
    end = file_size + (uint8*)base;

    if (file_size < 8)
        ThrowString (StringFormat ("too small %s", file_name));

    uintLE& magic = (uintLE&)*base;
    uintLE& version = (uintLE&)*(base + 4);
    printf ("magic: %X\n", (uint)magic);
    printf ("version: %X\n", (uint)version);

    if (memcmp (&magic,"\0asm", 4))
        ThrowString (StringFormat ("incorrect magic: %X", (uint)magic));

    if (version != 1)
        ThrowString (StringFormat ("incorrect version: %X", (uint)version));

    // Valid module with no section
    if (file_size == 8)
        return;

    uint8* cursor = base + 8;
    while (cursor < end)
        read_section (cursor);

    Assert (cursor == end);
}

// TODO once we have Validate, Interp, Jit, CppGen,
// we will invert this structure and have a class per instruction with those 4 virtual functions.
// Or we will token-paste those names on to the instruction names,
// in order to avoid virtual function call cost. Let's get Interp working first.
struct IInterp
{
    virtual ~IInterp ()
    {
    }

    virtual void Reserved (DecodedInstruction* instr) = 0;
#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) void name (DecodedInstruction* instr) { abort (); }
INSTRUCTIONS
};

static
void
Overflow ()
{
    Assert (!"Overflow");
}

struct Interp : Stack, IInterp
{
private:
    Interp(const Interp&);
    void operator =(const Interp&);
public:
    Interp() : stack (*this)
    {
        frame = 0;
    }

    void* LoadStore (DecodedInstruction*, size_t size);

    Module* module;
    ModuleInstance* module_instance;
    Frame* frame;

    // FIXME multiple modules

    Stack& stack;

    void Invoke (Function&);

    void interp (Module* mod, Export* emain)
    {
#if 0
        Assert (mod && emain && emain->is_main && emain->tag == ExportTag_Function);
        Assert (emain->function < mod->functions.size ());
        Assert (emain->function < mod->code.size ());
        Assert (mod->functions.size () == mod->code.size ());

//      Function& fmain = mod->functions [emain->function];
        Code& cmain = mod->code [emain->function];
#endif
        // Simulate call to initial function.
        // instantiate module
        ModuleInstance instance (mod);
        this->module_instance = &instance;
        this->module = mod;
        Function function { };
        function.function_index = mod->import_function_count + 1; // TODO $_start in .name custom section
        Invoke (function);
    }

    void Reserved (DecodedInstruction*);

#undef RESERVED
#define RESERVED(b0) INSTRUCTION (0x ## b0, 0, 0, Reserved ## b0, Imm_none, 0, 0, Type_none, Type_none, Type_none, Type_none) { Reserved (instr); }

#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) ; void name (DecodedInstruction* instr)
INSTRUCTIONS
    ;
};

//unreach
//memsize
//memgrow
//call
//calli
//if
//loop
//else
//drop
//select
//local get set tree
//gloal get set
//block
//br
//brif
//brtable
//ret

void* Interp::LoadStore (DecodedInstruction* instr, size_t size)
{
    // TODO Not clear from spec and paper what to do here, despite
    // focused discussion on it.
    // Why is the operand signed??
    size_t effective_address;
    const ptrdiff_t i = pop_i32 ();
    const size_t offset = instr->offset;
    if (i >= 0)
    {
        effective_address = offset + (uint)i;
        if (effective_address < offset)
            Overflow ();
    }
    else
    {
        const size_t u = int_magnitude ((int)i);
        if (u > offset)
            Overflow ();
        effective_address = offset - u;
    }
    if (effective_address > UINT_MAX - size)
        Overflow ();
    Assert (effective_address + size < frame->module->memory.size ());
    return &frame->module->memory [effective_address];
}

#undef INTERP
#define INTERP(x) void Interp::x (DecodedInstruction* instr)

INTERP (Call)
{
    const size_t function_index = instr->u32;
    Assert (function_index < module->functions.size ());
    Function& function = module->functions [function_index];
    function.function_index = function_index; // TODO remove this
    Invoke (module->functions [function_index]);
}

void Interp::Invoke (Function& function)
{
    __debugbreak ();
    // Decode function upon first call.
    // TODO thread safety
    // TODO merge with calli (invoke)

    Code& code = module->code [function.function_index];
    uint8* cursor = code.cursor;
    if (cursor)
    {
        DecodeFunction (module, code, cursor);
        code.cursor = 0;
    }
    size_t size = code.decoded_instructions.size ();
    Assert (size);

    // TODO cross-module calls
    // TODO calling embedding
    // setup frame
    StackValue f { };
    Frame frame_value; // TODO within StackValue by value instead of by pointer? Changed due to circular typing.
    f.frame = &frame_value;
    frame_value.code = &code;
    f.type = StackTag_Frame;
    frame_value.next = this->frame;
    frame_value.module = this->module; // TODO cross module calls
    frame_value.module_instance = this->module_instance; // TODO cross module calls
    frame_value.function_index = function.function_index;
    const size_t local_count = code.locals.size ();
    frame_value.local_count = local_count;

    this->frame = &frame_value;

    // CONSIDER put the interp loop elsewhere
    // Invoke would adjust member data and return to it

    uint function_type_index = function.function_type_index;

    Assert (function_type_index < module->function_types.size ());
    FunctionType& function_type = module->function_types [function_type_index];

    // Push locals on stack.
    // TODO params also
    // TODO reserve (size () + local_count);
    for (size_t i = 0; i < local_count; ++i)
    {
        StackValue value = {StackTag_Value, { code.locals [i] } };
        push_value (value);
    }
    // Provide for indexing locals.
    frame_value.locals = stack.end () - (ptrdiff_t)local_count;

    // TODO provide for separate depth -- i.e. here is now 0; locals/params cannot be popped

    DecodedInstruction* instr = &code.decoded_instructions [0];
    DecodedInstruction* end = instr + size;
    for (; instr < end; ++instr)
    {
        switch (instr->name)
        {
#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, pop, push, in0, in1, in2, out0) case w3::name: printf("interp%s x:%X u:%u i:%i\n", #name, instr->u32, instr->u32, instr->u32); this->name (instr); break;
INSTRUCTIONS
        }
    }
    // TODO handle ret
    __debugbreak ();
}

INTERP (Block)
{
	Assert (!"Block"); // not yet implemented
}

INTERP (Loop)
{
	Assert (!"Loop"); // not yet implemented
}

INTERP (MemGrow)
{
	Assert (!"MemGrow"); // not yet implemented
}

INTERP (MemSize)
{
	Assert (!"MemSize"); // not yet implemented
}

INTERP (Global_set)
{
    uint index = instr->u32;
    Assert (index < module->globals.size ());
    AssertFormat (tag () == module->globals [index].global_type.value_type, ("%X %X", tag (), module->globals [index].global_type.value_type));
    module_instance->globals [index].value.value = value ();
    pop_value ();
}

INTERP (Global_get)
{
    uint index = instr->u32;
    Assert (index < module->globals.size ());
    StackValue value {};
    value.type = StackTag_Value;
    value.value.tag = module->globals [index].global_type.value_type; // TODO initialize the instance with types?
    value.value.value = module_instance->globals [index].value.value;
    push_value (value);
}

INTERP (Local_set)
{
    Local_tee (instr);
    pop_value ();
}

INTERP (Local_tee)
{
    // TODO params
    Assert (instr->u32 < frame->local_count);
    //Assert (value_depth >= 1);
    AssertFormat (tag () == frame->code->locals [instr->u32], ("%X %X", tag (), frame->code->locals [instr->u32]));
    AssertFormat (tag () == frame->locals [instr->u32].value.tag, ("%X %X", tag (), frame->locals [instr->u32].value.tag));
    frame->locals [instr->u32].value.value = value ();
}

INTERP (Local_get)
{
    Assert (!"Local_get"); // not yet implemented
}

INTERP (If)
{
    Assert (!"If"); // not yet implemented
}

INTERP (Else)
{
    Assert (!"Else"); // not yet implemented
}

INTERP (BlockEnd)
{
    Assert (!"BlockEnd"); // not yet implemented
}

INTERP (BrIf)
{
    Assert (!"BrIf"); // not yet implemented
}

INTERP (BrTable)
{
    Assert (!"BrTable"); // not yet implemented
}

INTERP (Ret)
{
    Assert (!"Ret"); // not yet implemented
}

INTERP (Br)
{
    Assert (!"Br"); // not yet implemented
}

INTERP (Select)
{
    Assert (!"Select"); // not yet implemented
}

INTERP (Calli)
{
    // TODO signed or unsigned
    // call is unsigned
    const int sfunction_index = pop_i32 ();
    Assert (sfunction_index >= 0);
    const size_t function_index = (uint)sfunction_index;

    const uint type_index1 = instr->u32;

    // This seems like it could be validated earlier.
    Assert (function_index < module->functions.size ());

    Function& function = module->functions [function_index];

    const uint type_index2 = function.function_type_index;

    Assert (type_index1 < module->function_types.size ());
    Assert (type_index2 < module->function_types.size ());

    const FunctionType& type1 = module->function_types [type_index1];
    const FunctionType& type2 = module->function_types [type_index2];

    Assert (type_index2 == type_index1 || type1 == type2);

    Assert (type1.results.size () <= 1); // future

    Invoke (function);
}

ModuleInstance::ModuleInstance (Module* mod)
{
    module = mod;
    //memory.resize (mod->memory.size (), 0); // TODO intialize
    globals.resize (mod->globals.size (), StackValue ()); // TODO intialize
}

INTERP (Unreach)
{
    Assert (!"unreach");
}

INTERP (i32_Const)
{
    push_i32 (instr->i32);
}

INTERP (i64_Const)
{
    push_i64 (instr->i64);
}

INTERP (f32_Const)
{
    push_f32 (instr->f32);
}

INTERP (f64_Const)
{
    push_f64 (instr->f64);
}

INTERP (i32_Load)
{
    push_i32 (*(int*)LoadStore (instr, 4));
}

INTERP (i32_Load8s)
{
    push_i32 (*(int8*)LoadStore (instr, 1));
}

INTERP (i32_Load16s)
{
    push_i32 (*(int16*)LoadStore (instr, 2));
}

INTERP (i32_Load8u)
{
    push_i32 (*(uint8*)LoadStore (instr, 1));
}

INTERP (i32_Load16u)
{
    push_i32 (*(uint16*)LoadStore (instr, 2));
}

INTERP (i64_Load)
{
    push_i64 (*(int64*)LoadStore (instr, 8));
}

INTERP (i64_Load8s)
{
    push_i64 (*(int8*)LoadStore (instr, 1));
}

INTERP (i64_Load16s)
{
    push_i64 (*(int16*)LoadStore (instr, 2));
}

INTERP (i64_Load8u)
{
    push_i64 (*(uint8*)LoadStore (instr, 1));
}

INTERP (i64_Load16u)
{
    push_i64 (*(uint16*)LoadStore (instr, 2));
}

INTERP (i64_Load32s)
{
    push_i64 (*(int*)LoadStore (instr, 4));
}

INTERP (i64_Load32u)
{
    push_i64 (*(uint*)LoadStore (instr, 4));
}

INTERP (f32_Load)
{
    push_f32 (*(float*)LoadStore (instr, 4));
}

INTERP (f64_Load)
{
    push_f64 (*(double*)LoadStore (instr, 8));
}

INTERP (i32_Store)
{
    const uint a = pop_u32 ();
    *(uint*)LoadStore (instr, 4) = a;
}

INTERP (i32_Store8)
{
    const uint a = pop_u32 ();
    *(uint8*)LoadStore (instr, 1) = (uint8)(a & 0xFF);
}

INTERP (i32_Store16)
{
    const uint a = pop_u32 ();
    *(uint16*)LoadStore (instr, 1) = (uint16)(a & 0xFFFF);
}

INTERP (i64_Store8)
{
    const uint64 a = pop_u64 ();
    *(uint8*)LoadStore (instr, 1) = (uint8)(a & 0xFF);
}

INTERP (i64_Store16)
{
    const uint64 a = pop_u64 ();
    *(uint16*)LoadStore (instr, 2) = (uint16)(a & 0xFFFF);
}

INTERP (i64_Store32)
{
    const uint64 a = pop_u64 ();
    *(uint*)LoadStore (instr, 4) = (uint)(a & 0xFFFFFFFF);
}

INTERP (i64_Store)
{
    const uint64 a = pop_u64 ();
    *(uint64*)LoadStore (instr, 8) = a;
}

INTERP (f32_Store)
{
    float a = pop_f32 ();
    *(float*)LoadStore (instr, 4) = a;
}

INTERP (f64_Store)
{
    double a = pop_f64 ();
    *(double*)LoadStore (instr, 8) = a;
}

INTERP (Nop)
{
}

INTERP (Drop)
{
    pop_value ();
}

void Interp:: Reserved (DecodedInstruction* instr)
{
#if _WIN32
    if (IsDebuggerPresent ()) DebugBreak();
#endif
    static const char reserved [] = "reserved\n";
#if _WIN32
    if (IsDebuggerPresent())
    {
        OutputDebugStringA (reserved);
        DebugBreak();
    }
    _write (1, reserved, sizeof (reserved) - 1);
#else
    write (1, reserved, sizeof (reserved) - 1);
#endif
    abort ();
}

INTERP (Eqz_i32)
{
    push_bool (pop_i32 () == 0);
}

INTERP (Eqz_i64)
{
    push_bool (pop_i64 () == 0);
}

INTERP (Eq_i32)
{
    push_bool (pop_i32 () == pop_i32 ());
}

INTERP (Eq_i64)
{
    push_bool (pop_i64 () == pop_i64 ());
}

INTERP (Eq_f32)
{
    push_bool (pop_f32 () == pop_f32 ());
}

INTERP (Eq_f64)
{
    push_bool (pop_f64 () == pop_f64 ());
}

INTERP (Ne_i32)
{
    push_bool (pop_i32 () != pop_i32 ());
}

INTERP (Ne_i64)
{
    push_bool (pop_i64 () != pop_i64 ());
}

INTERP (Ne_f32)
{
    push_bool (pop_f32 () != pop_f32 ());
}

INTERP (Ne_f64)
{
    push_bool (pop_f64 () != pop_f64 ());
}

// Lt

INTERP (Lt_i32s)
{
    const int b = pop_i32 ();
    const int a = pop_i32 ();
    push_bool (a < b);
}

INTERP (Lt_i32u)
{
    const uint b = pop_u32 ();
    const uint a = pop_u32 ();
    push_bool (a < b);
}

INTERP (Lt_i64s)
{
    const int64 b = pop_i64 ();
    const int64 a = pop_i64 ();
    push_bool (a < b);
}

INTERP (Lt_i64u)
{
    const uint64 b = pop_u32 ();
    const uint64 a = pop_u32 ();
    push_bool (a < b);
}

INTERP (Lt_f32)
{
    const float b = pop_f32 ();
    const float a = pop_f32 ();
    push_bool (a < b);
}

INTERP (Lt_f64)
{
    const double b = pop_f64 ();
    const double a = pop_f64 ();
    push_bool (a < b);
}

// Gt

INTERP (Gt_i32s)
{
    const int b = pop_i32 ();
    const int a = pop_i32 ();
    push_bool (a > b);
}

INTERP (Gt_i32u)
{
    const uint b = pop_u32 ();
    const uint a = pop_u32 ();
    push_bool (a > b);
}

INTERP (Gt_i64s)
{
    const int64 b = pop_i64 ();
    const int64 a = pop_i64 ();
    push_bool (a > b);
}

INTERP (Gt_i64u)
{
    const uint64 b = pop_u32 ();
    const uint64 a = pop_u32 ();
    push_bool (a > b);
}

INTERP (Gt_f32)
{
    const float b = pop_f32 ();
    const float a = pop_f32 ();
    push_bool (a > b);
}

INTERP (Gt_f64)
{
    const double b = pop_f64 ();
    const double a = pop_f64 ();
    push_bool (a > b);
}

// Le

INTERP (Le_i32s)
{
    const int b = pop_i32 ();
    push_bool (pop_i32 () <= b);
}

INTERP (Le_i64s)
{
    const int64 b = pop_i64 ();
    push_bool (pop_i64 () <= b);

}

INTERP (Le_i32u)
{
    const uint b = pop_u32 ();
    push_bool (pop_u32 () <= b);
}

INTERP (Le_i64u)
{
    const uint64 b = pop_u64 ();
    push_bool (pop_u64 () <= b);

}

INTERP (Le_f32)
{
    const float z2 = pop_f32 ();
    const float z1 = pop_f32 ();
    push_bool (z1 <= z2);
}

INTERP (Le_f64)
{
    const double z2 = pop_f64 ();
    const double z1 = pop_f64 ();
    push_bool (z1 <= z2);
}

// Ge

INTERP (Ge_i32u)
{
    const uint b = pop_u32 ();
    const uint a = pop_u32 ();
    push_bool (a >= b);
}

INTERP (Ge_i64u)
{
    const uint64 b = pop_u64 ();
    const uint64 a = pop_u64 ();
    push_bool (a >= b);
}

INTERP (Ge_i32s)
{
    const int b = pop_i32 ();
    const int a = pop_i32 ();
    push_bool (a >= b);
}

INTERP (Ge_i64s)
{
    const int64 b = pop_i64 ();
    const int64 a = pop_i64 ();
    push_bool (a >= b);
}

INTERP (Ge_f32)
{
    const float b = pop_f32 ();
    const float a = pop_f32 ();
    push_bool (a >= b);
}

INTERP (Ge_f64)
{
    const double b = pop_f64 ();
    const double a = pop_f64 ();
    push_bool (a >= b);
}

template <typename T>
static
uint
count_set_bits (T a)
{
    uint n = 0;
    while (a)
    {
        n += (a & 1);
        a >>= 1;
    }
    return n;
}

template <typename T>
static
uint
count_trailing_zeros (T a)
{
    uint n = 0;
    while (!(a & 1))
    {
        ++n;
        a >>= 1;
    }
    return n;
}

template <typename T>
static
uint
count_leading_zeros (T a)
{
    uint n = 0;
    while (!(a & (((T)1) << ((sizeof (T) * 8) - 1))))
    {
        ++n;
        a <<= 1;
    }
    return n;
}

INTERP (Popcnt_i32)
{
    uint& a = u32 ();
#if _MSC_VER && (_M_AMD64 || _M_IX86)
    a = __popcnt (a);
#else
    a = count_set_bits (a);
#endif
}

INTERP (Popcnt_i64)
{
    uint64& a = u64 ();
#if _MSC_VER && _M_AMD64
    a = __popcnt64 (a);
#else
    a = count_set_bits (a);
#endif
}

INTERP (Ctz_i32)
{
    uint& a = u32 ();
    a = count_trailing_zeros (a);
}

INTERP (Ctz_i64)
{
    uint64& a = u64 ();
    a = count_trailing_zeros (a);
}

INTERP (Clz_i32)
{
    uint& a = u32 ();
    a = count_leading_zeros (a);
}

INTERP (Clz_i64)
{
    uint64& a = u64 ();
    a = count_leading_zeros (a);
}

INTERP (Add_i32)
{
    const int a = pop_i32 ();
    i32 () += a;
}

INTERP (Add_i64)
{
    const int64 a = pop_i64 ();
    i64 () += a;
}

INTERP (Sub_i32)
{
    const int a = pop_i32 ();
    i32 () -= a;
}

INTERP (Sub_i64)
{
    const int64 a = pop_i64 ();
    i64 () -= a;
}

INTERP (Mul_i32)
{
    const int a = pop_i32 ();
    i32 () *= a;
}

INTERP (Mul_i64)
{
    const int64 a = pop_i64 ();
    i64 () *= a;
}

INTERP (Div_s_i32)
{
    const int a = pop_i32 ();
    i32 () /= a;
}

INTERP (Div_u_i32)
{
    const uint a = pop_u32 ();
    u32 () /= a;
}

INTERP (Rem_s_i32)
{
    const int a = pop_i32 ();
    i32 () %= a;
}

INTERP (Rem_u_i32)
{
    const uint a = pop_u32 ();
    u32 () %= a;
}

INTERP (Div_s_i64)
{
    const int64 a = pop_i64 ();
    i64 () /= a;
}

INTERP (Div_u_i64)
{
    const uint64 a = pop_u64 ();
    u64 () /= a;
}

INTERP (Rem_s_i64)
{
    const int64 a = pop_i64 ();
    i64 () %= a;
}

INTERP (Rem_u_i64)
{
    const uint64 a = pop_u64 ();
    u64 () %= a;
}

INTERP (And_i32)
{
    const int a = pop_i32 ();
    i32 () &= a;
}

INTERP (And_i64)
{
    const int64 a = pop_i64 ();
    i64 () &= a;
}

INTERP (Or_i32)
{
    const int a = pop_i32 ();
    i32 () |= a;
}

INTERP (Or_i64)
{
    const int64 a = pop_i64 ();
    i64 () |= a;
}

INTERP (Xor_i32)
{
    const int a = pop_i32 ();
    i32 () ^= a;
}

INTERP (Xor_i64)
{
    const int64 a = pop_i64 ();
    i64 () ^= a;
}

INTERP (Shl_i32)
{
    const int a = pop_i32 ();
    i32 () <<= (a & 31);
}

INTERP (Shl_i64)
{
    const int64 a = pop_i64 ();
    i64 () <<= (a & 63);
}

INTERP (Shr_s_i32)
{
    const int b = pop_i32 ();
    i32 () <<= (b & 31);
}

INTERP (Shr_s_i64)
{
    const int64 b = pop_i64 ();
    i64 () <<= (b & 63);
}

INTERP (Shr_u_i32)
{
    const uint b = pop_u32 ();
    u32 () >>= (b & 31);
}

INTERP (Shr_u_i64)
{
    const uint64 b = pop_u64 ();
    u64 () >>= (b & 63);
}

INTERP (Rotl_i32)
{
    const int n = 32;
    const int b = (pop_i32 () & (n - 1));
    uint& r = u32 ();
    uint a = r;
#if _MSC_VER
    r = _rotl (a, b);
#else
    r = ((a << b) | (a >> (n - b)));
#endif
}

INTERP (Rotl_i64)
{
    const int n = 64;
    const int b = (pop_i64 () & (n - 1));
    uint64& r = u64 ();
    uint64 a = r;
#if _MSC_VER
    r = _rotl64 (a, b);
#else
    r = (a << b) | (a >> (n - b));
#endif
}

INTERP (Rotr_i32)
{
    const int n = 32;
    const int b = (int)(pop_u32 () & (n - 1));
    uint& r = u32 ();
    uint a = r;
#if _MSC_VER
    r = _rotr (a, b);
#else
    r = (a >> b) | (a << (n - b));
#endif
}

INTERP (Rotr_i64)
{
    const int n = 64;
    const int b = (int)(pop_u64 () & (n - 1));
    uint64& r = u64 ();
    uint64 a = r;
#if _MSC_VER
    r = _rotr64 (a, b);
#else
    r = (a >> b) | (a << (n - b));
#endif
}

INTERP (Abs_f32)
{
    float& z = f32 ();
    z = std::abs (z);
}

INTERP (Abs_f64)
{
    double& z = f64 ();
    z = std::abs (z);
}

INTERP (Neg_f32)
{
    f32 () *= -1;
}

INTERP (Neg_f64)
{
    f64 () *= -1;
}

INTERP (Ceil_f32)
{
    float& z = f32 ();
    z = ceilf (z);
}

INTERP (Ceil_f64)
{
    double& z = f64 ();
    z = ceil (z);
}

INTERP (Floor_f32)
{
    float& z = f32 ();
    z = floorf (z);
}

INTERP (Floor_f64)
{
    double& z = f64 ();
    z = floor (z);
}

INTERP (Trunc_f32)
{
    float& z = f32 ();
    z = truncf (z); // TODO C99
}

INTERP (Trunc_f64)
{
    double& z = f64 ();
    z = trunc (z);
}

INTERP (Nearest_f32)
{
    float& z = f32 ();
    z = roundf (z);
}

INTERP (Nearest_f64)
{
    double& z = f64 ();
    z = round (z);
}

INTERP (Sqrt_f32)
{
    float& z = f32 ();
    z = sqrtf (z);
}

INTERP (Sqrt_f64)
{
    double& z = f64 ();
    z = sqrt (z);
}

INTERP (Add_f32)
{
    const float a = pop_f32 ();
    f32 () += a;
}

INTERP (Add_f64)
{
    const double a = pop_f64 ();
    f64 () += a;
}

INTERP (Sub_f32)
{
    const float a = pop_f32 ();
    f32 () -= a;
}

INTERP (Sub_f64)
{
    const double a = pop_f64 ();
    f64 () -= a;
}

INTERP (Mul_f32)
{
    const float a = pop_f32 ();
    f32 () *= a;
}

INTERP (Mul_f64)
{
    const double a = pop_f64 ();
    f64 () *= a;
}

INTERP (Div_f32)
{
    const float a = pop_f32 ();
    f32 () /= a;
}

INTERP (Div_f64)
{
    const double a = pop_f64 ();
    f64 () /= a;
}

INTERP (Min_f32)
{
    const float z2 = pop_f32 ();
    float& z1 = f32 ();
    z1 = Min (z1, z2);
}

INTERP (Min_f64)
{
    const double z2 = pop_f64 ();
    double& z1 = f64 ();
    z1 = Min (z1, z2);
}

INTERP (Max_f32)
{
    const float z2 = pop_f32 ();
    float& z1 = f32 ();
    z1 = Max (z1, z2);
}

INTERP (Max_f64)
{
    const double z2 = pop_f64 ();
    double& z1 = f64 ();
    z1 = Max (z1, z2);
}

INTERP (Copysign_f32)
{
    const float z2 = pop_f32 ();
    float& z1 = f32 ();
    z1 = ((z2 < 0) != (z1 < 0)) ? -z1 : z1;
}

INTERP (Copysign_f64)
{
    const double z2 = pop_f64 ();
    double& z1 = f64 ();
    z1 = ((z2 < 0) != (z1 < 0)) ? -z1 : z1;
}

// Various lossless and lossy conversions.

INTERP (i32_Wrap_i64)
{
    set_i32 ((int)(i64 () & 0xFFFFFFFF));
}

INTERP (i32_Trunc_f32s)
{
    set_i32 ((int)f32 ());
}

INTERP (i32_Trunc_f32u)
{
    set_u32 ((uint)f32 ());
}

INTERP (i32_Trunc_f64s)
{
    set_i32 ((int)f64 ());
}

INTERP (i32_Trunc_f64u)
{
    set_u32 ((uint)f64 ());
}

INTERP (i64_Extend_i32s)
{
    set_i64 ((int64)i32 ());
}

INTERP (i64_Extend_i32u)
{
    set_u64 ((uint64)u32 ());
}

INTERP (i64_Trunc_f32s)
{
    set_i64 ((int64)f32 ());
}

INTERP (i64_Trunc_f32u)
{
    set_u64 ((uint64)f32 ());
}

INTERP (i64_Trunc_f64s)
{
    set_i64 ((int64)f64 ());
}

INTERP (i64_Trunc_f64u)
{
    set_u64 ((uint64)f64 ());
}

INTERP (f32_Convert_i32u)
{
    set_f32 ((float)(uint)u32 ());
}

INTERP (f32_Convert_i32s)
{
    set_f32 ((float)(int)i32 ());
}

INTERP (f32_Convert_i64u)
{
    set_f32 ((float)u64 ());
}

INTERP (f32_Convert_i64s)
{
    set_f32 ((float)i64 ());
}

INTERP (f32_Demote_f64)
{
    set_f32 ((float)f64 ());
}

INTERP (f64_Convert_i32s)
{
    set_f64 ((double)i32 ());
}

INTERP (f64_Convert_i32u)
{
    set_f64 ((double)u32 ());
}

INTERP (f64_Convert_i64s)
{
    set_f64 ((double)i64 ());
}

INTERP (f64_Convert_i64u)
{
    set_f64 ((double)u64 ());
}

INTERP (f64_Promote_f32)
{
    set_f64 ((double)f32 ());
}

// reinterpret; these could be more automated

INTERP (i32_Reinterpret_f32)
{
    tag (ValueType_f32) = ValueType_i32;
}

INTERP (f32_Reinterpret_i32)
{
    tag (ValueType_i32) = ValueType_f32;
}

INTERP (i64_Reinterpret_f64)
{
    tag (ValueType_f64) = ValueType_i64;
}

INTERP (f64_Reinterpret_i64)
{
    tag (ValueType_i64) = ValueType_f64;
}

};

using namespace w3; // TODO C or C++?

int
main (int argc, char** argv)
{
    printf ("3 %s\n", InstructionName (1));
    printf ("4 %s\n", InstructionName (0x44));
#if 0 // test code TODO move it elsewhere? Or under a switch.
    char buf [99] = { 0 };
    uint len;
#define Xd(x) printf ("%s %I64d\n", #x, x);
#define Xx(x) printf ("%s %I64x\n", #x, x);
#define Xs(x) len = x; buf [len] = 0; printf ("%s %s\n", #x, buf);
    Xd (UIntGetPrecision (0));
    Xd (UIntGetPrecision (1));
    Xd (UIntGetPrecision (0x2));
    Xd (UIntGetPrecision (0x2));
    Xd (UIntGetPrecision (0x7));
    Xd (UIntGetPrecision (0x8));
    Xd (IntGetPrecision (0));
    Xd (IntGetPrecision (1));
    Xd (IntGetPrecision (0x2));
    Xd (IntGetPrecision (0x2));
    Xd (IntGetPrecision (0x7));
    Xd (IntGetPrecision (0x8));
    Xd (IntGetPrecision (0));
    Xd (IntGetPrecision (-1));
    Xd (IntGetPrecision (-0x2));
    Xd (IntGetPrecision (-0x2));
    Xd (IntGetPrecision (-0x7));
    Xd (IntGetPrecision (-0x8));
    Xd (IntToDec_GetLength (0))
    Xd (IntToDec_GetLength (1))
    Xd (IntToDec_GetLength (2))
    Xd (IntToDec_GetLength (300))
    Xd (IntToDec_GetLength (-1))
    Xx (SignExtend (0xf, 0));
    Xx (SignExtend (0xf, 1));
    Xx (SignExtend (0xf, 2));
    Xx (SignExtend (0xf, 3));
    Xx (SignExtend (0xf, 4));
    Xx (SignExtend (0xf, 5));
    Xd (IntToHex_GetLength (0xffffffffa65304e4));
    Xd (IntToHex_GetLength (0xfffffffa65304e4));
    Xd (IntToHex_GetLength (-1));
    Xd (IntToHex_GetLength (-1ui64>>4));
    Xd (IntToHex_GetLength (0xf));
    Xd (IntToHex_GetLength (32767));
    Xd (IntToHex_GetLength (-32767));
    Xs (IntToHex (32767, buf));
    Xs (IntToHex (-32767, buf));
    Xs (IntToHex8 (0x123, buf));
    Xs (IntToHex8 (0xffffffffa65304e4, buf));
    Xs (IntToHex8 (-1, buf));
    Xs (IntToHex (0x1, buf));
    Xs (IntToHex (0x12, buf));
    Xs (IntToHex (0x123, buf));
    Xs (IntToHex (0x12345678, buf));
    Xs (IntToHex (-1, buf));
    Xd (IntToHex_GetLength (0x1));
    Xd (IntToHex_GetLength (0x12));
    Xd (IntToHex_GetLength (0x12345678));
    Xd (IntToHex_GetLength (0x01234567));
    Xd (IntToHex_GetLength (-1));
    Xd (IntToHex_GetLength (~0u >> 1));
    Xd (IntToHex_GetLength (~0u >> 2));
    Xd (IntToHex_GetLength (~0u >> 4));
    exit (0);
#endif
#if 1
    try
#endif
    {
        // FIXME command line parsing
        // FIXME verbosity
        Module module;
        module.read_module (argv [1]);

        // TODO read .name custom section

        Interp().interp (&module, 0);

        if (module.start)
            Interp().interp (&module, module.start);
        else if (module.main)
            Interp().interp (&module, module.main);
    }
#if 1
    catch (int er)
    {
        fprintf (stderr, "error 0x%08X\n", er);
    }
    catch (const std::string& er)
    {
        fprintf (stderr, "%s", er.c_str ());
    }
#endif
    return 0;
}
