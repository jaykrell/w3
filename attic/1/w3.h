// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#include "w3-1.h"
typedef ptrdiff_t ssize_t;
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

uint32_t GetUint16LE (const void* a);
uint32_t GetUint32LE (const void* a);
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

void throw_Win32Error (int err, PCSTR a);
void throw_GetLastError (PCSTR a);

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

#include "w3Immediate.h"
#include "w3InstructionMacros.h"
#include "w3InstructionEnum.h"
#include "w3BitsForInt.h"

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
