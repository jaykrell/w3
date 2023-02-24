#pragma once

#define _DARWIN_USE_64_BIT_INODE 1
//#define __DARWIN_ONLY_64_BIT_INO_T 1
// TODO cmake
// TODO big endian and packed I/O
//#define _LARGEFILE_SOURCE
//#define _LARGEFILE64_SOURCE

#ifdef _MSC_VER
#pragma warning (disable:4201) // nameless struct/union
#pragma warning (disable:4355) // this used in base member initializer list
#pragma warning (disable:4100) // unused parameter
#pragma warning (disable:4371) // layout change from previous compiler version
#pragma warning (disable:4505) // unused static function
#pragma warning (disable:4514) // unused function
#pragma warning (disable:4668) // #if not_defined is #if 0
#pragma warning (disable:4710) // function not inlined
#pragma warning (disable:4820) // padding
#pragma warning (push) // TODO
#pragma warning (disable:4571) // catch(...)
#pragma warning (disable:4626) // assignment implicitly deleted
#pragma warning (disable:4625) // copy constructor implicitly deleted
#pragma warning (disable:4668) // #if not_defined as #if 0
#pragma warning (disable:4774) // printf used without constant format
#pragma warning (disable:4820) // ucrt\malloc.h(45): warning C4820: '_heapinfo': '4' bytes padding added after data member '_heapinfo::_useflag'
#pragma warning (disable:5039) // exception handling and function pointers
#endif

#ifdef _MSC_VER
#include <intrin.h>
#endif

#if __GNUC__ || __clang__
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#ifndef _ISOC99_SOURCE
#define _ISOC99_SOURCE
#endif

#include <assert.h>
#include <errno.h>
#include <memory.h>
#include <stdarg.h>
#include <stddef.h>
typedef ptrdiff_t ssize_t;
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stack>
#include <string>
#include <vector>
#include <memory>

#ifdef _WIN32

#define NOMINMAX 1
#include <io.h>
#include <windows.h>
#define DebugBreak __debugbreak

#else

#define IsDebuggerPresent() (0)
#define __debugbreak() ((void)0)
#define DebugBreak() ((void)0)
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#endif

#ifdef _MSC_VER
#include <malloc.h> // for _alloca
#pragma warning (pop)
#endif

namespace w3
{

struct StackValueZeroInit; // work in progress
struct StackValue;
struct StackBase;
struct Stack;

struct Interp;
struct Frame;
struct Label;
struct Limits;
struct Module;
struct ModuleInstance;
struct Runtime;
struct Section;
struct TableType;

struct stream;
struct stdout_stream;
struct stderr_stream;

struct Handle; // e.g. Win32 CreateFile result, with destructor
struct Fd; // e.g. open result, with destructor
struct MemoryMappedFile; // e.g. mmap result, with destructor

void AssertFailed (const char* file, int line, const char* expr);
//#define Assert(expr) ((void)((expr) || AssertFailed (__FILE__, __LINE__, #expr)))
void AssertFailedFormat (const char* condition, const std::string& extra);
void AssertFailed (const char* expr);
#define Assert(x)         ((x) || ( AssertFailed (#x), (int)0))
#define AssertFormat(x, extra) ((x) || (AssertFailedFormat (#x, StringFormat extra), 0))

void ThrowString (const std::string& a);
std::string StringFormatVa (const char* format, va_list va);
std::string StringFormat (const char* format, ...);

void ThrowInt (int i, const char* a = "");
void ThrowErrno (const char* a = "");

#ifdef _WIN32
void throw_Win32Error (int err, const char* a = "");
void throw_GetLastError (const char* a = "");
#endif

#ifdef _WIN64
#define FORMAT_SIZE "I64"
#define long_t __int64 // aka ptrdiff, aka Unix long but not Windows long
#else
#define FORMAT_SIZE "l"
#define long_t long // aka ptrdiff, aka Unix long but not Windows long
#endif

size_t string_vformat_length (const char* format, va_list va);

struct FuncAddr;// TODO
struct TableAddr; // TODO
struct MemAddr; // TODO
struct GlobalAddr; // TODO

// TODO: Maybe use this for all interpreter values
// For now it is only for the newer SourceGen.
//
enum struct TypeTag : uint8_t
{
    none = 0, // zero-init
    Bool = 1, // i32
    any  = 2, // often has some constraints

    empty = 0x40, // defined by wasm in some contexts
    i32 = 0x7F, // defined by wasm
    i64 = 0x7E, // defined by wasm
    f32 = 0x7D, // defined by wasm
    f64 = 0x7C, // defined by wasm
    string = 0x80, // sourcegen extension
    label  = 0x81, // sourcegen extension, needed?
};

// This should probabably be combined with ResultType, and called Tag.
enum ValueType : uint8_t
{
    ValueType_i32 = 0x7F,
    ValueType_i64 = 0x7E,
    ValueType_f32 = 0x7D,
    ValueType_f64 = 0x7C,
};

std::string StringFormatVa (const char* format, va_list va);

const uint32_t PageSize = (1UL << 16);
const uint32_t PageShift = 16;

#define NotImplementedYed() (AssertFormat (0, ("not yet implemented %s 0x%08X ", __func__, __LINE__)))

uint32_t Unpack2 (const void* a);
uint32_t Unpack4 (const void* a);

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

// C++98 workaround for what C++11 offers.
struct explicit_operator_bool
{
    typedef void (explicit_operator_bool::*T) () const;
    void True () const;
};

typedef void (explicit_operator_bool::*bool_type) () const;

uint64_t SignExtend (uint64_t value, uint32_t bits);
size_t int_magnitude (ssize_t i);

struct int_split_sign_magnitude_t
{
    int_split_sign_magnitude_t (int64_t a)
    : neg ((a < 0) ? 1u : 0u),
        u ((a < 0) ? (1 + (uint64_t)-(a + 1)) // Avoid negating most negative number.
                  : (uint64_t)a) { }
    uint32_t neg;
    uint64_t u;
};

uint32_t UIntGetPrecision (uint64_t a);
uint32_t IntGetPrecision (int64_t a);
uint32_t UIntToDec_GetLength (uint64_t b);
uint32_t UIntToDec (uint64_t a, char* buf);
uint32_t IntToDec (int64_t a, char* buf);
uint32_t IntToDec_GetLength (int64_t a);
uint32_t UIntToHex_GetLength (uint64_t b);
uint32_t IntToHex_GetLength (int64_t a);
void UIntToHexLength (uint64_t a, uint32_t len, char* buf);
void IntToHexLength (int64_t a, uint32_t len, char* buf);
uint32_t IntToHex (int64_t a, char* buf);
uint32_t IntToHex8 (int64_t a, char* buf);
uint32_t IntToHex_GetLength_AtLeast8 (int64_t a);
uint32_t UIntToHex_GetLength_AtLeast8 (uint64_t a);
uint32_t IntToHex_AtLeast8 (int64_t a, char* buf);
uint32_t UIntToHex_AtLeast8 (uint64_t a, char* buf);

uint8_t read_byte (uint8_t** cursor, const uint8_t* end);
uint64_t read_varuint64 (uint8_t** cursor, const uint8_t* end);
uint32_t read_varuint32 (uint8_t** cursor, const uint8_t* end);
uint8_t read_varuint7 (uint8_t** cursor, const uint8_t* end);
int64_t read_varint64 (uint8_t** cursor, const uint8_t* end);
int32_t read_varint32 (uint8_t** cursor, const uint8_t* end);

enum Type : uint8_t
{
    Type_none,
    Type_bool, // i32
    Type_any, // often has some constraints
    Type_i32 = 0x7F,
    Type_i64 = 0x7E,
    Type_f32 = 0x7D,
    Type_f64 = 0x7C,
};

union Value
{
    int32_t i32;
    uint32_t u32;
    uint64_t u64;
    int64_t i64;
    float f32;
    double f64;
};

struct TaggedValue
{
    ValueType tag;
    Value value;
};

// This should probabably be combined with ValueType, and called Tag.
enum ResultType : uint8_t
{
    ResultType_i32 = 0x7F,
    ResultType_i64 = 0x7E,
    ResultType_f32 = 0x7D,
    ResultType_f64 = 0x7C,
    ResultType_empty = 0x40
};

typedef ResultType BlockType; // TODO? remove separate name

const char* TypeToStringCxx (int tag);
const char* TypeToString (int tag);

enum TableElementType : uint32_t
{
    TableElementType_funcRef = 0x70,
};

enum LimitsTag // specific to tabletype?
{
    LimitsTag_min = 0,
    Limits_minMax = 1,
};

const uint32_t FunctionTypeTag = 0x60;
const uint32_t TableTypeFuncRef = 0x70; // Table types have an value type, funcref

// Globals are mutable or constant.
enum Mutable
{
    Mutable_constant = 0, // aka false
    Mutable_variable = 1, // aka true
};

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

struct FunctionType;
struct Function;
struct Code;
struct Frame; // work in progress
struct DecodedInstruction;

enum StackTag : uint8_t
{
    StackTag_Value = 1, // i32, i64, f32, f64
    StackTag_Label,     // branch target
    StackTag_Frame,     // return address + locals + params
};

const char* StackTagToString (StackTag tag);

enum Immediate : uint8_t
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
};

enum InstructionEnum : uint16_t;
struct InstructionNames;

#define BITS_FOR_UINT_HELPER(a, x) (a) >= (1u << x) ? (x) + 1 :
#define BITS_FOR_UINT(a)                                                                                \
  (BITS_FOR_UINT_HELPER (a, 31) BITS_FOR_UINT_HELPER (a, 30)                                                          \
   BITS_FOR_UINT_HELPER (a, 29) BITS_FOR_UINT_HELPER (a, 28) BITS_FOR_UINT_HELPER (a, 27) BITS_FOR_UINT_HELPER (a, 26) BITS_FOR_UINT_HELPER (a, 25) \
   BITS_FOR_UINT_HELPER (a, 24) BITS_FOR_UINT_HELPER (a, 23) BITS_FOR_UINT_HELPER (a, 22) BITS_FOR_UINT_HELPER (a, 21) BITS_FOR_UINT_HELPER (a, 20) \
   BITS_FOR_UINT_HELPER (a, 19) BITS_FOR_UINT_HELPER (a, 18) BITS_FOR_UINT_HELPER (a, 17) BITS_FOR_UINT_HELPER (a, 16) BITS_FOR_UINT_HELPER (a, 15) \
   BITS_FOR_UINT_HELPER (a, 14) BITS_FOR_UINT_HELPER (a, 13) BITS_FOR_UINT_HELPER (a, 12) BITS_FOR_UINT_HELPER (a, 11) BITS_FOR_UINT_HELPER (a, 10) \
   BITS_FOR_UINT_HELPER (a,  9) BITS_FOR_UINT_HELPER (a,  8) BITS_FOR_UINT_HELPER (a,  7) BITS_FOR_UINT_HELPER (a,  6) BITS_FOR_UINT_HELPER (a,  5) \
   BITS_FOR_UINT_HELPER (a,  4) BITS_FOR_UINT_HELPER (a,  3) BITS_FOR_UINT_HELPER (a,  2) BITS_FOR_UINT_HELPER (a,  1) BITS_FOR_UINT_HELPER (a,  0) 1)

#if (_MSC_VER && _MSC_VER <= 1700) || __WATCOMC__

#define bits_for_uint(x) BITS_FOR_UINT (x)

#else

constexpr int bits_for_uint (uint32_t a)
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

#if defined (_WIN32) && defined (C_ASSERT) // older compiler
#define static_assert(x, y) C_ASSERT (x)
#endif

static_assert (BITS_FOR_UINT (0) == 1, "0");
static_assert (BITS_FOR_UINT (1) == 1, "1");
static_assert (BITS_FOR_UINT (2) == 2, "2");
static_assert (BITS_FOR_UINT (3) == 2, "3");
static_assert (BITS_FOR_UINT (4) == 3, "4");
static_assert (BITS_FOR_UINT (10) == 4, "");
static_assert (BITS_FOR_UINT (30) == 5, "");
static_assert (BITS_FOR_UINT (200) == 8, "");

static_assert (bits_for_uint (0) == 1, "0");
static_assert (bits_for_uint (1) == 1, "1");
static_assert (bits_for_uint (2) == 2, "2");
static_assert (bits_for_uint (3) == 2, "3");
static_assert (bits_for_uint (4) == 3, "4");
static_assert (bits_for_uint (10) == 4, "");
static_assert (bits_for_uint (30) == 5, "");
static_assert (bits_for_uint (200) == 8, "");

//static_assert (BITS_FOR_UINT (sizeof (instructionNames)) == 12, "");
//static_assert (bits_for_uint (sizeof (instructionNames)) == 12, "");

#define InstructionName(i) (&instructionNames.data [instructionEncode [i].string_offset])

struct InstructionEncoding;
struct DecodedInstructionZeroInit;
struct DecodedInstruction;
struct WasmString;
struct Section;
struct ModuleBase; // workaround old compiler (?)
struct SectionTraits;

extern const InstructionEncoding instructionEncode[];

enum BuiltinString
{
    BuiltinString_none = 0,
    BuiltinString_main,
    BuiltinString_start,
};

enum ImportTag // aka desc
{
    ImportTag_Function = 0, // aka type
    ImportTag_Table = 1,
    ImportTag_Memory = 2,
    ImportTag_Global = 3,
};
typedef ImportTag ExportTag;
#define ExportTag_Function ImportTag_Function
#define ExportTag_Table ImportTag_Table
#define ExportTag_Memory ImportTag_Memory
#define ExportTag_Global ImportTag_Global
struct MemoryType;
struct ImportFunction;
struct ImportTable;
struct ImportMemory;
struct GlobalType;
struct Import;
struct ExternalValue; // external to a module, an export instance
struct ExportInstance; // work in progress
struct ModuleInstance; // work in progress
struct FunctionInstance; // work in progress
struct Function; // section3
struct Global;
struct Element;
struct Export;
struct Data; // section11
struct Code; // The code to a function.
             // Functions are split between section3 and section10.
             // Instructions are in section10.
             // Function code is decoded upon first (or only) visit.

// Initial representation of X and XSection are the same.
// This might evolve, i.e. into separate TypesSection and Types,
// or just Types that is not Section.
struct FunctionType;

struct Module;

InstructionEnum
DecodeInstructions (Module* module, std::vector <DecodedInstruction>& instructions, uint8_t** cursor, Code* code);

InstructionEnum
DecodeInstructions (Module* module, std::vector <DecodedInstruction>& instructions, uint8_t** cursor, Code* code);

void DecodeFunction (Module* module, Code* code, uint8_t** cursor);
extern const SectionTraits section_traits[];
struct Wasm;
void Overflow (void);
struct Interp;
struct SourceGen;
struct RustGen;
struct CGen;

}
