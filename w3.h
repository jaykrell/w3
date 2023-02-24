// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#include "w3-1.h"

#include <assert.h>
#include <errno.h>
#include <memory.h>
#include <stack>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <string>
#include <vector>
#include "w3Tag.h"
#include "w3Label.h"

std::string StringFormat (PCSTR format, ...);
void ThrowString (const std::string& a);

#define _DARWIN_USE_64_BIT_INODE 1
//#define __DARWIN_ONLY_64_BIT_INO_T 1
// TODO cmake
// TODO big endian and packed I/O
//#define _LARGEFILE_SOURCE
//#define _LARGEFILE64_SOURCE

#if _MSC_VER
#include <intrin.h>
#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#ifndef _ISOC99_SOURCE
#define _ISOC99_SOURCE
#endif

#ifdef _MSC_VER
#include <malloc.h>
#define alloca _alloca
#endif

size_t string_vformat_length (PCSTR format, va_list va);
//void AssertFailedFormat (PCSTR condition, const std::string& extra);
void AssertFailed (PCSTR expr);

#ifdef _WIN64
#define FORMAT_SIZE "I64"
#define long_t __int64 // aka ptrdiff, aka Unix long but not Windows long
#else
#define FORMAT_SIZE "l"
#define long_t long // aka ptrdiff, aka Unix long but not Windows long
#endif

struct Code;
struct DecodedInstruction;
struct explicit_operator_bool;
struct Fd;
struct Frame; // work in progress
struct Function;
struct FunctionType;
struct Handle;
struct Label;
struct MemoryMappedFile;
struct Module;
struct ModuleInstance;
struct Runtime;
struct Section;
struct Stack;
struct StackValue;
struct Stream;
struct Variable; //local, global, temp

char TagChar(Tag t); //todo: short string?

uint32_t Unpack2 (const void* a);
uint32_t Unpack4 (const void* a);
uint64_t SignExtend (uint64_t value, uint32_t bits);
size_t ssize_magnitude (ssize_t i);
size_t int_magnitude (ssize_t i); //todo remove

uint32_t UIntGetPrecision (uint64_t a); // How many bits needed to represent.
uint32_t IntGetPrecision (int64_t a); // How many bits needed to represent.

uint32_t UIntToDec_GetLength (uint64_t b); // e.g. 99=>2
uint32_t IntToDec_GetLength (int64_t a); // e.g. -99=>3
uint32_t UIntToHex_GetLength (uint64_t b); // e.g. 255=>2

// If negative and first digit is <8, add one to induce leading 8-F
// so that sign extension of most significant bit will work.
// This might be a bad idea. TODO.
uint32_t IntToHex_GetLength (int64_t a);

uint32_t UIntToDec (uint64_t a, PCH buf); // returns length
uint32_t IntToDec (int64_t a, PCH buf); // returns length
void UIntToHexLength (uint64_t a, uint32_t len, PCH buf);
void IntToHexLength (int64_t a, uint32_t len, PCH buf);
uint32_t IntToHex (int64_t a, PCH buf); // returns length
uint32_t IntToHex8 (int64_t a, PCH buf); // always outputs 8 chars and returns 8
uint32_t IntToHex_AtLeast8 (int64_t a, PCH buf); // always outputs at least 8 chars
uint32_t UIntToHex_AtLeast8 (uint64_t a, PCH buf); // always outputs at least 8 chars
uint32_t UIntToHex_GetLength_AtLeast8 (uint64_t a); // always at least 8

//todo these all need module or thread or other context,
// like for error reporting maybe, and move cursor there possibly..
uint8_t read_byte (uint8_t** cursor, const uint8_t* end);
uint64_t read_varuint64 (uint8_t** cursor, const uint8_t* end);
uint32_t read_varuint32 (uint8_t** cursor, const uint8_t* end);
uint8_t read_varuint7 (uint8_t** cursor, const uint8_t* end);
int64_t read_varint64 (uint8_t** cursor, const uint8_t* end);
int32_t read_varint32 (uint8_t** cursor, const uint8_t* end);

//todo don't use C++ or at least not exceptions?
void ThrowInt (int i, PCSTR a = "");
void ThrowErrno (PCSTR a = "");

//todo commit to C++11? probably not
// C++98 workaround for what C++11 offers.
struct explicit_operator_bool
{
    typedef void (explicit_operator_bool::*T) () const;
#if 1
    void True () const { }
#else
    void True () const;
#endif
};

typedef void (explicit_operator_bool::*bool_type) () const;

#if _WIN32

void throw_Win32Error (int err, PCSTR a = "");
void throw_GetLastError (PCSTR a = "");

#endif

void DecodeFunction (Module* module, Code* code, uint8_t** cursor);

//todo: trim these?
#ifdef _WIN32

#define NOMINMAX 1
#include <io.h>
#include <windows.h>
#define DebugBreak __debugbreak

#else

typedef char* PCH;
typedef const char* PCSTR;
#define IsDebuggerPresent() (0)
#define __debugbreak() ((void)0)
#define DebugBreak() ((void)0)
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#endif

#ifdef _MSC_VER
#pragma warning(disable:4061) // enumerator in switch of enum is not explicitly handled 
#endif

PCH VarName(Variable* var);

void AssertFailedFormat (PCSTR condition, const std::string& extra);
void AssertFailed (PCSTR expr);

#define Assert(x)         ((x) || ( AssertFailed (#x), (int)0))
#define AssertFormat(x, extra) ((x) || (AssertFailedFormat (#x, StringFormat extra), 0))

typedef enum Immediate : uint8_t
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

enum InstructionEnum : int; //TODO: underlying type?

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


template <uint32_t N> struct uintLEn_to_native_exact;
template <uint32_t N> struct uintLEn_to_native_fast;

template <> struct uintLEn_to_native_exact<16> { typedef uint16_t T; };
template <> struct uintLEn_to_native_exact<32> { typedef uint32_t T; };
template <> struct uintLEn_to_native_exact<64> { typedef uint64_t T; };
template <> struct uintLEn_to_native_fast<16> { typedef uint32_t T; };
template <> struct uintLEn_to_native_fast<32> { typedef uint32_t T; };
template <> struct uintLEn_to_native_fast<64> { typedef uint64_t T; };

template <uint32_t N>
struct uintLEn // unsigned little endian integer, size n bits
{
    union
    {
        typename uintLEn_to_native_exact<N>::T native;
        unsigned char data [N / 8];
    };

    operator typename uintLEn_to_native_fast<N>::T ()
    {
#if BYTE_ORDER == LITTLE_ENDIAN
        return native;
#else
        typename uintLEn_to_native_fast<N>::T a = 0;
        for (uint32_t i = N / 8; i; )
            a = (a << 8) | data [--i];
        return a;
#endif
    }
    void operator= (uint32_t);
};

//typedef uintLEn<16> uintLE16;
typedef uintLEn<32> uintLE32;
//typedef uintLEn<64> uintLE64;

// integer | float, unary | binary | test | relation
#define IUNOP(b0, name, size)   INSTRUCTION (b0, 1, 0, name ## _i ## size, Imm_none, 1, 1, Tag_i ## size, Tag_none, Tag_none, Tag_i ## size)
#define FUNOP(b0, name, size)   INSTRUCTION (b0, 1, 0, name ## _f ## size, Imm_none, 1, 1, Tag_f ## size, Tag_none, Tag_none, Tag_f ## size)
#define IBINOP(b0, name, size)  INSTRUCTION (b0, 1, 0, name ## _i ## size, Imm_none, 2, 1, Tag_i ## size, Tag_i ## size, Tag_none, Tag_i ## size)
#define FBINOP(b0, name, size)  INSTRUCTION (b0, 1, 0, name ## _f ## size, Imm_none, 2, 1, Tag_f ## size, Tag_f ## size, Tag_none, Tag_f ## size)
#define ITESTOP(b0, name, size) INSTRUCTION (b0, 1, 0, name ## _i ## size, Imm_none, 1, 1, Tag_i ## size, Tag_none,     Tag_none, Tag_bool)
#define FRELOP(b0, name, size)  INSTRUCTION (b0, 1, 0, name ## _f ## size, Imm_none, 2, 1, Tag_f ## size, Tag_f ## size, Tag_none, Tag_bool)
#define IRELOP(b0, name, size, sign)  INSTRUCTION (b0, 1, 0, name ## _i ## size ## sign, Imm_none, 2, 1, Tag_i ## size, Tag_i ## size, Tag_none, Tag_bool)

// convert; TODO make ordering more sensible?
#define CVTOP(b0, name, to, from, sign) INSTRUCTION (b0, 1, 0, to ## _ ## name ## _ ## from ## sign, Imm_none, 1, 1, Tag_ ## from, Tag_none, Tag_none, Tag_ ## to)

#undef CONST
#define CONST(b0, type) INSTRUCTION (b0, 1, 0, type ## _Const, Imm_ ## type, 0, 1, Tag_none, Tag_none, Tag_none, Tag_ ## type)

#define LOAD(b0, to, from)  INSTRUCTION (b0, 1, 0, to ## _Load ## from,  Imm_memory, 0, 1, Tag_none,     Tag_none, Tag_none, Tag_ ## to)
#define STORE(b0, from, to) INSTRUCTION (b0, 1, 0, from ## _Store ## to, Imm_memory, 1, 0, Tag_ ## from, Tag_none, Tag_none, Tag_none)

#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) name,

#undef RESERVED
#define RESERVED(b0) INSTRUCTION (0x ## b0, 0, 0, Reserved ## b0, Imm_none, 0, 0, Tag_none, Tag_none, Tag_none, Tag_none)

#undef INSTRUCTION
