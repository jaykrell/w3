// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#include "w3.h"
#include "w3Fd.h"
#include "w3Handle.h"
#include "w3MemoryMappedFile.h"
#include "w3Module.h"

#include <stack>
#include <string>
#include <vector>
#include <memory>

#include "ieee.h"
#include "math_private.h"
static const float wasm_hugef = 1.0e30F;
static const double wasm_huged = 1.0e300;
#include "s_floor.c"
#include "s_floorf.c"
#include "isnan.c"
#include "isinf.c"
#include "s_trunc.c"
#include "s_truncf.c"
#include "s_round.c"
#include "s_roundf.c"

void ThrowString (const std::string& a)
{
    //fprintf (stderr, "%s\n", a.c_str ());
    throw a + "\n";
    //abort ();
}

#if 0
void AssertFailed (PCSTR file, int line, PCSTR expr)
{
    fprintf (stderr, "Assert failed: %s(%d):%s\n", file, line, expr);
#ifdef _WIN32
    DebugBreak();
#else
    abort ();
#endif
}

#define Assert(expr) ((void)((expr) || AssertFailed (__FILE__, __LINE__, #expr)))
#else


void AssertFailedFormat (PCSTR condition, const std::string& extra)
{
    fputs (("AssertFailed:" + std::string (condition) + ":" + extra + "\n").c_str (), stderr);
    //Assert (0);
    //abort ();
#ifdef _WIN32 // TODO
    if (IsDebuggerPresent ()) __debugbreak ();
#endif
    ThrowString ("AssertFailed:" + std::string (condition) + ":" + extra);
}

void AssertFailed (PCSTR expr)
{
    fprintf (stderr, "AssertFailed:%s\n", expr);
#ifdef _WIN32 // TODO
    if (IsDebuggerPresent ()) __debugbreak ();
#endif
    assert (0);
    abort ();
}

#endif

std::string StringFormatVa (PCSTR format, va_list va);

std::string StringFormat (PCSTR format, ...)
{
    va_list va;
    va_start (va, format);
    std::string a = StringFormatVa (format, va);
    va_end (va);
    return a;
}

void ThrowInt (int i, PCSTR a)
{
    ThrowString (StringFormat ("error 0x%08X %s", i, a));
}

void ThrowErrno (PCSTR a)
{
    ThrowInt (errno, a);
}

#ifdef _WIN32

void throw_Win32Error (int err, PCSTR a)
{
    ThrowInt (err, a);

}

void throw_GetLastError (PCSTR a)
{
    ThrowInt ((int)GetLastError (), a);

}

#endif

#ifdef _MSC_VER
#pragma warning (disable:4777) // printf maybe wrong for other platforms
#pragma warning (push)
#pragma warning (disable:4996) // _vsnprintf dangerous
#endif

size_t string_vformat_length (PCSTR format, va_list va)
{
#ifdef _MSC_VER
    return 2 + _vscprintf (format, va);
#else
    return 2 + vsnprintf (0, 0, format, va);
#endif
}

#if _MSC_VER >= 1200
#pragma warning (pop)
#endif

template <class T> const T& Min (const T& a, const T& b)
{
    return (a <= b) ? a : b;
}

template <class T> const T& Max (const T& a, const T& b)
{
    return (a >= b) ? a : b;
}

template <class T> void WasmStdConstructN (T* a, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        new (a++) T ();
}

template <class T> void WasmStdCopyConstructNtoN (T* to, T* from, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        new (to++) T (*from++);
}

template <class T> void WasmStdCopyConstruct1toN (T* to, const T& from, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        new (to++) T (from);
}

template <class T> void WasmStdCopyConstruct1 (T& to, const T& from)
{
    WasmStdCopyConstruct1toN (&to, from, 1u);
}

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

char TagChar(Tag t) //todo: short string?
{
    switch (t)
    {
    case Tag_none: return 'n';
    case Tag_bool: return 'b';
    case Tag_any: return 'a';
    //case Tag_i8: return 'c';
    //case Tag_u8: return 'd';
    //case Tag_i16: return 's';
    //case Tag_u16: return 't';
    case Tag_i32: return 'i';
    case Tag_i64: return 'j';
    case Tag_f32: return 'f';
    case Tag_f64: return 'g';
    case Tag_empty: return 'e';
    case Tag_Value: return 'v';
    case Tag_Label: return 'L';
    case Tag_Frame: return 'r';
    case Tag_FuncRef: return 'u';
    }
    return '$';
}

// TODO templatize on existance of string and possibly label
// so that SourceGen and Interp can share.
struct SourceGenValue
{
    Tag tag;
    std::string str;
    PCSTR cstr() { return str.c_str(); }
};

struct SourceGenStack : std::stack<SourceGenValue>
{
    typedef std::stack<SourceGenValue> base;

    void clear()
    {
        while (size()) pop();
    }

    PCSTR cstr() { return top ().str.c_str (); }

    std::string pop ()
    {
        std::string str = top ().str;
        base::pop();
        return str;
    }

    void push_label (...) { } //todo
};

std::string StringFormatVa (PCSTR format, va_list va)
{
    // Some systems, including Linux/amd64, cannot consume a
    // va_list multiple times. It must be copied first.
    // Passing the parameter twice does not work.
#ifndef _WIN32
    va_list va2;
#ifdef __va_copy
    __va_copy (va2, va);
#else
    va_copy (va2, va); // C99
#endif
#endif

    std::vector <char> s (string_vformat_length (format, va));

#ifdef _WIN32
    _vsnprintf (&s [0], s.size (), format, va);
#else
    vsnprintf (&s [0], s.size (), format, va2);
#endif
    return &s [0];
}

//const uint32_t PageSize = (1UL << 16);
const uint32_t PageShift = 16;

#define NotImplementedYed() (AssertFormat (0, ("not yet implemented %s 0x%08X ", __func__, __LINE__)))

uint32_t Unpack2 (const void* a)
{
    uint8_t* b = (uint8_t*)a;
    return ((b [1]) << 8) | (uint32_t)b [0];
}

uint32_t Unpack4 (const void* a)
{
    return (Unpack2 ((PCH)a + 2) << 16) | Unpack2 (a);
}

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

size_t int_magnitude (ssize_t i)
{
    // Avoid negating the most negative number.
    return 1 + (size_t)-(i + 1);
}

struct int_split_sign_magnitude_t
{
    int_split_sign_magnitude_t (int64_t a)
    : neg ((a < 0) ? 1u : 0u),
        u ((a < 0) ? (1 + (uint64_t)-(a + 1)) // Avoid negating most negative number.
                  : (uint64_t)a) { }
    uint32_t neg;
    uint64_t u;
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
    return Min (64u, 1 + UIntGetPrecision (int_split_sign_magnitude_t (a).u));
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
    const int_split_sign_magnitude_t split (a);
    if (split.neg)
        *buf++ = '-';
    return split.neg + UIntToDec (split.u, buf);
}

uint32_t IntToDec_GetLength (int64_t a)
{
    const int_split_sign_magnitude_t split (a);
    return split.neg + UIntToDec_GetLength (split.u);
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
    return Max (len, 8u);
}

uint32_t UIntToHex_GetLength_AtLeast8 (uint64_t a)
{
    uint32_t const len = UIntToHex_GetLength (a);
    return Max (len, 8u);
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

struct Stream
{
    virtual ~Stream() { }

    virtual void write (const void* bytes, size_t size) = 0;

    void prints (PCSTR a) { write (a, strlen (a)); }

    void prints (const std::string& a) { write (a.c_str (), a.length ()); }

    void printc (char a) { write (&a, 1); }

    void printf (PCSTR format, ...)
    {
        va_list va;
        va_start (va, format);
        printv (format, va);
        va_end (va);
    }

    void printv (PCSTR format, va_list va)
    {
        prints (StringFormatVa (format, va));
    }
};

struct stdout_stream : Stream
{
    virtual void write (const void* bytes, size_t size)
    {
        fflush (stdout);
        PCSTR pc = (PCSTR)bytes;
        while (size > 0)
        {
            // TODO: Use smaller number for Win32 console?
            uint32_t const n = (uint32_t)Min (size, (((size_t)1) << 30));
#ifdef _WIN32
            ::_write (_fileno (stdout), pc, n);
#else
            ::write (fileno (stdout), pc, n);
#endif
            size -= n;
            pc += n;
        }
    }
};

struct stderr_stream : Stream
{
    // TODO: Refactor with stdout.
    virtual void write (const void* bytes, size_t size)
    {
        fflush (stderr);
        PCSTR pc = (PCSTR)bytes;
        while (size > 0)
        {
            uint32_t const n = (uint32_t)Min (size, (((size_t)1) << 30));
#ifdef _WIN32
            ::_write (_fileno (stderr), pc, n);
#else
            ::write (fileno (stderr), pc, n);
#endif
            size -= n;
            pc += n;
        }
    }
};

uint8_t read_byte (uint8_t** cursor, const uint8_t* end)
{
    if (*cursor >= end)
        ThrowString (StringFormat ("malformed %d", __LINE__)); // UNDONE context (move to module or section)
    return *(*cursor)++;
}

uint64_t read_varuint64 (uint8_t** cursor, const uint8_t* end)
{
    uint64_t result = 0;
    uint32_t shift = 0;
    while (true)
    {
        const uint32_t byte = read_byte (cursor, end);
        result |= (byte & 0x7F) << shift;
        if ((byte & 0x80) == 0)
            return result;
        shift += 7;
    }
}

uint32_t read_varuint32 (uint8_t** cursor, const uint8_t* end)
{
    uint32_t result = 0;
    uint32_t shift = 0;
    while (true)
    {
        const uint32_t byte = read_byte (cursor, end);
        result |= (byte & 0x7F) << shift;
        if ((byte & 0x80) == 0)
            return result;
        shift += 7;
    }
}

uint8_t read_varuint7 (uint8_t** cursor, const uint8_t* end)
{
    const uint8_t result = read_byte (cursor, end);
    if (result & 0x80)
        ThrowString (StringFormat ("malformed %d", __LINE__)); // UNDONE context (move to module or section)
    return result;
}

int64_t read_varint64 (uint8_t** cursor, const uint8_t* end)
{
    int64_t result = 0;
    uint32_t shift = 0;
    uint32_t size = 64;
    uint32_t byte = 0;
    do
    {
        byte = read_byte (cursor, end);
        result |= (byte & 0x7F) << shift;
        shift += 7;
    } while (byte & 0x80);

    // sign bit of byte is second high order bit (0x40)
    if ((shift < size) && (byte & 0x40))
        result |= (~0 << shift); // sign extend

    return result;
}

int32_t read_varint32 (uint8_t** cursor, const uint8_t* end)
{
    int32_t result = 0;
    uint32_t shift = 0;
    uint32_t size = 32;
    uint32_t byte = 0;
    do
    {
        byte = read_byte (cursor, end);
        result |= (byte & 0x7F) << shift;
        shift += 7;
    } while (byte & 0x80);

    // sign bit of byte is second high order bit (0x40)
    if ((shift < size) && (byte & 0x40))
        result |= (~0 << shift); // sign extend

    return result;
}

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
    Tag tag;
    Value value;

    TaggedValue()
    {
        memset (this, 0, sizeof(*this));
    }
};

PCSTR TagToStringCxx (int tag)
{
    switch ((Tag)tag)
    {
    default:break;
    case Tag_none:     return "void";
    case Tag_bool:     return "bool";
    case Tag_any:      return "void*"; // union?
    case Tag_i32:      return "int";
    case Tag_i64:      return "int64_t";
    case Tag_f32:      return "float";
    case Tag_f64:      return "double";
    case Tag_empty:    return "void";
    case Tag_Value:    return "Value";
    case Tag_Label:    return "Label";
    case Tag_Frame:    return "Frame";
    }
    return "unknown";
}

PCSTR TagToString (int tag)
{
    switch ((Tag)tag)
    {
    default:break;
    case Tag_none:     return "none(0)";
    case Tag_bool:     return "bool(1)";
    case Tag_any:      return "any(2)";
    case Tag_i32:      return "i32(7F)";
    case Tag_i64:      return "i64(7E)";
    case Tag_f32:      return "f32(7D)";
    case Tag_f64:      return "f64(7C)";
    case Tag_empty:    return "empty(40)";

    case Tag_Value:    return "Value(1)";
    case Tag_Label:    return "Label(2)";
    case Tag_Frame:    return "Frame(3)";
    }
    return "unknown";
}

//const uint32_t FunctionTypeTag = 0x60;

// Globals are mutable or constant.
typedef enum Mutable
{
    Mutable_constant = 0, // aka false
    Mutable_variable = 1, // aka true
} Mutable;

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

struct String
{
    PCH buf;
    size_t len;
};

#if 0

struct Variable
{    Tag tag;
    bool temp : 1;
    bool global : 1;
    bool local : 1;
    long id; // in lieue of name
    PCH name;
    char namebuf[64];
};

PCH VarName(Variable* var)
{
//    if (var->name)
//        return var->name;
//    sprintf("
    return 0;
}

#endif

struct StackValue
{
    // TODO remove two level tagging
    Tag tag;
    //union {
        TaggedValue value; // TODO: change to Value or otherwise remove redundant tag
        Frame* frame; // TODO by value? Probably not. This was changed
                      // to resolve circular types, and for the initial frame that seemed
                      // wrong, but now that call/ret being implemented, seems right
        //DecodedInstruction* instr;
        Label label;
    //};

    StackValue() : tag (Tag_none), frame (0)
    {
    }

    StackValue (Tag t) : tag (t), frame (0)
    {
        if ((t & 0x78) == 0x78)
        {
            tag = Tag_Value;
            value.tag = t; //todo: remove?
        }
    }

    StackValue (TaggedValue t) : tag (Tag_Value), frame (0)
    {
        value = t;
    }

    StackValue (Frame* f) : tag (Tag_Frame), frame (0)
    {
 //     frame = f;
    }
};

// TODO consider a vector instead, but it affects frame.locals staying valid across push/pop
//typedef std::deque <StackValue> StackBaseBase;
typedef std::vector <StackValue> StackBaseBase;

struct StackBase : private StackBaseBase
{
    typedef StackBaseBase base;
    typedef base::iterator iterator;
    StackValue& back () { return base::back (); }
    StackValue& front () { return base::front (); }
    iterator begin () { return base::begin (); }
    iterator end () { return base::end (); }
    bool empty () const { return base::empty (); }
    void resize (size_t newsize) { base::resize (newsize); }
    size_t size () { return base::size (); }
    StackValue& operator [ ] (size_t index) { return base::operator [ ] (index); }

    void push (const StackValue& a)
    {   // While ultimately a stack of values, labels, and frames, values dominate,
        // so the usage is made convenient for them.
        push_back (a);
    }

    void push_i32(...) // todo
    {
    }

    void push_i64(...) // todo
    {
    }

    void push_f32(...) // todo
    {
    }

    void push_f64(...) // todo
    {
    }

    void pop ()
    {
        pop_back ();
    }

    StackValue& top ()
    {   // While ultimately a stack of values, labels, and frames, values dominate,
        // so the usage is made convenient for them.
        return back ();
    }
};

struct Interp;

struct Frame
{
    // FUTURE spec return_arity
    size_t function_index; // replace with pointer?
    ModuleInstance* module_instance;
    Module* module;
//    Frame* next; // TODO remove this; it is on stack
    Code* code;
    size_t param_count;
    size_t local_only_count;
    size_t param_and_local_count;
    Tag* local_only_types;
    Tag* param_types;
    FunctionType* function_type;
    // TODO locals/params
    // This should just be stack pointer, to another stack,
    // along with type information (module->module->locals_types[])

    Interp* interp;
    size_t locals; // index in stack to start of params and locals, params first

    Frame ()
    {
        ZeroMem(this, sizeof(*this));
    }

    StackValue& Local (size_t index);
};

// work in progress
struct Stack : private StackBase
{
    Stack () { }

    // old compilers lack using.
    typedef StackBase base;
    typedef base::iterator iterator;
    void pop () { base::pop (); }
    StackValue& top () { return base::top (); }
    StackValue& back () { return base::back (); }
    StackValue& front () { return base::front (); }
    iterator begin () { return base::begin (); }
    iterator end () { return base::end (); }
    bool empty () const { return base::empty (); }
    void resize (size_t newsize) { base::resize (newsize); }
    size_t size () { return base::size (); }
    StackValue& operator [ ] (size_t index) { return base::operator [ ] (index); }

    void reserve (size_t n)
    {
        // TODO
    }

    // While ultimately a stack of values, labels, and frames, values dominate.

    Tag& tag (Tag tag)
    {
        AssertTopIsValue ();
        StackValue& t = top ();
        AssertFormat (t.value.tag == tag, ("%X %X", t.value.tag, tag));
        return t.value.tag;
    }

    Tag& tag ()
    {
        AssertTopIsValue ();
        StackValue& t = top ();
        return t.value.tag;
    }

    Value& value ()
    {
        AssertTopIsValue ();
        StackValue& t = top ();
        return t.value.value;
    }

    Value& value (Tag tag)
    {
        AssertTopIsValue ();
        StackValue& t = top ();
        AssertFormat (t.value.tag == tag, ("%X %X", t.value.tag, tag));
        return t.value.value;
    }

    void pop_label ()
    {
        if (size () < 1 || top ().tag != Tag_Label)
            DumpStack ("AssertTopIsValue");
        AssertFormat (size () >= 1, ("%" FORMAT_SIZE "X", size ()));
        AssertFormat (top ().tag == Tag_Label, ("%X %X", top ().tag, Tag_Label));
        pop ();
    }

    void pop_value ()
    {
        AssertTopIsValue ();
        //int t = tag ();
        pop ();
        //printf ("pop_value tag:%s depth:%" FORMAT_SIZE "X\n", TagToString (t), size ());
    }

    void push_value (const StackValue& value)
    {
        AssertFormat (value.tag == Tag_Value, ("%X %X", value.tag, Tag_Value));
        push (value);
        //printf ("push_value tag:%s value:%X depth:%" FORMAT_SIZE "X\n", TagToString (value.value.tag), value.value.value.i32, size ());
    }

    void push_label (const StackValue& value)
    {
        AssertFormat (value.tag == Tag_Label, ("%X %X", value.tag, Tag_Label));
        push (value);
        //printf ("push_label depth:%" FORMAT_SIZE "X\n", size ());
    }

    void push_frame (const StackValue& value)
    {
        AssertFormat (value.tag == Tag_Frame, ("%X %X", value.tag, Tag_Frame));
        push (value);
        //printf ("push_frame depth:%" FORMAT_SIZE "X\n", size ());
    }

    // type specific pushers

    void push_i32 (int32_t i)
    {
        StackValue value (Tag_i32);
        value.value.value.i32 = i;
        push_value (value);
    }

    void push_i64 (int64_t i)
    {
        StackValue value (Tag_i64);
        value.value.value.i64 = i;
        push_value (value);
    }

    void push_u32 (uint32_t i)
    {
        push_i32 ((int32_t)i);
    }

    void push_u64 (uint64_t i)
    {
        push_i64 ((int64_t)i);
    }

    void push_f32 (float i)
    {
        StackValue value (Tag_f32);
        value.value.value.f32 = i;
        push_value (value);
    }

    void push_f64 (double i)
    {
        StackValue value (Tag_f64);
        value.value.value.f64 = i;
        push_value (value);
    }

    void push_bool (bool b)
    {
        push_i32 (b);
    }

    // accessors, check tag, return ref

    int32_t& i32 ()
    {
        return value (Tag_i32).i32;
    }

    int64_t& i64 ()
    {
        return value (Tag_i64).i64;
    }

    uint32_t& u32 ()
    {
        return value (Tag_i32).u32;
    }

    uint64_t& u64 ()
    {
        return value (Tag_i64).u64;
    }

    float& f32 ()
    {
        return value (Tag_f32).f32;
    }

    double& f64 ()
    {
        return value (Tag_f64).f64;
    }

    void DumpStack (PCSTR prefix)
    {
        const size_t n = size ();
        printf ("stack@%s: %" FORMAT_SIZE "X ", prefix, n);
        for (size_t i = 0; i != n; ++i)
        {
            printf ("%s:", TagToString (begin () [(ssize_t)i].tag));
            switch (begin () [(ssize_t)i].tag)
            {
            case Tag_Label:
                break;
            case Tag_Frame:
                break;
            case Tag_Value:
                printf ("%s", TagToString (begin () [(ssize_t)i].value.tag));
                break;
            default:; //todo
            }
            printf (" ");
        }
        printf ("\n");
    }

    void AssertTopIsValue ()
    {
        if (size () < 1 || top ().tag != Tag_Value)
            DumpStack ("AssertTopIsValue");
        AssertFormat (size () >= 1, ("%" FORMAT_SIZE "X", size ()));
        AssertFormat (top ().tag == Tag_Value, ("%X %X", top ().tag, Tag_Value));
    }

    // setter, changes tag, returns ref

    Value& set (Tag tag)
    {
        AssertTopIsValue ();
        StackValue& t = top ();
        TaggedValue& v = t.value;
        v.tag = tag;
        return v.value;
    }

    // type-specific setters

    void set_i32 (int32_t a)
    {
        set (Tag_i32).i32 = a;
    }

    void set_u32 (uint32_t a)
    {
        set (Tag_i32).u32 = a;
    }

    void set_bool (bool a)
    {
        set_i32 (a);
    }

    void set_i64 (int64_t a)
    {
        set (Tag_i64).i64 = a;
    }

    void set_u64 (uint64_t a)
    {
        set (Tag_i64).u64 = a;
    }

    void set_f32 (float a)
    {
        set (Tag_f32).f32 = a;
    }

    void set_f64 (double a)
    {
        set (Tag_f64).f64 = a;
    }

    // type specific poppers

    int32_t pop_i32 ()
    {
        int32_t a = i32 ();
        pop_value ();
        return a;
    }

    uint32_t pop_u32 ()
    {
        uint32_t a = u32 ();
        pop_value ();
        return a;
    }

    int64_t pop_i64 ()
    {
        int64_t a = i64 ();
        pop_value ();
        return a;
    }

    uint64_t pop_u64 ()
    {
        uint64_t a = u64 ();
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

#define RESERVED(b0) INSTRUCTION (0x ## b0, 0, 0, Reserved ## b0, Imm_none, 0, 0, Tag_none, Tag_none, Tag_none, Tag_none)

#undef CONST
#define CONST(b0, type) INSTRUCTION (b0, 1, 0, type ## _Const, Imm_ ## type, 0, 1, Tag_none, Tag_none, Tag_none, Tag_ ## type)

#define LOAD(b0, to, from)  INSTRUCTION (b0, 1, 0, to ## _Load ## from,  Imm_memory, 0, 1, Tag_none,     Tag_none, Tag_none, Tag_ ## to)
#define STORE(b0, from, to) INSTRUCTION (b0, 1, 0, from ## _Store ## to, Imm_memory, 1, 0, Tag_ ## from, Tag_none, Tag_none, Tag_none)

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

#define ExportTag_Function ImportTag_Function
#define ExportTag_Table ImportTag_Table
#define ExportTag_Memory ImportTag_Memory
#define ExportTag_Global ImportTag_Global

//struct ImportFunction;
//struct ImportTable;
//struct ImportMemory;

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
    WasmString name;
    ExternalValue external_value;
};

struct ModuleInstance // work in progress
{
    ModuleInstance (Module* mod);
    ModuleInstance () : module(0) { }

    Module* module;
    std::vector <uint8_t> memory;
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

struct SectionTraits
{
    void (Module::*read)(uint8_t** cursor);
    PCSTR name;
};

extern const SectionTraits section_traits [ ] =
{
    { 0 },
#define SECTIONS        \
    SECTION (TypesSection, read_types)          \
    SECTION (ImportsSection, read_imports)      \
    SECTION (FunctionsSection, read_functions)  \
    SECTION (TablesSection, read_tables)        \
    SECTION (MemorySection, read_memory)        \
    SECTION (GlobalsSection, read_globals)      \
    SECTION (ExportsSection, read_exports)      \
    SECTION (StartSection, read_start)          \
    SECTION (ElementsSection, read_elements)    \
    SECTION (CodeSection, read_code)            \
    SECTION (DataSection, read_data)            \

#undef SECTION
#define SECTION(x, read) {&Module::read, #x},
SECTIONS

};

// TODO once we have Validate, Interp, Jit, CppGen,
// we might invert this structure and have a class per instruction with those 4 virtual functions.
// Or we will token-paste those names on to the instruction names,
// in order to avoid virtual function call cost. Let's get Interp working first.
struct Wasm
{
    DecodedInstruction* instr; // TODO make local variable

    Wasm () : instr (0) { }

    virtual ~Wasm () { }

    virtual void Reserved () = 0;
#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) void name () { abort (); }
#include "w3instructions.h"
};

void Overflow (void)
{
    Assert (!"Overflow");
}

struct Interp : Stack, Wasm
{
private:
    Interp(const Interp&);
    void operator =(const Interp&);
public:
    Interp() : module (0), module_instance (0), frame (0), stack (*this)
    {
    }

    void* LoadStore (size_t size);

    Module* module;                     // TODO multiple modules
    ModuleInstance* module_instance;    // TODO multiple modules
    Frame* frame;

    // FIXME multiple modules

    Stack& stack;

    void Invoke (Function&);

    void interp (Module* mod, Export* emain = 0)
    {
        Assert (mod && emain && emain->tag == ExportTag_Function);
        Assert (emain->function < mod->functions.size ());
        Assert (emain->function < mod->code.size ());
        Assert (mod->functions.size () == mod->code.size ());

        Function& fmain = mod->functions [emain->function];
        //Code& cmain = mod->code [emain->function];

        // instantiate module
        this->module = mod;
        ModuleInstance instance (module);
        this->module_instance = &instance;

        // Simulate call to initial function.
        Invoke (fmain);
    }

    void Reserved ();

#undef RESERVED
#define RESERVED(b0) INSTRUCTION (0x ## b0, 0, 0, Reserved ## b0, Imm_none, 0, 0, Tag_none, Tag_none, Tag_none, Tag_none) { Reserved (); }

#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) ; void name ()
#include "w3instructions.h"
    ;
};

struct SourceGenFunction
{
    //std::vector<Variable> locals;
    //std::vector<Variable> temps;

    //void reset()
    //{
    //    locals.clear();
    //    temps.clear();
    //}
};

struct SourceGen : Wasm
{
    SourceGen() : temp(0), function_type(0)
    {
    }

    long temp;
    //std::vector<Variable> globals;

    SourceGenStack stack; // TODO? std::stack<std::string>
    std::stack<Label> labels;
    FunctionType* function_type;

    // The value stack is the central data structure so assume it.

    PCSTR cstr() { return stack.top().cstr(); }

    void push_i32 (int i)
    {
        char s[99];
        sprintf(s, "%d", i); // TODO C vs. Rust TODO perf
        SourceGenValue sourceGenValue = {Tag_i32, s};
        stack.push(sourceGenValue);
    }

    void push_i32 (PCSTR s)
    {
        SourceGenValue sourceGenValue = {Tag_i32, s};
        stack.push(sourceGenValue);
    }

    void push_i64 (...) //todo
    {
    }

    void push_f32(...) // todo
    {
    }

    void push_f64(...) // todo
    {
    }

    std::string pop ()
    {
        return stack.pop();
    }

    /*std::string top ()
    {
        std::string a = pop ();
        push (a);
        return a;
    }*/

    static std::string string_format(PCSTR, ...)
    {
        return "todo";
    }

    void push(const std::string& s)
    {
        SourceGenValue value;
        value.str = s;
        stack.push(value);
    }

    SourceGenValue& top() { return stack.top(); }
};

struct RustGen : SourceGen //TODO
{
};

struct WasmCGen : SourceGen
{
private:
    WasmCGen(const WasmCGen&);
    void operator=(const WasmCGen&);
public:

    void Prefix();

    void flush() { }
    void print(...) { }

    virtual ~WasmCGen()
    {
    }

    WasmCGen() : module (0)
    {
    }

    void Load (PCSTR stack_type, PCSTR mem_type);
    void Store (PCSTR stack_type, PCSTR mem_type);

    void LoadStore (PCSTR stack_type, PCSTR mem_type, bool loadOrStore);

    Module* module;

    void interp (Module* mod, Export* emain = 0)
    {
        // instantiate module
        this->module = mod;
        ModuleInstance instance (module);
        //this->module_instance = &instance;

        // Simulate call to initial function.

        Prefix ();

        size_t function_count = mod->functions.size ();
        for (size_t function_index = 0; function_index < function_count; ++function_index)
        {
            // TODO Rust not C.

            printf("/*function%d*/\n", (int)function_index);

            Function& function = mod->functions[function_index];
            const size_t function_type_index = function.function_type_index;

            printf("/*function_type%d*/\n", (int)function_type_index);

            Assert (function_type_index < module->function_types.size ());
            function_type = &module->function_types [function_type_index];

            Code* code = &module->code [function.function_index];
            const size_t param_count = function_type->parameters.size ();

            uint8_t* cursor = code->cursor;
            if (cursor)
            {
                printf("\n#if 0\n");
                DecodeFunction (module, code, &cursor);
                printf("\n#endif\n");
                code->cursor = 0;
            }

            if (function_type->results.size() == 0)
                printf("\nvoid ");
            else
                printf("\n%s ", TagToStringCxx(function_type->results[0]));

            printf(" function%d ( \n", (int)function_index);

            // args are the first locals
            // join them here and elsewhere does not care

            if (param_count == 0)
                printf("void");
            else
            {
                for (size_t j = 0; j < param_count; ++j)
                {
                    if (j)
                        printf(",");
                    printf("%s local%" FORMAT_SIZE "d", TagToStringCxx(function_type->parameters[j]), (long_t)j);
                }
            }

            printf(") {");
            // Declare locals, merged with parameters.
            const size_t local_count = code->locals.size();
            for (size_t k = 0; k < local_count; ++k)
            {
                printf("%s local%" FORMAT_SIZE "d;\n", TagToStringCxx(code->locals[k]), (long_t)k + param_count);
            }

            this->instr = &code->decoded_instructions [0];
            const size_t function_size = code->decoded_instructions.size ();
            DecodedInstruction* end = instr + function_size;
            for (; this->instr < end; ++instr)
            {
                Assert (this->instr);
                switch (this->instr->name)
                {
                    // break before instead of after to avoid unreachable code warning
#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, pop, push, in0, in1, in2, out0)                            \
                break;                                                                                              \
                case ::name:                                                                                      \
                printf ("/*gen%s x:%X u:%u i:%i*/\n", #name, this->instr->u32, this->instr->u32, this->instr->u32); \
                this->name ();
#include "w3instructions.h"
                }
            }
            while (stack.size() > 1)
                printf("%s;\n", pop ().c_str());
            if (function_type->results.size())
                printf("return ");
            while (stack.size())
                printf("%s;\n", pop ().c_str());
            printf("\n}\n");
        }
    }

    void Reserved ()
    {
    }

#undef RESERVED
#define RESERVED(b0) void Reserved ## b0 () { Reserved (); }

#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) void name ();
#include "w3instructions.h"
};

#include "w3cgen.cpp"

StackValue& Frame::Local (size_t index)
{
    return interp->stack [locals + index];
}

//if
//else
//brtable

void* Interp::LoadStore (size_t size)
{
    // TODO Not clear from spec and paper what to do here, despite
    // focused discussion on it.
    // Why is the operand signed??
    size_t effective_address = 0;
    const int32_t i = pop_i32 ();
    const size_t offset = instr->offset;
    if (i >= 0)
    {
        effective_address = offset + (uint32_t)i;
        if (effective_address < offset)
            Overflow ();
    }
    else
    {
        const size_t u = int_magnitude (i);
        if (u > offset)
            Overflow ();
        effective_address = offset - u;
    }
    if (effective_address > UINT_MAX - size)
        Overflow ();
    AssertFormat (effective_address + size <= frame->module_instance->memory.size (),
        ("%" FORMAT_SIZE "X %" FORMAT_SIZE "X %" FORMAT_SIZE "X",
        (long_t)effective_address, (long_t)size, (long_t)frame->module_instance->memory.size ()));
    return &frame->module_instance->memory [effective_address];
}

#undef INTERP
#define INTERP(x) void Interp::x ()

void Interp::Invoke (Function& function)
{
    //__debugbreak ();
    // Decode function upon first call.
    // TODO thread safety
    // TODO merge with calli (invoke)

    const size_t function_type_index = function.function_type_index;
    Assert (function_type_index < module->function_types.size ());
    FunctionType* function_type = &module->function_types [function_type_index];
    Code* code = &module->code [function.function_index];
    const size_t local_only_count = code->locals.size ();
    const size_t param_count = function_type->parameters.size ();

    uint8_t* cursor = code->cursor;
    if (cursor)
    {
        DecodeFunction (module, code, &cursor);
        code->cursor = 0;
    }
    const size_t size = code->decoded_instructions.size ();
    Assert (size);

    DumpStack ("invoke1");

    // TODO cross-module calls
    // TODO calling embedding
    // setup frame
    Frame frame_value;
    frame_value.interp = this;
    frame_value.code = code;
    frame_value.module = this->module; // TODO cross module calls
    frame_value.module_instance = this->module_instance; // TODO cross module calls
    frame_value.function_index = function.function_index;
    frame_value.param_count = param_count;
    frame_value.param_and_local_count = local_only_count + param_count; // TODO overflow
    frame_value.local_only_count = local_only_count;
    frame_value.local_only_types = local_only_count ? &code->locals [0] : 0;
    frame_value.param_types = param_count ? &function_type->parameters [0] : 0;
    frame_value.function_type = function_type;

    StackValue ret (frame);
    this->frame = &frame_value;
    push_frame (ret);
    DumpStack ("pushed_frame");

    size_t i = 0;
    size_t j = 0;

    for (j = 0; j != param_count; ++j)
    {
        printf ("2 entering function with param [%" FORMAT_SIZE "X] type %X\n", j, (end () - (ssize_t)param_count + (ssize_t)j)->value.tag);
    }

    // CONSIDER put the interp loop elsewhere
    // Invoke would adjust member data and return to it

    // params are subsumed into new frame

    // TODO We really want this to be more efficient.
    // Either by having split stacks, or by computing
    // maximum parameter count and putting return before it.

    // For now live with memcpy or slower (stack is not presently contiguous)..

    if (param_count)
    {
        for (i = 0; i < param_count; ++i)
        {
            DumpStack ("moved_param_before");
            *(end () - 1 - (ssize_t)i) = *(end () - 2 - (ssize_t)i);
            DumpStack ("moved_param_after");
        }
    }

    // place return frame/address (frame is just a marker now, the data is on the native stack)

    (end () - 1 - (ssize_t)param_count)->tag = Tag_Frame;
    //(end () - 1 - param_count)->frame = frame;
    //(end () - 1 - param_count)->instr = instr + !!instr;

    // Push locals on stack.
    // TODO params also
    // TODO reserve (size () + local_only_count);
    DumpStack ("push_locals_before");
    for (i = 0; i != local_only_count; ++i)
        push_value (StackValue (code->locals [i]));

    DumpStack ("push_locals_after");
    // Provide for indexing locals.
    frame_value.locals = stack.size () - local_only_count - param_count;

    for (j = 0; j != local_only_count + param_count; ++j)
        printf ("2 entering function with local [%" FORMAT_SIZE "X] type %X\n", j, frame_value.Local (j).value.tag);

    //DumpStack ("invoke2");

    // TODO provide for separate depth -- i.e. here is now 0; locals/params cannot be popped

    DecodedInstruction* previous = instr; // call/ret handling
    instr = &code->decoded_instructions [0];
    DecodedInstruction* end = instr + size;
    for (; instr < end; ++instr) // Br subtracts one so this works.
    {
        Assert (instr);
        switch (instr->name)
        {
            // break before instead of after to avoid unreachable code warning
#undef RESERVED
#define RESERVED(b0) INSTRUCTION (0x ## b0, 0, 0, Reserved ## b0, Imm_none, 0, 0, Tag_none, Tag_none, Tag_none, Tag_none) { Reserved (); }

#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, pop, push, in0, in1, in2, out0)     \
            break;                                                                           \
            case ::name:                                                                   \
            printf ("interp%s x:%X u:%u i:%i\n", #name, instr->u32, instr->u32, instr->u32); \
            this->name ();
#include "w3instructions.h"
        }
        // special handling
        if (instr->name == ::Ret) // gross but most choices are
            break;
#include "diag-switch-push.h"
        switch (instr->name)
        {
        case ::Call:
        case ::Calli:
            frame = &frame_value; // TODO should handled in Ret.
            break;
        }
#include "diag-switch-pop.h"
    }
    instr = previous;
    // TODO handle ret
    //__debugbreak ();
}

#include "w3interp.cpp" //todo separate compile

int
main (int argc, PCH* argv)
{
    if (IsDebuggerPresent ()) DebugBreak ();
#if 0 // test code TODO move it elsewhere? Or under a switch.
    printf ("3 %s\n", InstructionName (1));
    printf ("4 %s\n", InstructionName (0x44));
    char buf [99] = { 0 };
    uint32_t len;
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

        // Support --run-all-exports for wabt test suite.

        size_t file = 1;
        size_t i = 0;
        bool run_all_exports = 0;
        bool rust_gen = 0;
        bool cgen = 0;

        Assert(argc >= 0);
        for (i = 1 ; i < (uint32_t)argc; ++i)
        {
            if (strcmp (argv [i], "--cgen") == 0)
            {
                cgen = true;
                if (i == 1)
                    file = 2;
                else if (i == 2)
                    file = 1;
            }
            if (strcmp (argv [i], "--rust-gen") == 0)
            {
                rust_gen = true;
                if (i == 1)
                    file = 2;
                else if (i == 2)
                    file = 1;
            }
            else if (strcmp (argv [i], "--run-all-exports") == 0)
            {
                run_all_exports = true;
                if (i == 1)
                    file = 2;
                else if (i == 2)
                    file = 1;
            }
        }

        module.read_module (argv [file]);

        if (run_all_exports)
        {
            const size_t j = module.exports.size ();
            for (i = 0; i < j; ++i)
            {
                Export* const xport = &module.exports [i];
                if (xport->tag != ExportTag_Function)
                    continue;
                Interp().interp (&module, xport);
            }
        }

        if (cgen || rust_gen)
        {
            WasmCGen().interp (&module);
        }
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
