// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#define _CRT_SECURE_NO_WARNINGS 1

#include <assert.h>
#include <algorithm>
#include <vector>
#include <string>
#include <stdio.h>
using std::min;
using std::max;
using std::vector;
using std::string;
#include "w3.h"
#include "w3-2.h"

extern const float wasm_hugef = 1.0e30F;
extern const double wasm_huged = 1.0e300;

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

#if _WIN32

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

#if _MSC_VER
#include <malloc.h> // for _alloca
#endif

namespace w3
{

#if 0
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

void
ThrowString (const std::string& a)
{
    //fprintf (stderr, "%s\n", a.c_str ());
    throw a + "\n";
    //abort ();
}

std::string
StringFormatVa (const char* format, va_list va);

std::string
StringFormat (const char* format, ...)
{
    va_list va;
    va_start (va, format);
    std::string a = StringFormatVa (format, va);
    va_end (va);
    return a;
}

void
ThrowInt (int i, const char* a)
{
    ThrowString (StringFormat ("error 0x%08X %s", i, a));
}

void
ThrowErrno (const char* a)
{
    ThrowInt (errno, a);
}

#if _WIN32

void
throw_Win32Error (int err, const char* a)
{
    ThrowInt (err, a);

}

void
throw_GetLastError (const char* a)
{
    ThrowInt ((int)GetLastError (), a);

}

#endif

#ifdef _WIN64
#define FORMAT_SIZE "I64"
#define long_t __int64 // aka ptrdiff
#else
#define FORMAT_SIZE "l"
#define long_t long // aka ptrdiff
#endif

#if _MSC_VER
#pragma warning (disable:4777) // printf maybe wrong for other platforms
#pragma warning (push)
#pragma warning (disable:4996) // _vsnprintf dangerous
#endif

size_t
string_vformat_length (const char* format, va_list va)
{
#if _MSC_VER
    return 2 + _vscprintf (format, va);
#else
    return 2 + vsnprintf (0, 0, format, va);
#endif
}

#if _MSC_VER >= 1200
#pragma warning (pop)
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

//template <class T> void WasmStdConstructN (T* a, size_t n) { for (size_t i = 0; i < n; ++i) new (a++) T (); }
//template <class T> void WasmStdCopyConstructNtoN (T* to, T* from, size_t n) { for (size_t i = 0; i < n; ++i) new (to++) T (*from++); }
//template <class T> void WasmStdCopyConstruct1toN (T* to, const T& from, size_t n) { for (size_t i = 0; i < n; ++i) new (to++) T (from); }
//template <class T> void WasmStdCopyConstruct1 (T& to, const T& from) { WasmStdCopyConstruct1toN (&to, from, 1u); }

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

// This should probabably be combined with ResultType, and called Tag.
enum ValueType : uint8_t
{
    Tag_i32 = 0x7F,
    Tag_i64 = 0x7E,
    Tag_f32 = 0x7D,
    Tag_f64 = 0x7C,
};

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

    std::vector <char> s (string_vformat_length (format, va));

#if _WIN32
    _vsnprintf (&s [0], s.size (), format, va);
#else
    vsnprintf (&s [0], s.size (), format, va2);
#endif
    return &s [0];
}

const uint32_t PageShift = 16;

#define NotImplementedYed() (AssertFormat (0, ("not yet implemented %s 0x%08X ", __func__, __LINE__)))

uint32_t
GetUint16LE (const void* a)
{
    uint8_t* b = (uint8_t*)a;
    return ((b [1]) << 8) | (uint32_t)b [0];
}

uint32_t
GetUint32LE (const void* a)
{
    return (GetUint16LE ((char*)a + 2) << 16) | GetUint16LE (a);
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

    uint64_t get_file_size (const char* file_name = "")
    {
        DWORD hi = 0;
        DWORD lo = GetFileSize (h, &hi);
        if (lo == INVALID_FILE_SIZE)
        {
            DWORD err = GetLastError ();
            if (err != NO_ERROR)
                throw_Win32Error ((int)err, StringFormat ("GetFileSize (%s)", file_name).c_str ());
        }
        return (((uint64_t)hi) << 32) | lo;
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

#if !_WIN32
    uint64_t get_file_size (const char* file_name = "")
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

    Fd& operator = (int a)
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
#ifndef FILE_SHARE_DELETE // ifndef required due to Watcom 4 vs. 4L.
#define FILE_SHARE_DELETE               0x00000004 // missing in older headers
#endif
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

struct stream
{
    virtual ~stream() { }

    virtual void write (const void* bytes, size_t size) = 0;

    void prints (const char* a) { write (a, strlen (a)); }

    void prints (const std::string& a) { write (a.c_str (), a.length ()); }

    void printc (char a) { write (&a, 1); }

    void printf (const char* format, ...)
    {
        va_list va;
        va_start (va, format);
        printv (format, va);
        va_end (va);
    }

    void printv (const char* format, va_list va)
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
            // TODO: Use smaller number for Win32 console?
            uint32_t const n = (uint32_t)std::min (size, (((size_t)1) << 30));
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

struct stderr_stream : stream
{
    // TODO: Refactor with stdout.
    virtual void write (const void* bytes, size_t size)
    {
        fflush (stderr);
        const char* pc = (const char*)bytes;
        while (size > 0)
        {
            uint32_t const n = (uint32_t)std::min (size, (((size_t)1) << 30));
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

uint8_t
read_byte (uint8_t** cursor, const uint8_t* end)
{
    if (*cursor >= end)
        ThrowString (StringFormat ("malformed %d", __LINE__)); // UNDONE context (move to module or section)
    return *(*cursor)++;
}

uint64_t
read_varuint64 (uint8_t** cursor, const uint8_t* end)
{
    uint64_t result = 0;
    uint32_t shift = 0;
    while (true)
    {
        const uint32_t byte = read_byte (cursor, end);
		printf ("read_varuint64_2:%X\n", byte);
        result |= (byte & 0x7F) << shift;
        if ((byte & 0x80) == 0)
            return result;
        shift += 7;
    }
}

uint32_t
read_varuint32 (uint8_t** cursor, const uint8_t* end)
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

uint8_t
read_varuint7 (uint8_t** cursor, const uint8_t* end)
{
    const uint8_t result = read_byte (cursor, end);
    if (result & 0x80)
        ThrowString (StringFormat ("malformed %d", __LINE__)); // UNDONE context (move to module or section)
    return result;
}

int64_t
read_varint64 (uint8_t** cursor, const uint8_t* end)
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

int32_t
read_varint32 (uint8_t** cursor, const uint8_t* end)
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

typedef enum Type : uint8_t
{
    Type_none,
    Type_bool, // i32
    Type_any, // often has some constraints
    Type_i32 = 0x7F,
    Type_i64 = 0x7E,
    Type_f32 = 0x7D,
    Type_f64 = 0x7C,
} Type;

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
    union
    {
        int32_t i32;
        uint32_t u32;
        uint64_t u64;
        int64_t i64;
        float f32;
        double f64;
        Value value;
    };
};

const char*
TypeToStringCxx (int tag)
{
    switch (tag)
    {
    case Type_none:     return "void";
    case Type_bool:     return "bool";
    case Type_any:      return "void*"; // union?
    case ResultType_i32: return "int";
    case ResultType_i64: return "int64_t";
    case ResultType_f32: return "float";
    case ResultType_f64: return "double";
    case ResultType_empty: return "void";
    }
    return "unknown";
}

const char*
TypeToString (int tag)
{
    switch (tag)
    {
    case Type_none:     return "none(0)";
    case Type_bool:     return "bool(1)";
    case Type_any:      return "any(2)";
    case ResultType_i32: return "i32(7F)";
    case ResultType_i64: return "i64(7E)";
    case ResultType_f32: return "f32(7D)";
    case ResultType_f64: return "f64(7C)";
    case ResultType_empty: return "empty(40)";
    }
    return "unknown";
}

typedef enum TableElementType : uint32_t
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
    // TODO size_t? null?
    Limits () : min (0), max (0), hasMax (false) { }

    uint32_t min;
    uint32_t max;
    bool hasMax;
};

const uint32_t FunctionTypeTag = 0x60;

struct TableType
{
    TableType () : elementType ((TableElementType)0) { }

    TableElementType elementType;
    Limits limits;

    bool operator < (const TableType&) const; // workaround old compiler
    bool operator == (const TableType&) const; // workaround old compiler
    bool operator != (const TableType&) const; // workaround old compiler
};

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

typedef struct FunctionType FunctionType;
typedef struct Function Function;
typedef struct Code Code;
typedef struct Frame Frame; // work in progress
typedef struct DecodedInstruction DecodedInstruction;

typedef enum StackTag
{
    StackTag_Value = 1, // i32, i64, f32, f64
    StackTag_Label,     // branch target
    StackTag_Frame,     // return address + locals + params
} StackTag;

const char*
StackTagToString (StackTag tag)
{
    switch (tag)
    {
    case StackTag_Value: return "Value(1)";
    case StackTag_Label: return "Label(2)";
    case StackTag_Frame: return "Frame(3)";
    }
    return "unknown";
}

typedef struct LabelValue
{
    size_t arity, continuation;
} LabelValue;


struct StackValue : StackValueZeroInit
{
    void Init ();

    StackValue()
    {
        Init ();
    }

    StackValue (StackTag t)
    {
        Init ();
        tag = t;
    }

    StackValue (ValueType t)
    {
        Init ();
        tag = StackTag_Value;
        value.tag = t;
    }

    StackValue (TaggedValue t)
    {
        Init ();
        tag = StackTag_Value;
        value = t;
    }

    StackValue (Frame* f)
    {
        Init ();
        tag = StackTag_Frame;
 //     frame = f;
    }

    bool operator < (const StackValue&) const; // workaround old compiler
    bool operator == (const StackValue&) const; // workaround old compiler
    bool operator != (const StackValue&) const; // workaround old compiler
};

// TODO consider a vector instead, but it affects frame.locals staying valid across push/pop
//typedef std::deque <StackValue> StackBaseBase;
typedef std::vector <StackValue> StackBaseBase;

struct StackBase : private StackBaseBase
{
    typedef StackBaseBase base;
    typedef StackBaseBase::iterator iterator;
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
    size_t function_index {}; // replace with pointer?
    ModuleInstance* module_instance {};
    Module* module {};
//    Frame* next; // TODO remove this; it is on stack
    Code* code {};
    size_t param_count {};
    size_t local_only_count {};
    size_t param_and_local_count {};
    ValueType* local_only_types {};
    ValueType* param_types {};
    FunctionType* function_type {};
    // TODO locals/params
    // This should just be stack pointer, to another stack,
    // along with type information (module->module->locals_types[])

    Interp* interp {};
    size_t locals {}; // index in stack to start of params and locals, params first

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

    ValueType& tag (ValueType tag)
    {
        AssertTopIsValue ();
        StackValue& t = top ();
        AssertFormat (t.value.tag == tag, ("%X %X", t.value.tag, tag));
        return t.value.tag;
    }

    ValueType& tag ()
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

    Value& value (ValueType tag)
    {
        AssertTopIsValue ();
        StackValue& t = top ();
        AssertFormat (t.value.tag == tag, ("%X %X", t.value.tag, tag));
        return t.value.value;
    }

    void pop_label ()
    {
        if (size () < 1 || top ().tag != StackTag_Label)
            DumpStack ("AssertTopIsValue");
        AssertFormat (size () >= 1, ("%" FORMAT_SIZE "X", size ()));
        AssertFormat (top ().tag == StackTag_Label, ("%X %X", top ().tag, StackTag_Label));
        pop ();
    }

    void pop_value ()
    {
        AssertTopIsValue ();
        //int t = tag ();
        pop ();
        //printf ("pop_value tag:%s depth:%" FORMAT_SIZE "X\n", TypeToString (t), size ());
    }

    void push_value (const StackValue& value)
    {
        AssertFormat (value.tag == StackTag_Value, ("%X %X", value.tag, StackTag_Value));
        push (value);
        //printf ("push_value tag:%s value:%X depth:%" FORMAT_SIZE "X\n", TypeToString (value.value.tag), value.value.value.i32, size ());
    }

    void push_label (const StackValue& value)
    {
        AssertFormat (value.tag == StackTag_Label, ("%X %X", value.tag, StackTag_Label));
        push (value);
        //printf ("push_label depth:%" FORMAT_SIZE "X\n", size ());
    }

    void push_frame (const StackValue& value)
    {
        AssertFormat (value.tag == StackTag_Frame, ("%X %X", value.tag, StackTag_Frame));
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

    void DumpStack (const char* prefix)
    {
        const size_t n = size ();
        printf ("stack@%s: %" FORMAT_SIZE "X ", prefix, n);
        for (size_t i = 0; i != n; ++i)
        {
            printf ("%s:", StackTagToString (begin () [(ssize_t)i].tag));
            switch (begin () [(ssize_t)i].tag)
            {
            case StackTag_Label:
                break;
            case StackTag_Frame:
                break;
            case StackTag_Value:
                printf ("%s", TypeToString (begin () [(ssize_t)i].value.tag));
                break;
            }
            printf (" ");
        }
        printf ("\n");
    }

    void AssertTopIsValue ()
    {
        if (size () < 1 || top ().tag != StackTag_Value)
            DumpStack ("AssertTopIsValue");
        AssertFormat (size () >= 1, ("%" FORMAT_SIZE "X", size ()));
        AssertFormat (top ().tag == StackTag_Value, ("%X %X", top ().tag, StackTag_Value));
    }

    // setter, changes tag, returns ref

    Value& set (ValueType tag)
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

#define LOAD(b0, to, from)  INSTRUCTION (b0, 1, 0, to ## _Load ## from,  Imm_memory, 0, 1, Type_none,     Type_none, Type_none, Type_ ## to)
#define STORE(b0, from, to) INSTRUCTION (b0, 1, 0, from ## _Store ## to, Imm_memory, 1, 0, Type_ ## from, Type_none, Type_none, Type_none)

#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) name,

typedef enum InstructionEnum
{
#include __FILE__
} InstructionEnum;

#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) char name [ sizeof (#name) ];
typedef struct InstructionNames
{
#include __FILE__
} InstructionNames;

#if 0 // Split string up for old compiler.
const char instructionNames [ ] =
#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) #name "\0"
#include __FILE__
;
#else

union {
    struct {
#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) char name [sizeof (#name)];
#include __FILE__
    } x;
    char data [1];
} instructionNames = { {
#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) #name,
#include __FILE__
} };

#endif

#define BITS_FOR_UINT_HELPER(a, x) (a) >= (1u << x) ? (x) + 1 :
#define BITS_FOR_UINT(a)                                                                                \
  (BITS_FOR_UINT_HELPER (a, 31) BITS_FOR_UINT_HELPER (a, 30)                                                          \
   BITS_FOR_UINT_HELPER (a, 29) BITS_FOR_UINT_HELPER (a, 28) BITS_FOR_UINT_HELPER (a, 27) BITS_FOR_UINT_HELPER (a, 26) BITS_FOR_UINT_HELPER (a, 25) \
   BITS_FOR_UINT_HELPER (a, 24) BITS_FOR_UINT_HELPER (a, 23) BITS_FOR_UINT_HELPER (a, 22) BITS_FOR_UINT_HELPER (a, 21) BITS_FOR_UINT_HELPER (a, 20) \
   BITS_FOR_UINT_HELPER (a, 19) BITS_FOR_UINT_HELPER (a, 18) BITS_FOR_UINT_HELPER (a, 17) BITS_FOR_UINT_HELPER (a, 16) BITS_FOR_UINT_HELPER (a, 15) \
   BITS_FOR_UINT_HELPER (a, 14) BITS_FOR_UINT_HELPER (a, 13) BITS_FOR_UINT_HELPER (a, 12) BITS_FOR_UINT_HELPER (a, 11) BITS_FOR_UINT_HELPER (a, 10) \
   BITS_FOR_UINT_HELPER (a,  9) BITS_FOR_UINT_HELPER (a,  8) BITS_FOR_UINT_HELPER (a,  7) BITS_FOR_UINT_HELPER (a,  6) BITS_FOR_UINT_HELPER (a,  5) \
   BITS_FOR_UINT_HELPER (a,  4) BITS_FOR_UINT_HELPER (a,  3) BITS_FOR_UINT_HELPER (a,  2) BITS_FOR_UINT_HELPER (a,  1) BITS_FOR_UINT_HELPER (a,  0) 1)

#if (_MSC_VER && _MSC_VER <= 1700) || defined(__WATCOMC__)

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

#include "w3StdStaticAssert.h"

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

#define InstructionName(i) (&instructionNames.data [instructionEncode [i].string_offset])

struct InstructionEncoding
{
    uint8_t byte0;
    //uint8_t byte1;              // FIXME always 0 if fixed_size > 1
    uint8_t fixed_size    : 2;    // 0, 1, 2
    Immediate immediate;
    uint8_t pop           : 2;    // required minimum stack in
    uint8_t push          : 1;
    InstructionEnum name;
    uint32_t string_offset : bits_for_uint (sizeof (instructionNames));
    Type stack_in0  ; // type of stack [0] upon input, if pop >= 1
    Type stack_in1  ; // type of stack [1] upon input, if pop >= 2
    Type stack_in2  ; // type of stack [2] upon input, if pop == 3
    Type stack_out0 ; // type of stack [1] upon input, if push == 1
    void (*interp) (Module*); // Module* wrong
};

struct DecodedInstructionZeroInit // ZeroMem-compatible part
{
    union
    {
        uint32_t u32;
        uint64_t u64;
        int32_t i32;
        int64_t i64;
        float f32;
        double f64;
        struct // memory
        {
            uint32_t align;
            uint32_t offset;
        };
        struct // block / loop
        {
            size_t label;
        };

        struct // if / else
        {
            size_t if_false;
            size_t if_end;
        };
        // etc.
    };

    uint64_t file_offset; // to match up with disasm output, unsigned for hex
    InstructionEnum name;
    Tag blockType;
};

struct DecodedInstruction : DecodedInstructionZeroInit
{
    DecodedInstruction () : DecodedInstructionZeroInit {}
    {
    }

    size_t Arity() const
    {
        return (blockType == ResultType_empty) ? 0u : 1u; // FUTURE
    }

    std::vector <uint32_t> vecLabel;
};

#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, pop, push, in0, in1, in2, out0) { byte0, fixed_size, imm, pop, push, name, offsetof (InstructionNames, name), in0, in1, in2, out0 },
const InstructionEncoding instructionEncode [ ] = {
#include __FILE__
};

static_assert (sizeof (instructionEncode) / sizeof (instructionEncode [0]) == 256, "not 256 instructions");

typedef enum BuiltinString {
    BuiltinString_none = 0,
    BuiltinString_main,
    BuiltinString_start,
} BuiltinString;

struct WasmString
{
    WasmString() :
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
    uint32_t id;
    WasmString name;
    size_t payload_size;
    uint8_t* payload;
};

struct ModuleBase // workaround old compiler (?)
{
    virtual ~ModuleBase() { }
    virtual void read_types (uint8_t** cursor) = 0;
    virtual void read_imports (uint8_t** cursor) = 0;
    virtual void read_functions (uint8_t** cursor) = 0;
    virtual void read_tables (uint8_t** cursor) = 0;
    virtual void read_memory (uint8_t** cursor) = 0;
    virtual void read_globals (uint8_t** cursor) = 0;
    virtual void read_exports (uint8_t** cursor) = 0;
    virtual void read_start (uint8_t** cursor) = 0;
    virtual void read_elements (uint8_t** cursor) = 0;
    virtual void read_code (uint8_t** cursor) = 0;
    virtual void read_data (uint8_t** cursor) = 0;
};

struct SectionTraits
{
    void (ModuleBase::*read)(uint8_t** cursor);
    const char* name;
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
    ValueType value_type {};
    bool is_mutable {};
};

struct Import
{
    Import() : tag ((ImportTag)-1) { }

    WasmString module;
    WasmString name;
    ImportTag tag;
    // TODO virtual functions to model union
    //union
    //{
        TableType table;
        uint32_t function;
        MemoryType memory;
        GlobalType global;
    //};
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
    WasmString name;
    ExternalValue external_value;
};

struct ModuleInstance // work in progress
{
    ModuleInstance (Module* mod);

    Module* module;
    std::vector <uint8_t> memory;
    std::vector <FuncAddr*> funcs;
    std::vector <TableAddr*> tables;
    //std::vector <MemAddr*> mem; // mem [0] => memory for now
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
    // Functions are split between two sections: types in section3, locals/body in section10
    size_t function_index {}; // TODO needed?
    size_t function_type_index {};
    size_t local_only_count {};
    size_t param_count {};
    bool import {}; // TODO needed?
};

struct Global
{
    GlobalType global_type;
    std::vector <DecodedInstruction> init;
};

struct Element
{
    uint32_t table;
    std::vector <DecodedInstruction> offset_instructions;
    uint32_t offset;
    std::vector <uint32_t> functions;
};

struct Export
{
    Export ()
    {
    }

    Export (const Export& e)
    {
        printf ("copy export %X %X %X %X\n", tag, is_main, is_start, table);
        *this = e;
    }

    //void operator = (const Export& e);

    ExportTag tag {};
    WasmString name {};
    bool is_start {};
    bool is_main {};
    union
    {
        uint32_t function {};
        uint32_t memory;
        uint32_t table;
        uint32_t global;
    };
};

struct Data // section11
{
    Data () : memory (0), bytes (0) { }

    uint32_t memory;
    std::vector <DecodedInstruction> expr;
    void* bytes;
};

struct Code
// The code to a function.
// Functions are split between section3 and section10.
// Instructions are in section10.
// Function code is decoded upon first (or only) visit.
{
    Code () : size (0), cursor (0), import (false)
    {
    }

    size_t size;
    uint8_t* cursor;
    std::vector <ValueType> locals; // params in FunctionType
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
        return parameters == other.parameters && results == other.results;
    }
};

struct Module : ModuleBase
{
    virtual ~Module() { }

    Module () : base (0), file_size (0), end (0), start (0), main (0),
        import_function_count (0),
        import_table_count (0),
        import_memory_count (0),
        import_global_count (0)
    {
    }

    MemoryMappedFile mmf;
    uint8_t* base;
    uint64_t file_size;
    uint8_t* end;
    Section sections [12];
    //std::vector <std::shared_ptr<Section>> custom_sections; // FIXME

    // The order can be take advantage of.
    // For example global is read before any code,
    // so the index of any global.get/set can be validated right away.
    std::vector <FunctionType> function_types; // section1 function signatures
    std::vector <Import> imports; // section2
    std::vector <Function> functions; // section3 and section10 function declarations
    std::vector <TableType> tables; // section4 indirect tables
    std::vector <Global> globals; // section6
    std::vector <Export> exports; // section7
    std::vector <Element> elements; // section9 table initialization
    std::vector <Code> code; // section10
    std::vector <Data> data; // section11 memory initialization
    Limits memory_limits;

    Export* start;
    Export* main;

    size_t import_function_count;
    size_t import_table_count;
    size_t import_memory_count;
    size_t import_global_count;

    WasmString read_string (uint8_t** cursor);

    int32_t read_i32 (uint8_t** cursor);
    int64_t read_i64 (uint8_t** cursor);
    float read_f32 (uint8_t** cursor);
    double read_f64 (uint8_t** cursor);

    uint8_t read_byte (uint8_t** cursor);
    uint8_t read_varuint7 (uint8_t** cursor);
    uint32_t read_varuint32 (uint8_t** cursor);

    void read_vector_varuint32 (std::vector <uint32_t>&, uint8_t** cursor);
    Limits read_limits (uint8_t** cursor);
    MemoryType read_memorytype (uint8_t** cursor);
    GlobalType read_globaltype (uint8_t** cursor);
    TableType read_tabletype (uint8_t** cursor);
    ValueType read_valuetype (uint8_t** cursor);
    Tag read_blocktype(uint8_t** cursor);
    TableElementType read_elementtype (uint8_t** cursor);
    bool read_mutable (uint8_t** cursor);
    void read_section (uint8_t** cursor);
    void read_module (const char* file_name);
    void read_vector_ValueType (std::vector <ValueType>& result, uint8_t** cursor);
    void read_function_type (FunctionType& functionType, uint8_t** cursor);

    virtual void read_types (uint8_t** cursor);
    virtual void read_imports (uint8_t** cursor);
    virtual void read_functions (uint8_t** cursor);
    virtual void read_tables (uint8_t** cursor);
    virtual void read_memory (uint8_t** cursor);
    virtual void read_globals (uint8_t** cursor);
    virtual void read_exports (uint8_t** cursor);
    virtual void read_start (uint8_t** cursor)
    {
        ThrowString ("Start::read not yet implemented");
    }
    virtual void read_elements (uint8_t** cursor);
    virtual void read_code (uint8_t** cursor);
    virtual void read_data (uint8_t** cursor);
};

InstructionEnum
DecodeInstructions (Module* module, std::vector <DecodedInstruction>& instructions, uint8_t** cursor, Code* code);

void Module::read_data (uint8_t** cursor)
{
    const size_t size1 = read_varuint32 (cursor);
    printf ("reading data11 size:%" FORMAT_SIZE "X\n", size1);
    data.resize (size1);
    for (size_t i = 0; i < size1; ++i)
    {
        Data& a = data [i];
        a.memory = read_varuint32 (cursor);
        DecodeInstructions (this, a.expr, cursor, 0);
        const size_t size2 = read_varuint32 (cursor);
        if (*cursor + size2 > end)
            ThrowString ("data out of bounds");
        a.bytes = *cursor;
        printf ("data [%" FORMAT_SIZE "X]:{%X}\n", i, (*cursor) [0]);
        *cursor += size2;
    }
    printf ("read data11 size:%" FORMAT_SIZE "X\n", size1);
}

void Module::read_code (uint8_t** cursor)
{
    printf ("reading CodeSection10\n");
    const size_t size = read_varuint32 (cursor);
    printf ("reading CodeSection size:%" FORMAT_SIZE "X\n", size);
    if (*cursor + size > end)
        ThrowString (StringFormat ("code out of bounds cursor:%p end:%p size:%" FORMAT_SIZE "X line:%X", *cursor, end, size, __LINE__));
    const size_t old = code.size ();
    AssertFormat (old == import_function_count, ("%" FORMAT_SIZE "X %" FORMAT_SIZE "X", old, import_function_count));
    code.resize (old + size);
    for (size_t i = 0; i < size; ++i)
    {
        Code& a = code [old + i];
        a.import = false;
        a.size = read_varuint32 (cursor);
        if (*cursor + a.size > end)
            ThrowString (StringFormat ("code out of bounds cursor:%p end:%p size:%" FORMAT_SIZE "X line:%X", *cursor, end, (long_t)a.size, __LINE__));
        a.cursor = *cursor;
        printf ("code [%" FORMAT_SIZE "X]: %p/%" FORMAT_SIZE "X\n", (long_t)i, a.cursor, (long_t)a.size);
        if (a.size)
        {
            //printf (InstructionName ((*cursor) [0]));
            *cursor += a.size;
        }
    }
}

void Module::read_elements (uint8_t** cursor)
{
    const size_t size1 = read_varuint32 (cursor);
    printf ("reading section9 elements size1:%" FORMAT_SIZE "X\n", size1);
    elements.resize (size1);
    for (size_t i = 0; i < size1; ++i)
    {
        Element& a = elements [i];
        a.table = read_varuint32 (cursor);
        DecodeInstructions (this, a.offset_instructions, cursor, 0);
        const size_t size2 = read_varuint32 (cursor);
        a.functions.resize (size2);
        for (size_t j = 0; j < size2; ++j)
        {
            uint32_t& b = a.functions [j];
            b = read_varuint32 (cursor);
            printf ("elem.function [%" FORMAT_SIZE "X/%" FORMAT_SIZE "X]:%X\n", j, size2, b);
        }
    }
    printf ("read elements9 size:%" FORMAT_SIZE "X\n", size1);
}

void Module::read_exports (uint8_t** cursor)
{
    printf ("reading section 7\n");
    const size_t size = read_varuint32 (cursor);
    printf ("reading exports7 count:%" FORMAT_SIZE "X\n", size);
    exports.resize (size);
    for (size_t i = 0; i < size; ++i)
    {
        Export& a = exports [i];
        a.name = read_string (cursor);
        a.tag = (ExportTag)read_byte (cursor);
        a.function = read_varuint32 (cursor);
        a.is_main = a.name.builtin == BuiltinString_main;
        a.is_start = a.name.builtin == BuiltinString_start;
        printf ("read_export %" FORMAT_SIZE "X:%" FORMAT_SIZE "X %s tag:%X index:%X is_main:%X is_start:%X\n", i, size, a.name.c_str (), a.tag, a.function, a.is_main, a.is_start);

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
    printf ("read exports7 size:%" FORMAT_SIZE "X\n", size);
}

void Module::read_globals (uint8_t** cursor)
{
    //printf ("reading section 6\n");
    const size_t size = read_varuint32 (cursor);
    printf ("reading globals6 size:%" FORMAT_SIZE "X\n", size);
    globals.resize (size);
    for (size_t i = 0; i < size; ++i)
    {
        Global& a = globals [i];
        a.global_type = read_globaltype (cursor);
        printf ("read_globals %" FORMAT_SIZE "X:%" FORMAT_SIZE "X value_type:%X  mutable:%X init:%p\n", i, size, a.global_type.value_type, a.global_type.is_mutable, *cursor);
        DecodeInstructions (this, a.init, cursor, 0);
        // Init points to code -- Instructions until end of block 0x0B Instruction.
    }
    printf ("read globals6 size:%" FORMAT_SIZE "X\n", size);
}

void Module::read_functions (uint8_t** cursor)
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

void Module::read_imports (uint8_t** cursor)
{
    printf ("reading section 2\n");
    const size_t size = read_varuint32 (cursor);
    imports.resize (size);
    // TODO two passes to limit realloc?
    for (size_t i = 0; i < size; ++i)
    {
        Import& r = imports [i];
        r.module = read_string (cursor);
        r.name = read_string (cursor);
        ImportTag tag = r.tag = (ImportTag)read_byte (cursor);
        printf ("import %s.%s %X\n", r.module.c_str (), r.name.c_str (), (uint32_t)tag);
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
        (long_t)import_function_count,
        (long_t)import_table_count,
        (long_t)import_memory_count,
        (long_t)import_global_count);

    // TODO fill in more about imports?
    Code imported_code;
    imported_code.import = true;
    code.resize (import_function_count, imported_code);
}

void Module::read_vector_ValueType (std::vector <ValueType>& result, uint8_t** cursor)
{
    const size_t size = read_varuint32 (cursor);
    result.resize (size);
    for (size_t i = 0; i < size; ++i)
        result [i] = read_valuetype (cursor);
}

void Module::read_function_type (FunctionType& functionType, uint8_t** cursor)
{
    read_vector_ValueType (functionType.parameters, cursor);
    read_vector_ValueType (functionType.results, cursor);
}

void Module::read_types (uint8_t** cursor)
{
    printf ("reading section 1\n");
    const size_t size = read_varuint32 (cursor);
    function_types.resize (size);
    for (size_t i = 0; i < size; ++i)
    {
        const uint32_t marker = read_byte (cursor);
        if (marker != 0x60)
            ThrowString ("malformed2 in Types::read");
        read_function_type (function_types [i], cursor);
    }
    printf ("read section 1\n");
}

InstructionEnum
DecodeInstructions (Module* module, std::vector <DecodedInstruction>& instructions, uint8_t** cursor, Code* code)
{
    uint32_t b0 = (uint32_t)Block;
    size_t index = 0;
    uint32_t b1 = 0;
    size_t pc = ~(size_t)0;

    while (b0 != (uint32_t)BlockEnd && b0 != (uint32_t)Else)
    {
        ++pc;
        InstructionEncoding e;
        DecodedInstruction i;
        b0 = module->read_byte (cursor); // TODO multi-byte instructions
        e = instructionEncode [b0];
        if (e.fixed_size == 0)
        {
#if _WIN32
            if (IsDebuggerPresent ()) DebugBreak();
#endif
            ThrowString ("reserved");
        }
        i.name = e.name;
        i.file_offset = (uint64_t)(*cursor - module->base - 1);
        if (e.fixed_size == 2) // TODO
        {
            b1 = module->read_byte (cursor);
            if (b1)
                ThrowString ("second byte not 0");
        }
        printf ("decode1:%" FORMAT_SIZE "X %s\n", pc, InstructionName (i.name));
        size_t if_false = 0;
        size_t if_end = 0;
        switch (e.immediate)
        {
        case Imm_sequence:
            i.blockType = module->read_blocktype (cursor);
            index = instructions.size ();
            instructions.push_back (i);
            InstructionEnum next;
            next = DecodeInstructions (module, instructions, cursor, code);
            Assert (next == BlockEnd || (i.name == If && next == Else));
            switch (b0)
            {
            default:
                Assert (!"invalid Imm_sequnce");
                break;
            case If:
#include "diag-switch-push.h"
                switch (next)
                {
                default:
                    Assert (!"invalid next after If");
                    break;
                case BlockEnd:
                    if_false = instructions.size () - 1; // to BlockEnd
                    if_end = instructions.size () - 1; // to BlockEnd
                    break;
                case Else:
                    if_false = instructions.size (); // past Else
                    // If we fall to Else tell it how many values to keep.
                    instructions [if_false - 1].blockType = i.blockType;
                    next = DecodeInstructions (module, instructions, cursor, code);
                    Assert (next == BlockEnd);
                    if_end = instructions.size () - 1; // to BlockEnd
                    break;
                }
#include "diag-switch-pop.h"
                instructions [index].if_false = if_false;
                instructions [index].if_end = if_end;
                break;
            case Block: // label is forward
                instructions [index].label = instructions.size (); // past BlockEnd
                break;
            case Loop: // label is backward, to self; each invocation repushes the label
                // and there need not be any instructions between it and
                // a branch -- infinite empty loop -- and a branch
                // to self would fail for lack of labels on stack,
                // or branch incorrect to ever further out labels
                // Obviously TODO is make branches optimized
                // and not deal with a stack at all, at least
                // not as late as currently.
                instructions [index].label = index; // to Loop
                break;
            }
            // If we fall to BlockEnd tell it how many values to keep.
            instructions [instructions.size () - 1].blockType = i.blockType;
            break;
        case Imm_memory:
            i.align = module->read_varuint32 (cursor);
            i.offset = module->read_varuint32 (cursor);
            break;
        case Imm_none:
            break;
        case Imm_global:
        case Imm_label:
        case Imm_function:
        case Imm_local:
        case Imm_type:
            i.u32 = module->read_varuint32 (cursor);
            break;
        default:
            ThrowString ("unknown immediate");
            break;
        case Imm_i32: // Spec is confusing here, signed or unsigned.
            i.i32 = module->read_i32 (cursor);
            break;
        case Imm_i64: // Spec is confusing here, signed or unsigned.
            i.i64 = module->read_i64 (cursor);
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
#include "diag-switch-push.h"
        switch (e.immediate)
        {
        case Imm_global:
            Assert (i.u32 < module->globals.size ());
            break;
        case Imm_label:
            //Assert (i.u32 < module->globals.size ());
            break;
        case Imm_function:
            //Assert (i.u32 < module->globals.size ());
            break;
        case Imm_local:
            //Assert (i.u32 < code->globals.size ());
            break;
        case Imm_type:
            //Assert (i.u32 < module->globals.size ());
            break;
        }
#include "diag-switch-pop.h"
        printf ("decode2:%" FORMAT_SIZE "X %s 0x%X %d\n", pc, InstructionName (i.name), i.i32, i.i32);
        if (e.immediate != Imm_sequence)
            instructions.push_back (i);
    }
    return (InstructionEnum)b0;
}

void DecodeFunction (Module* module, Code* code, uint8_t** cursor)
{
    // read count of types
    // for each type
    //   read type and count for that type
    const size_t local_type_count = module->read_varuint32 (cursor);
    printf ("local_type_count:%" FORMAT_SIZE "X\n", local_type_count);
    for (size_t i = 0; i < local_type_count; ++i)
    {
        const size_t j = module->read_varuint32 (cursor);
        ValueType value_type = module->read_valuetype (cursor);
        printf ("local_type_count %" FORMAT_SIZE "X-of-%" FORMAT_SIZE "X count:%" FORMAT_SIZE "X type:%X\n", i, local_type_count, j, value_type);
        code->locals.resize (code->locals.size () + j, value_type);
    }
    DecodeInstructions (module, code->decoded_instructions, cursor, code);
}

extern const SectionTraits section_traits [ ] =
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
#define SECTION(x, read) {&ModuleBase::read, #x},
SECTIONS

};

int32_t Module::read_i32 (uint8_t** cursor)
// Unspecified signedness is unsigned. Spec is unclear.
{
    return w3::read_varint32 (cursor, end);
}

int64_t Module::read_i64 (uint8_t** cursor)
// Unspecified signedness is unsigned. Spec is unclear.
{
    return (int64_t)w3::read_varint64 (cursor, end);
}

#if _MSC_VER
#pragma warning (push)
#pragma warning (disable:4701) // uninitialized variable
#endif

float Module::read_f32 (uint8_t** cursor)
// floats are not variably sized? Spec is unclear due to fancy notation
// getting in the way.
{
    union {
        uint8_t bytes [4];
        float f32;
    } u;
    for (uint32_t i = 0; i < 4; ++i)
        u.bytes [i] = (uint8_t)read_byte (cursor);
    return u.f32;
}

double Module::read_f64 (uint8_t** cursor)
// floats are not variably sized? Spec is unclear due to fancy notation
// getting in the way.
{
    union {
        uint8_t bytes [8];
        double f64;
    } u;
    for (uint32_t i = 0; i < 8; ++i)
        u.bytes [i] = (uint8_t)read_byte (cursor);
    return u.f64;
}

#if _MSC_VER >= 1200
#pragma warning (pop)
#endif

uint8_t Module::read_varuint7 (uint8_t** cursor)
{
    // TODO move implementation here, i.e. for context, for errors
    return w3::read_varuint7 (cursor, end);
}

uint8_t Module::read_byte (uint8_t** cursor)
{
    // TODO move implementation here, i.e. for context, for errors
    return w3::read_byte (cursor, end);
}

// TODO efficiency
// i.e. string_view or such pointing right into the mmap
WasmString Module::read_string (uint8_t** cursor)
{
    const uint32_t size = read_varuint32 (cursor);
    if (size + *cursor > end)
        ThrowString ("malformed in read_string");
    // TODO UTF8 handling
    WasmString a;
    a.data = (char*)*cursor;
    a.size = size;

    // TODO string recognizer?
    if (size <= INT_MAX)
        printf ("read_string %X:%.*s\n", size, (int)size, *cursor);
    if (size == 7 && !memcmp (*cursor, "$_start", 7))
    {
        a.builtin = BuiltinString_start;
    }
    else if (size == 5 && !memcmp (*cursor, "_main", 5))
    {
        a.builtin = BuiltinString_main;
    }
    *cursor += size;
    return a;
}

void Module::read_vector_varuint32 (std::vector <uint32_t>& result, uint8_t** cursor)
{
    const size_t size = read_varuint32 (cursor);
    result.resize (size);
    for (size_t i = 0; i < size; ++i)
        result [i] = read_varuint32 (cursor);
}

uint32_t Module::read_varuint32 (uint8_t** cursor)
{
    // TODO move implementation here, i.e. for context, for errors
    return w3::read_varuint32 (cursor, end);
}

Limits Module::read_limits (uint8_t** cursor)
{
    Limits limits;
    const uint32_t tag = read_byte (cursor);
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

MemoryType Module::read_memorytype (uint8_t** cursor)
{
    MemoryType m {};
    m.limits = read_limits (cursor);
    return m;
}

bool Module::read_mutable (uint8_t** cursor)
{
    const uint32_t m = read_byte (cursor);
    switch (m)
    {
    case 0:
    case 1: break;
    default:
        ThrowString ("invalid mutable");
    }
    return m == 1;
}

ValueType Module::read_valuetype (uint8_t** cursor)
{
    const uint32_t value_type = read_byte (cursor);
    switch (value_type)
    {
    default:
        ThrowString (StringFormat ("invalid ValueType:%X", value_type));
        break;
    case Tag_i32:
    case Tag_i64:
    case Tag_f32:
    case Tag_f64:
        break;
    }
    return (ValueType)value_type;
}

Tag Module::read_blocktype(uint8_t** cursor)
{
    const uint32_t block_type = read_byte (cursor);
    switch (block_type)
    {
    default:
        ThrowString (StringFormat ("invalid BlockType:%X", block_type));
        break;
    case Tag_i32:
    case Tag_i64:
    case Tag_f32:
    case Tag_f64:
    case ResultType_empty:
        break;
    }
    return (Tag)block_type;
}

GlobalType Module::read_globaltype (uint8_t** cursor)
{
    GlobalType globalType;
    globalType.value_type = read_valuetype (cursor);
    globalType.is_mutable = read_mutable (cursor);
    return globalType;
}

TableElementType Module::read_elementtype (uint8_t** cursor)
{
    TableElementType elementType = (TableElementType)read_byte (cursor);
    if (elementType != TableElementType_funcRef)
        ThrowString ("invalid elementType");
    return elementType;
}

TableType Module::read_tabletype (uint8_t** cursor)
{
    TableType tableType;
    tableType.elementType = read_elementtype (cursor);
    tableType.limits = read_limits (cursor);
    printf ("read_tabletype:type:%X min:%X hasMax:%X max:%X\n", tableType.elementType, tableType.limits.min, tableType.limits.hasMax, tableType.limits.max);
    return tableType;
}

void Module::read_memory (uint8_t** cursor)
{
    printf ("reading section5\n");
    const size_t size = read_varuint32 (cursor);
    AssertFormat (size <= 1, ("%" FORMAT_SIZE "X", size)); // FUTURE
    for (size_t i = 0; i < size; ++i)
        memory_limits = read_limits (cursor);
    printf ("read section5 min:%X hasMax:%X max:%X\n", memory_limits.min, memory_limits.hasMax, memory_limits.max);
}

void Module::read_tables (uint8_t** cursor)
{
    const size_t size = read_varuint32 (cursor);
    printf ("reading tables size:%" FORMAT_SIZE "X\n", size);
    AssertFormat (size == 1, ("%" FORMAT_SIZE "X", size));
    tables.resize (size);
    for (size_t i = 0; i < size; ++i)
        tables [0] = read_tabletype (cursor);
}

void Module::read_section (uint8_t** cursor)
{
    uint8_t* payload = *cursor;
    const uint32_t id = read_varuint7 (cursor);

    if (id > 11)
        ThrowString (StringFormat ("malformed line:%d id:%X payload:%p base:%p end:%p", __LINE__, id, payload, base, end)); // UNDONE context

    printf("%s(%d)\n", __FILE__, __LINE__);

    const size_t payload_size = read_varuint32 (cursor);
    printf ("%s payload_size:%" FORMAT_SIZE "X\n", __func__, (long_t)payload_size);
    payload = *cursor;
    uint32_t name_size = 0;
    char* name = 0;
    if (id == 0)
    {
        name_size = read_varuint32 (cursor);
        name = (char*)*cursor;
        if (name + name_size > (char*)end)
            ThrowString (StringFormat ("malformed %d", __LINE__)); // UNDONE context (move to module or section)
    }
    if (payload + payload_size > end)
        ThrowString (StringFormat ("malformed line:%d id:%X payload:%p payload_size:%" FORMAT_SIZE "X base:%p end:%p", __LINE__, id, payload, (long_t)payload_size, base, end)); // UNDONE context

    printf("%s(%d)\n", __FILE__, __LINE__);

    *cursor = payload + payload_size;

    if (id == 0)
    {
        if (name_size < INT_MAX)
            printf ("skipping custom section:.%.*s\n", (int)name_size, name);
        // UNDONE custom sections
        return;
    }

    printf("%s(%d)\n", __FILE__, __LINE__);

    Section& section = sections [id];
    section.id = id;
    section.name.data = name;
    section.name.size = name_size;
    section.payload_size = payload_size;
    section.payload = payload;

    printf("%s(%d) %d\n", __FILE__, __LINE__, (int)id);
    //DebugBreak ();

    (this->*section_traits [id].read) (&payload);

    if (payload != *cursor)
        ThrowString (StringFormat ("failed to read section:%X payload:%p cursor:%p\n", id, payload, *cursor));
}

void Module::read_module (const char* file_name)
{
    mmf.read (file_name);
    base = (uint8_t*)mmf.base;
    file_size = mmf.file.get_file_size ();
    end = file_size + (uint8_t*)base;

    if (file_size < 8)
        ThrowString (StringFormat ("too small %s", file_name));

    uint32_t magic = GetUint32LE(base);
    uint32_t version = GetUint32LE(base + 4);
    printf ("magic: %X\n", magic);
    printf ("version: %X\n", version);

    if (memcmp (&magic,"\0asm", 4))
        ThrowString (StringFormat ("incorrect magic: %X", (uint32_t)magic));

    if (version != 1)
        ThrowString (StringFormat ("incorrect version: %X", (uint32_t)version));

    // Valid module with no sections.
    if (file_size == 8)
        return;

    uint8_t* cursor = base + 8;
    while (cursor < end)
        read_section (&cursor);

    Assert (cursor == end);
}

// TODO once we have Validate, Interp, Jit, CppGen,
// we might invert this structure and have a class per instruction with those 4 virtual functions.
// Or we will token-paste those names on to the instruction names,
// in order to avoid virtual function call cost. Let's get Interp working first.
struct IWasm
{
    DecodedInstruction* instr; // TODO make local variable

    IWasm () : instr (0) { }

    virtual ~IWasm () { }

    virtual void Reserved () = 0;
#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) void name () { abort (); }
#include __FILE__
};

void
Overflow (void)
{
    Assert (!"Overflow");
}

struct Interp : Stack, IWasm
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
#define RESERVED(b0) INSTRUCTION (0x ## b0, 0, 0, Reserved ## b0, Imm_none, 0, 0, Type_none, Type_none, Type_none, Type_none) { Reserved (); }

#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) virtual void name ();
#include __FILE__
    ;
};

struct RustGen : IWasm
{
private:
    RustGen(const RustGen&);
    void operator =(const RustGen&);
public:

    virtual ~RustGen()
    {
    }

    RustGen() : module (0)
    {
    }

    void* LoadStore (size_t size);

    Module* module;

    void interp (Module* mod, Export* emain = 0)
    {
        // instantiate module
        this->module = mod;
        ModuleInstance instance (module);
        //this->module_instance = &instance;

        // Simulate call to initial function.

        size_t size = mod->functions.size ();
        for (size_t i = 0; i < size; ++i)
        {
            // TODO Rust not C.

            printf("%d\n", (int)i);

            Function& function = mod->functions[i];
            const size_t function_type_index = function.function_type_index;

            printf("%d\n", (int)function_type_index);

            Assert (function_type_index < module->function_types.size ());
            FunctionType* function_type = &module->function_types [function_type_index];

            Code* code = &module->code [function.function_index];
            const size_t param_count = function_type->parameters.size ();

            uint8_t* cursor = code->cursor;
            if (cursor)
            {
                DecodeFunction (module, code, &cursor);
                code->cursor = 0;
            }

            if (function_type->results.size() == 0)
                printf("\nvoid ");
            else
                printf("\n%s ", TypeToStringCxx(function_type->results[0]));

            printf(" function%d ( \n", (int)i);

            if (param_count == 0)
                printf("void");
            else for (size_t j = 0; j < param_count; ++j)
            {
                if (j)
                    printf(",\n");
                printf("%s arg%" FORMAT_SIZE "d", TypeToStringCxx(function_type->parameters[j]), (long_t)j);
            }

            printf(") {");

#if 0
            // Declare locals; maybe eventually merge locals and args.
            const size_t local_count = code->locals.size();
            for (size_t k = 0; k < local_count; ++k)
            {
                printf("%s local%" FORMAT_SIZE "d;\n", TypeToStringCxx(code->locals[k]), (long_t)k);
            }

            instr = &code->decoded_instructions [0];
            DecodedInstruction* end = instr + size;
            for (; instr < end; ++instr)
            {
                Assert (instr);
                switch (instr->name)
                {
                    // break before instead of after to avoid unreachable code warning
#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, pop, push, in0, in1, in2, out0)     \
                break;                                                                           \
                case w3::name:                                                                   \
                printf ("gen%s x:%X u:%u i:%i\n", #name, instr->u32, instr->u32, instr->u32); \
                this->name ();
#include __FILE__
                }
            }
        }
        // TODO handle ret
        //__debugbreak ();
#endif
        printf("\n}\n");
    }

    void Reserved ()
    {
        printf("//reserved\n");
    }

#undef RESERVED
#define RESERVED(b0) void Reserved ## b0 ( ) ; 

#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) void name ();
//#include __FILE__

#if 0

INTERP (Block)
{
    printf("//block\n");
}

INTERP (Loop)
{
    printf("//loop\n");
}

INTERP (MemGrow)
{
    printf("//\n");
}

INTERP (MemSize)
{
    __debugbreak (); // not yet tested
    printf("//\n");
}

INTERP (Global_set)
{
    printf("//\n");
}

INTERP (Global_get)
{
    printf("//\n");
}

INTERP (Local_set)
{
    printf("//\n");
}

INTERP (Local_tee)
{
    printf("//\n");
}

INTERP (Local_get)
{
    printf("//\n");
}

INTERP (If)
{
    printf("//\n");
}

INTERP (Else)
{
    printf("//\n");
}

INTERP (BlockEnd)
{
    printf("//\n");
}

INTERP (BrIf)
{
    printf("//\n");
}

INTERP (BrTable)
{
    printf("//\n");
}

INTERP (Ret)
{
    printf("//\n");
}

INTERP (Br)
{
    printf("//\n");
}

INTERP (Select)
{
    printf("//\n");
}

INTERP (Calli)
{
    printf("//\n");
}

INTERP (Unreach)
{
    printf("//\n");
}

INTERP (i32_Const)
{
    printf("//\n");
}

INTERP (i64_Const)
{
    printf("//\n");
}

INTERP (f32_Const)
{
    printf("//\n");
}

INTERP (f64_Const)
{
    printf("//\n");
}

INTERP (i32_Load_)
{
    printf("//\n");
}

INTERP (i32_Load8s)
{
    printf("//\n");
}

INTERP (i32_Load16s)
{
    printf("//\n");
}

INTERP (i32_Load8u)
{
    printf("//\n");
}

INTERP (i32_Load16u)
{
    printf("//\n");
}

INTERP (i64_Load_)
{
    printf("//\n");
}

INTERP (i64_Load8s)
{
    printf("//\n");
}

INTERP (i64_Load16s)
{
    printf("//\n");
}

INTERP (i64_Load8u)
{
    push_i64 (*(uint8_t*)LoadStore (1));
}

INTERP (i64_Load16u)
{
    push_i64 (*(uint16_t*)LoadStore (2));
}

INTERP (i64_Load32s)
{
    push_i64 (*(int32_t*)LoadStore (4));
}

INTERP (i64_Load32u)
{
    push_i64 (*(uint32_t*)LoadStore (4));
}

INTERP (f32_Load_)
{
    push_f32 (*(float*)LoadStore (4));
}

INTERP (f64_Load_)
{
    push_f64 (*(double*)LoadStore (8));
}

INTERP (i32_Store_)
{
    const uint32_t a = pop_u32 ();
    *(uint32_t*)LoadStore (4) = a;
}

INTERP (i32_Store8)
{
    const uint32_t a = pop_u32 ();
    *(uint8_t*)LoadStore (1) = (uint8_t)(a & 0xFF);
}

INTERP (i32_Store16)
{
    const uint32_t a = pop_u32 ();
    *(uint16_t*)LoadStore (1) = (uint16_t)(a & 0xFFFF);
}

INTERP (i64_Store8)
{
    const uint64_t a = pop_u64 ();
    *(uint8_t*)LoadStore (1) = (uint8_t)(a & 0xFF);
}

INTERP (i64_Store16)
{
    const uint64_t a = pop_u64 ();
    *(uint16_t*)LoadStore (2) = (uint16_t)(a & 0xFFFF);
}

INTERP (i64_Store32)
{
    const uint64_t a = pop_u64 ();
    *(uint32_t*)LoadStore (4) = (uint32_t)(a & 0xFFFFFFFF);
}

INTERP (i64_Store_)
{
    const uint64_t a = pop_u64 ();
    *(uint64_t*)LoadStore (8) = a;
}

INTERP (f32_Store_)
{
    float a = pop_f32 ();
    *(float*)LoadStore (4) = a;
}

INTERP (f64_Store_)
{
    double a = pop_f64 ();
    *(double*)LoadStore (8) = a;
}

INTERP (Nop)
{
}

INTERP (Drop)
{
    pop_value ();
}

void Interp:: Reserved ()
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

INTERP (Eq_i32_)
{
    push_bool (pop_i32 () == pop_i32 ());
}

INTERP (Eq_i64_)
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

INTERP (Ne_i32_)
{
    push_bool (pop_i32 () != pop_i32 ());
}

INTERP (Ne_i64_)
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
    const int32_t b = pop_i32 ();
    const int32_t a = pop_i32 ();
    push_bool (a < b);
}

INTERP (Lt_i32u)
{
    const uint32_t b = pop_u32 ();
    const uint32_t a = pop_u32 ();
    push_bool (a < b);
}

INTERP (Lt_i64s)
{
    const int64_t b = pop_i64 ();
    const int64_t a = pop_i64 ();
    push_bool (a < b);
}

INTERP (Lt_i64u)
{
    const uint64_t b = pop_u32 ();
    const uint64_t a = pop_u32 ();
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
    const int32_t b = pop_i32 ();
    const int32_t a = pop_i32 ();
    push_bool (a > b);
}

INTERP (Gt_i32u)
{
    const uint32_t b = pop_u32 ();
    const uint32_t a = pop_u32 ();
    push_bool (a > b);
}

INTERP (Gt_i64s)
{
    const int64_t b = pop_i64 ();
    const int64_t a = pop_i64 ();
    push_bool (a > b);
}

INTERP (Gt_i64u)
{
    const uint64_t b = pop_u32 ();
    const uint64_t a = pop_u32 ();
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
    const int32_t b = pop_i32 ();
    push_bool (pop_i32 () <= b);
}

INTERP (Le_i64s)
{
    const int64_t b = pop_i64 ();
    push_bool (pop_i64 () <= b);

}

INTERP (Le_i32u)
{
    const uint32_t b = pop_u32 ();
    push_bool (pop_u32 () <= b);
}

INTERP (Le_i64u)
{
    const uint64_t b = pop_u64 ();
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
    const uint32_t b = pop_u32 ();
    const uint32_t a = pop_u32 ();
    push_bool (a >= b);
}

INTERP (Ge_i64u)
{
    const uint64_t b = pop_u64 ();
    const uint64_t a = pop_u64 ();
    push_bool (a >= b);
}

INTERP (Ge_i32s)
{
    const int32_t b = pop_i32 ();
    const int32_t a = pop_i32 ();
    push_bool (a >= b);
}

INTERP (Ge_i64s)
{
    const int64_t b = pop_i64 ();
    const int64_t a = pop_i64 ();
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

template <class T>
uint32_t
count_set_bits (T a)
{
    uint32_t n = 0;
    while (a)
    {
        n += (a & 1);
        a >>= 1;
    }
    return n;
}

template <class T>
uint32_t
count_trailing_zeros (T a)
{
    uint32_t n = 0;
    while (!(a & 1))
    {
        ++n;
        a >>= 1;
    }
    return n;
}

template <class T>
uint32_t
count_leading_zeros (T a)
{
    uint32_t n = 0;
    while (!(a & (((T)1) << ((sizeof (T) * 8) - 1))))
    {
        ++n;
        a <<= 1;
    }
    return n;
}

INTERP (CountSetBits_i32)
{
    uint32_t& a = u32 ();
#if (_M_AMD64 || _M_IX86) && _MSC_VER > 1100 // TODO which version
    a = __popcnt (a);
#else
    a = count_set_bits (a);
#endif
}

INTERP (CountSetBits_i64)
{
    uint64_t& a = u64 ();
#if _MSC_VER && _M_AMD64
    a = __CountSetBits64 (a);
#else
    a = count_set_bits (a);
#endif
}

INTERP (CountTrailingZeros_i32)
{
    uint32_t& a = u32 ();
    a = count_trailing_zeros (a);
}

INTERP (CountTrailingZeros_i64)
{
    uint64_t& a = u64 ();
    a = count_trailing_zeros (a);
}

INTERP (CountLeadingZeros_i32)
{
    uint32_t& a = u32 ();
    a = count_leading_zeros (a);
}

INTERP (CountLeadingZeros_i64)
{
    uint64_t& a = u64 ();
    a = count_leading_zeros (a);
}

INTERP (Add_i32)
{
    const uint32_t a = pop_u32 ();
    u32 () += a;
}

INTERP (Add_i64)
{
    const uint64_t a = pop_u64 ();
    u64 () += a;
}

INTERP (Sub_i32)
{
    const uint32_t a = pop_u32 ();
    u32 () -= a;
}

INTERP (Sub_i64)
{
    const uint64_t a = pop_u64 ();
    u64 () -= a;
}

INTERP (Mul_i32)
{
    const uint32_t a = pop_u32 ();
    u32 () *= a;
}

INTERP (Mul_i64)
{
    const uint64_t a = pop_u64 ();
    u64 () *= a;
}

INTERP (Div_s_i32)
{
    const int32_t a = pop_i32 ();
    i32 () /= a;
}

INTERP (Div_u_i32)
{
    const uint32_t a = pop_u32 ();
    u32 () /= a;
}

INTERP (Rem_s_i32)
{
    const int32_t a = pop_i32 ();
    i32 () %= a;
}

INTERP (Rem_u_i32)
{
    const uint32_t a = pop_u32 ();
    u32 () %= a;
}

INTERP (Div_s_i64)
{
    const int64_t a = pop_i64 ();
    i64 () /= a;
}

INTERP (Div_u_i64)
{
    const uint64_t a = pop_u64 ();
    u64 () /= a;
}

INTERP (Rem_s_i64)
{
    const int64_t a = pop_i64 ();
    i64 () %= a;
}

INTERP (Rem_u_i64)
{
    const uint64_t a = pop_u64 ();
    u64 () %= a;
}

INTERP (And_i32)
{
    const uint32_t a = pop_u32 ();
    u32 () &= a;
}

INTERP (And_i64)
{
    const uint64_t a = pop_u64 ();
    u64 () &= a;
}

INTERP (Or_i32)
{
    const uint32_t a = pop_u32 ();
    u32 () |= a;
}

INTERP (Or_i64)
{
    const uint64_t a = pop_u64 ();
    u64 () |= a;
}

INTERP (Xor_i32)
{
    const uint32_t a = pop_u32 ();
    u32 () ^= a;
}

INTERP (Xor_i64)
{
    const uint64_t a = pop_u64 ();
    u64 () ^= a;
}

INTERP (Shl_i32)
{
    const uint32_t a = pop_u32 ();
    u32 () <<= (a & 31);
}

INTERP (Shl_i64)
{
    const uint64_t a = pop_u64 ();
    u64 () <<= (a & 63);
}

INTERP (Shr_s_i32)
{
    const int32_t b = pop_i32 ();
    i32 () <<= (b & 31);
}

INTERP (Shr_s_i64)
{
    const int64_t b = pop_i64 ();
    i64 () <<= (b & 63);
}

INTERP (Shr_u_i32)
{
    const uint32_t b = pop_u32 ();
    u32 () >>= (b & 31);
}

INTERP (Shr_u_i64)
{
    const uint64_t b = pop_u64 ();
    u64 () >>= (b & 63);
}

INTERP (Rotl_i32)
{
    const int n = 32;
    const int32_t b = (pop_i32 () & (n - 1));
    uint32_t& r = u32 ();
    uint32_t a = r;
#if _MSC_VER
    r = _rotl (a, b);
#else
    r = ((a << b) | (a >> (n - b)));
#endif
}

INTERP (Rotl_i64)
{
    const uint32_t n = 64;
    const uint32_t b = (uint32_t)(pop_u64 () & (n - 1));
    uint64_t& r = u64 ();
    uint64_t a = r;
#if _MSC_VER > 1100 // TODO which version
    r = _rotl64 (a, (int)b);
#else
    r = (a << b) | (a >> (n - b));
#endif
}

INTERP (Rotr_i32)
{
    const uint32_t n = 32;
    const uint32_t b = (uint32_t)(pop_u32 () & (n - 1));
    uint32_t& r = u32 ();
    uint32_t a = r;
#if _MSC_VER
    r = _rotr (a, (int)b);
#else
    r = (a >> b) | (a << (n - b));
#endif
}

INTERP (Rotr_i64)
{
    const uint32_t n = 64;
    const uint32_t b = (uint32_t)(pop_u64 () & (n - 1));
    uint64_t& r = u64 ();
    uint64_t a = r;
#if _MSC_VER > 1100 // TODO which version
    r = _rotr64 (a, (int)b);
#else
    r = (a >> b) | (a << (n - b));
#endif
}

INTERP (Abs_f32)
{
    float& z = f32 ();
    z = fabsf (z);
}

INTERP (Abs_f64)
{
    double& z = f64 ();
    z = fabs (z);
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
    z = wasm_floorf (z);
}

INTERP (Floor_f64)
{
    double& z = f64 ();
    z = floor (z);
}

INTERP (Trunc_f32)
{
    float& z = f32 ();
    z = wasm_truncf (z); // TODO C99
}

INTERP (Trunc_f64)
{
    double& z = f64 ();
    z = wasm_truncd (z);
}

INTERP (Nearest_f32)
{
    float& z = f32 ();
    z = wasm_roundf (z);
}

INTERP (Nearest_f64)
{
    double& z = f64 ();
    z = wasm_roundd (z);
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
    z1 = std::min (z1, z2);
}

INTERP (Min_f64)
{
    const double z2 = pop_f64 ();
    double& z1 = f64 ();
    z1 = std::min (z1, z2);
}

INTERP (Max_f32)
{
    const float z2 = pop_f32 ();
    float& z1 = f32 ();
    z1 = std::max (z1, z2);
}

INTERP (Max_f64)
{
    const double z2 = pop_f64 ();
    double& z1 = f64 ();
    z1 = std::max (z1, z2);
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

INTERP (i32_Wrap_i64_)
{
    set_i32 ((int32_t)(i64 () & 0xFFFFFFFF));
}

INTERP (i32_Trunc_f32s)
{
    set_i32 ((int32_t)f32 ());
}

INTERP (i32_Trunc_f32u)
{
    set_u32 ((uint32_t)f32 ());
}

INTERP (i32_Trunc_f64s)
{
    set_i32 ((int32_t)f64 ());
}

INTERP (i32_Trunc_f64u)
{
    set_u32 ((uint32_t)f64 ());
}

INTERP (i64_Extend_i32s)
{
    set_i64 ((int64_t)i32 ());
}

INTERP (i64_Extend_i32u)
{
    set_u64 ((uint64_t)u32 ());
}

INTERP (i64_Trunc_f32s)
{
    set_i64 ((int64_t)f32 ());
}

INTERP (i64_Trunc_f32u)
{
    set_u64 ((uint64_t)f32 ());
}

INTERP (i64_Trunc_f64s)
{
    set_i64 ((int64_t)f64 ());
}

INTERP (i64_Trunc_f64u)
{
    set_u64 ((uint64_t)f64 ());
}

INTERP (f32_Convert_i32u)
{
    set_f32 ((float)(uint32_t)u32 ());
}

INTERP (f32_Convert_i32s)
{
    set_f32 ((float)i32 ());
}

template <class T>
T uint64_to_float (uint64_t ui64, T * unused = 0 /* old compiler bug workaround */)
{
#if _MSC_VER && _MSC_VER <= 1100 // error C2520: conversion from unsigned __int64 to double not implemented, use signed __int64
    __int64 i64 = (__int64)ui64;
    if (i64 >= 0)
    {
        return (T)(__int64)ui64;
    }
    __int64 low_bit = (__int64)(ui64 & 1);
    unsigned __int64 uhalf = ui64 >> 1;
    __int64 half = (__int64)uhalf;
    Assert (half >= 0);
    T f = (T)half;
    f *= 2;
    f += low_bit;
    return f;
#else
    return (T)ui64;
#endif
}

INTERP (f32_Convert_i64u)
{
    set_f32 (uint64_to_float (u64 (), (float*)0));
}

INTERP (f32_Convert_i64s)
{
    set_f32 ((float)i64 ());
}

INTERP (f32_Demote_f64_)
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
    set_f64 (uint64_to_float (u64 (), (double*)0));
}

INTERP (f64_Promote_f32_)
{
    set_f64 ((double)f32 ());
}

// reinterpret; these could be more automated

INTERP (i32_Reinterpret_f32_)
{
    tag (Tag_f32) = Tag_i32;
}

INTERP (f32_Reinterpret_i32_)
{
    tag (Tag_i32) = Tag_f32;
}

INTERP (i64_Reinterpret_f64_)
{
    // casting requires values, so call a function
    // future: temporaries
    top() = "i64_Reinterpret_f64_(" + top() + ")";
}

INTERP (f64_Reinterpret_i64_)
{
    // casting requires values, so call a function
    // future: temporaries
    top() = "f64_Reinterpret_i64_(" + top() + ")";
}

#endif

};

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
    size_t effective_address;
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
        const size_t u = IntMagnitude (i);
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

INTERP (Call)
{
    // FIXME In the instruction table
    const size_t function_index = instr->u32;
    Assert (function_index < module->functions.size ());
    Function* function = &module->functions [function_index];
    function->function_index = function_index; // TODO remove this
    Invoke (module->functions [function_index]);
}

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

    (end () - 1 - (ssize_t)param_count)->tag = StackTag_Frame;
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
#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, pop, push, in0, in1, in2, out0)     \
            break;                                                                           \
            case w3::name:                                                                   \
            printf ("interp%s x:%X u:%u i:%i\n", #name, instr->u32, instr->u32, instr->u32); \
            this->name ();
#include __FILE__
        }
        // special handling
        if (instr->name == w3::Ret) // gross but most choices are
            break;
#include "diag-switch-push.h"
        switch (instr->name)
        {
        case w3::Call:
        case w3::Calli:
            frame = &frame_value; // TODO should handled in Ret.
            break;
        }
#include "diag-switch-pop.h"
    }
    instr = previous;
    // TODO handle ret
    //__debugbreak ();
}

INTERP (Block)
{
    // Label is end.
    StackValue stack_value (StackTag_Label);
    stack_value.label.arity = instr->Arity ();
    stack_value.label.continuation = instr->label;
    push_label (stack_value);
}

INTERP (Loop)
{
    // Loop and Block are almost the same, esp. after decoding.
    // Loop continuation is start, block continuation is end.
    Block ();
    back ().label.arity = 0;
}

INTERP (MemGrow)
{
    //__debugbreak ();
    int32_t result = -1;
    const size_t previous_size = module_instance->memory.size ();
    const size_t page_growth = pop_u32 ();
    if (page_growth == 0)
    {
        result = (int32_t)(previous_size >> PageShift);
    }
    else
    {
        const size_t new_size = previous_size + (page_growth << PageShift);
        try
        {
            module_instance->memory.resize (new_size, 0);
            result = (int32_t)(previous_size >> PageShift);
        }
        catch (...)
        {
        }
    }
    push_i32 (result);
}

INTERP (MemSize)
{
    __debugbreak (); // not yet tested
    push_i32 ((int32_t)(module_instance->memory.size () >> PageShift));
}

INTERP (Global_set)
{
    const size_t i = instr->u32;
    AssertFormat (i < module->globals.size (), ("%" FORMAT_SIZE "X %" FORMAT_SIZE "X", i, module->globals.size ()));
    // TODO assert mutable
    AssertFormat (tag () == module->globals [i].global_type.value_type, ("%X %X", tag (), module->globals [i].global_type.value_type));
    module_instance->globals [i].value.value = value ();
    pop_value ();
}

INTERP (Global_get)
{
    const size_t i = instr->u32;
    AssertFormat (i < module->globals.size (), ("%" FORMAT_SIZE "X %" FORMAT_SIZE "X", i, module->globals.size ()));
    push_value (StackValue (module_instance->globals [i].value));
    AssertFormat (tag () == module->globals [i].global_type.value_type, ("%X %X", tag (), module->globals [i].global_type.value_type));
}

INTERP (Local_set)
{
    Local_tee ();
    pop_value ();
}

INTERP (Local_tee)
{
    //__debugbreak ();
    const size_t i = instr->u32;
    AssertFormat (i < frame->param_and_local_count, ("%" FORMAT_SIZE "X %" FORMAT_SIZE "X", i, frame->param_and_local_count));
    if (i < frame->param_count)
        AssertFormat (tag () == frame->param_types [i], ("%X %" FORMAT_SIZE "X", tag (), frame->param_types [i]));
    else
        AssertFormat (tag () == frame->local_only_types [i - frame->param_count], ("%X %X", tag (), frame->local_only_types [i - frame->param_count]));
    AssertFormat (tag () == frame->Local (i).value.tag, ("%X %X", tag (), frame->Local (i).value.tag));
    frame->Local (i).value.value = value ();
}

INTERP (Local_get)
{
    const size_t i = instr->u32;
    AssertFormat (i < frame->param_and_local_count, ("%" FORMAT_SIZE "X %" FORMAT_SIZE "X", i, frame->param_and_local_count));
    push_value (StackValue (frame->Local (i)));
    if (i < frame->param_count)
        AssertFormat (tag () == frame->param_types [i], ("%X %X", tag (), frame->param_types [i]));
    else
        AssertFormat (tag () == frame->local_only_types [i - frame->param_count], ("%X %X", tag (), frame->local_only_types [i - frame->param_count]));
}

INTERP (If)
{
     __debugbreak ();

    const uint32_t condition = pop_u32 ();

    // Push the same label either way.
    StackValue stack_value (StackTag_Label);
    stack_value.label.arity = instr->Arity ();
    stack_value.label.continuation = instr->if_end;
    push_label (stack_value);

    // If condition is false, skip ahead, to just past the Else.
    // The Else actually marks the end of If, more than the start of Else.
    if (!condition)
    {
        // Branch to one before target, because interpreter loop will increment.
        instr = &frame->code->decoded_instructions [instr->if_false] - 1;
    }
}

INTERP (Else)
{
     __debugbreak ();

    // Else marks the end of If is like BlockEnd, but also
    // skips the Else block.
    //.
    // If we are actually running the else case,
    // the If will have branched past the else instruction.

    BlockEnd ();
    // Branch to one before target, because interpreter loop will increment.
    instr = &frame->code->decoded_instructions [instr->if_end] - 1;
}

INTERP (BlockEnd)
{
    // Pop values until label is found.
    // Pop label.
    // Repush values.
    //
    // Alternatively, look for label,
    // and shift values down.
    //
    // Alternatively, something much faster.

    const size_t s = size ();
    size_t j = s;

    AssertFormat (s > 0, ("%" FORMAT_SIZE "X", s));

    // Skip any number of values until one label is found,

    StackValue* p = &front ();

    while (j > 0 && p [j - 1].tag == StackTag_Value)
        --j;

    // FIXME mark earlier if end of block or function
    // FIXME And then assert?

    Assert (j > 0 && (p [j - 1].tag == StackTag_Label || p [j - 1].tag == StackTag_Frame));

    for (size_t i = j - 1; i < s; ++i)
        p [i] = p [i + 1];

    resize (s - 1);
}

INTERP (BrIf)
{
    //DumpStack ("brIfStart");

    const uint32_t condition = pop_u32 ();

    if (condition)
    {
        printf ("BrIfTaken condition:%X label:%X\n", condition, instr->u32);
        Br ();
    }
    else
    {
        printf ("BrIfNotTaken condition:%X label:%X\n", condition, instr->u32);
    }
    //DumpStack ("brIfEnd");
}

INTERP (BrTable)
{
    Assert (!"BrTable"); // not yet implemented
}

INTERP (Ret)
{
    //__debugbreak ();

    Assert (!empty ());
    Assert (frame);

    FunctionType* function_type = frame->function_type;
    size_t arity = function_type->results.size ();
    Assert (arity == 0 || arity == 1);
    Assert (size () > arity);

    StackValue result;
    if (arity)
    {
        result = top ();
        pop_value ();
    }
    while (!empty () && top ().tag != StackTag_Frame)
        pop ();

    Assert (!empty ());
    Assert (top ().tag == StackTag_Frame);
//    frame = top ().frame; // TODO Why this is not working?
    pop ();

    if (arity)
        push_value (result);
}

INTERP (Br)
{
    DumpStack ("brStart");

    // This is confusing.

    // Walk the stack.
    // Label + 1 times.
    // Skipping values.
    // Checking for labels.
    // When arrive at the label + 1'th label, find the arity.
    // There must be at least arity values before the first label.

    const size_t label = instr->u32 + 1;

    //printf ("Br label:%" FORMAT_SIZE "X\n", label - 1);

    Assert (label);
    Assert (size () >= label);

    StackValue* p = &front ();
    size_t initial_values = 0;
    size_t j = size ();
    size_t arity = 0;

    // Iterate to find the arity.

    for (size_t i = 0; i != label; ++i)
    {
        while (j > 0 && p [j - 1].tag == StackTag_Value)
        {
            initial_values += (i == 0);
            --j; // Pop_values.
        }
        Assert (j > 0 && p [j - 1].tag == StackTag_Label);
        arity = p [j - 1].label.arity;
        Assert (arity == 0 || arity == 1); // FUTURE
        --j; // pop_label
    }

    Assert (initial_values >= arity);
    Assert (j >= arity);

    // Get the result.
    StackValue result;
    if (arity)
    {
        result = top ();
        pop_value ();
        --j;
    }

    // Branch to one before target, because interpreter loop will increment.
    instr = &frame->code->decoded_instructions [p [j].label.continuation] - 1;

    // Bulk pop.
    //printf ("Br resize %" FORMAT_SIZE "X => %" FORMAT_SIZE "X\n", size (), j);
    resize (j);

    // Repush result.
    if (arity)
        push_value (result);

    //DumpStack ("brEnd");
}

INTERP (Select)
{
    if (pop_i32 ())
    {
        pop_value ();
        Assert (size () >= 1);
        AssertTopIsValue ();
    }
    else
    {
        const StackValue val2 = top ();
        pop_value ();
        pop_value ();
        push_value (val2);
    }
}

INTERP (Calli)
{
    // TODO signed or unsigned
    // call is unsigned
    const size_t function_index = pop_u32 ();

    const size_t type_index1 = instr->u32;

    // This seems like it could be validated earlier.
    Assert (function_index < module->functions.size ());

    Function& function = module->functions [function_index];

    const size_t type_index2 = function.function_type_index;

    Assert (type_index1 < module->function_types.size ());
    Assert (type_index2 < module->function_types.size ());

    const FunctionType* type1 = &module->function_types [type_index1];
    const FunctionType* type2 = &module->function_types [type_index2];

    Assert (type_index2 == type_index1 || *type1 == *type2);

    Assert (type1->results.size () <= 1); // future

    Invoke (function);
}

ModuleInstance::ModuleInstance (Module* mod) : module (mod)
{
    // size memory
    memory.resize (module->memory_limits.min << PageShift, 0);

    // initialize memory TODO
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

INTERP (i32_Load_)
{
    push_i32 (*(int32_t*)LoadStore (4));
}

INTERP (i32_Load8s)
{
    push_i32 (*(int8_t*)LoadStore (1));
}

INTERP (i32_Load16s)
{
    push_i32 (*(int16_t*)LoadStore (2));
}

INTERP (i32_Load8u)
{
    push_i32 (*(uint8_t*)LoadStore (1));
}

INTERP (i32_Load16u)
{
    push_i32 (*(uint16_t*)LoadStore (2));
}

INTERP (i64_Load_)
{
    push_i64 (*(int64_t*)LoadStore (8));
}

INTERP (i64_Load8s)
{
    push_i64 (*(int8_t*)LoadStore (1));
}

INTERP (i64_Load16s)
{
    push_i64 (*(int16_t*)LoadStore (2));
}

INTERP (i64_Load8u)
{
    push_i64 (*(uint8_t*)LoadStore (1));
}

INTERP (i64_Load16u)
{
    push_i64 (*(uint16_t*)LoadStore (2));
}

INTERP (i64_Load32s)
{
    push_i64 (*(int32_t*)LoadStore (4));
}

INTERP (i64_Load32u)
{
    push_i64 (*(uint32_t*)LoadStore (4));
}

INTERP (f32_Load_)
{
    push_f32 (*(float*)LoadStore (4));
}

INTERP (f64_Load_)
{
    push_f64 (*(double*)LoadStore (8));
}

INTERP (i32_Store_)
{
    const uint32_t a = pop_u32 ();
    *(uint32_t*)LoadStore (4) = a;
}

INTERP (i32_Store8)
{
    const uint32_t a = pop_u32 ();
    *(uint8_t*)LoadStore (1) = (uint8_t)(a & 0xFF);
}

INTERP (i32_Store16)
{
    const uint32_t a = pop_u32 ();
    *(uint16_t*)LoadStore (1) = (uint16_t)(a & 0xFFFF);
}

INTERP (i64_Store8)
{
    const uint64_t a = pop_u64 ();
    *(uint8_t*)LoadStore (1) = (uint8_t)(a & 0xFF);
}

INTERP (i64_Store16)
{
    const uint64_t a = pop_u64 ();
    *(uint16_t*)LoadStore (2) = (uint16_t)(a & 0xFFFF);
}

INTERP (i64_Store32)
{
    const uint64_t a = pop_u64 ();
    *(uint32_t*)LoadStore (4) = (uint32_t)(a & 0xFFFFFFFF);
}

INTERP (i64_Store_)
{
    const uint64_t a = pop_u64 ();
    *(uint64_t*)LoadStore (8) = a;
}

INTERP (f32_Store_)
{
    float a = pop_f32 ();
    *(float*)LoadStore (4) = a;
}

INTERP (f64_Store_)
{
    double a = pop_f64 ();
    *(double*)LoadStore (8) = a;
}

INTERP (Nop)
{
}

INTERP (Drop)
{
    pop_value ();
}

void Interp:: Reserved ()
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

INTERP (Eq_i32_)
{
    push_bool (pop_i32 () == pop_i32 ());
}

INTERP (Eq_i64_)
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

INTERP (Ne_i32_)
{
    push_bool (pop_i32 () != pop_i32 ());
}

INTERP (Ne_i64_)
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
    const int32_t b = pop_i32 ();
    const int32_t a = pop_i32 ();
    push_bool (a < b);
}

INTERP (Lt_i32u)
{
    const uint32_t b = pop_u32 ();
    const uint32_t a = pop_u32 ();
    push_bool (a < b);
}

INTERP (Lt_i64s)
{
    const int64_t b = pop_i64 ();
    const int64_t a = pop_i64 ();
    push_bool (a < b);
}

INTERP (Lt_i64u)
{
    const uint64_t b = pop_u32 ();
    const uint64_t a = pop_u32 ();
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
    const int32_t b = pop_i32 ();
    const int32_t a = pop_i32 ();
    push_bool (a > b);
}

INTERP (Gt_i32u)
{
    const uint32_t b = pop_u32 ();
    const uint32_t a = pop_u32 ();
    push_bool (a > b);
}

INTERP (Gt_i64s)
{
    const int64_t b = pop_i64 ();
    const int64_t a = pop_i64 ();
    push_bool (a > b);
}

INTERP (Gt_i64u)
{
    const uint64_t b = pop_u32 ();
    const uint64_t a = pop_u32 ();
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
    const int32_t b = pop_i32 ();
    push_bool (pop_i32 () <= b);
}

INTERP (Le_i64s)
{
    const int64_t b = pop_i64 ();
    push_bool (pop_i64 () <= b);

}

INTERP (Le_i32u)
{
    const uint32_t b = pop_u32 ();
    push_bool (pop_u32 () <= b);
}

INTERP (Le_i64u)
{
    const uint64_t b = pop_u64 ();
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
    const uint32_t b = pop_u32 ();
    const uint32_t a = pop_u32 ();
    push_bool (a >= b);
}

INTERP (Ge_i64u)
{
    const uint64_t b = pop_u64 ();
    const uint64_t a = pop_u64 ();
    push_bool (a >= b);
}

INTERP (Ge_i32s)
{
    const int32_t b = pop_i32 ();
    const int32_t a = pop_i32 ();
    push_bool (a >= b);
}

INTERP (Ge_i64s)
{
    const int64_t b = pop_i64 ();
    const int64_t a = pop_i64 ();
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

template <class T>
uint32_t
count_set_bits (T a)
{
    uint32_t n = 0;
    while (a)
    {
        n += (a & 1);
        a >>= 1;
    }
    return n;
}

template <class T>
uint32_t
count_trailing_zeros (T a)
{
    uint32_t n = 0;
    while (!(a & 1))
    {
        ++n;
        a >>= 1;
    }
    return n;
}

template <class T>
uint32_t
count_leading_zeros (T a)
{
    uint32_t n = 0;
    while (!(a & (((T)1) << ((sizeof (T) * 8) - 1))))
    {
        ++n;
        a <<= 1;
    }
    return n;
}

INTERP (CountSetBits_i32)
{
    uint32_t& a = u32 ();
#if (_M_AMD64 || _M_IX86) && _MSC_VER > 1100 // TODO which version
    a = __popcnt (a);
#else
    a = count_set_bits (a);
#endif
}

INTERP (CountSetBits_i64)
{
    uint64_t& a = u64 ();
#if _MSC_VER && _M_AMD64
    a = __popcnt64 (a);
#else
    a = count_set_bits (a);
#endif
}

INTERP (CountTrailingZeros_i32)
{
    uint32_t& a = u32 ();
    a = count_trailing_zeros (a);
}

INTERP (CountTrailingZeros_i64)
{
    uint64_t& a = u64 ();
    a = count_trailing_zeros (a);
}

INTERP (CountLeadingZeros_i32)
{
    uint32_t& a = u32 ();
    a = count_leading_zeros (a);
}

INTERP (CountLeadingZeros_i64)
{
    uint64_t& a = u64 ();
    a = count_leading_zeros (a);
}

INTERP (Add_i32)
{
    const uint32_t a = pop_u32 ();
    u32 () += a;
}

INTERP (Add_i64)
{
    const uint64_t a = pop_u64 ();
    u64 () += a;
}

INTERP (Sub_i32)
{
    const uint32_t a = pop_u32 ();
    u32 () -= a;
}

INTERP (Sub_i64)
{
    const uint64_t a = pop_u64 ();
    u64 () -= a;
}

INTERP (Mul_i32)
{
    const uint32_t a = pop_u32 ();
    u32 () *= a;
}

INTERP (Mul_i64)
{
    const uint64_t a = pop_u64 ();
    u64 () *= a;
}

INTERP (Div_s_i32)
{
    const int32_t a = pop_i32 ();
    i32 () /= a;
}

INTERP (Div_u_i32)
{
    const uint32_t a = pop_u32 ();
    u32 () /= a;
}

INTERP (Rem_s_i32)
{
    const int32_t a = pop_i32 ();
    i32 () %= a;
}

INTERP (Rem_u_i32)
{
    const uint32_t a = pop_u32 ();
    u32 () %= a;
}

INTERP (Div_s_i64)
{
    const int64_t a = pop_i64 ();
    i64 () /= a;
}

INTERP (Div_u_i64)
{
    const uint64_t a = pop_u64 ();
    u64 () /= a;
}

INTERP (Rem_s_i64)
{
    const int64_t a = pop_i64 ();
    i64 () %= a;
}

INTERP (Rem_u_i64)
{
    const uint64_t a = pop_u64 ();
    u64 () %= a;
}

INTERP (And_i32)
{
    const uint32_t a = pop_u32 ();
    u32 () &= a;
}

INTERP (And_i64)
{
    const uint64_t a = pop_u64 ();
    u64 () &= a;
}

INTERP (Or_i32)
{
    const uint32_t a = pop_u32 ();
    u32 () |= a;
}

INTERP (Or_i64)
{
    const uint64_t a = pop_u64 ();
    u64 () |= a;
}

INTERP (Xor_i32)
{
    const uint32_t a = pop_u32 ();
    u32 () ^= a;
}

INTERP (Xor_i64)
{
    const uint64_t a = pop_u64 ();
    u64 () ^= a;
}

INTERP (Shl_i32)
{
    const uint32_t a = pop_u32 ();
    u32 () <<= (a & 31);
}

INTERP (Shl_i64)
{
    const uint64_t a = pop_u64 ();
    u64 () <<= (a & 63);
}

INTERP (Shr_s_i32)
{
    const int32_t b = pop_i32 ();
    i32 () <<= (b & 31);
}

INTERP (Shr_s_i64)
{
    const int64_t b = pop_i64 ();
    i64 () <<= (b & 63);
}

INTERP (Shr_u_i32)
{
    const uint32_t b = pop_u32 ();
    u32 () >>= (b & 31);
}

INTERP (Shr_u_i64)
{
    const uint64_t b = pop_u64 ();
    u64 () >>= (b & 63);
}

INTERP (Rotl_i32)
{
    const int n = 32;
    const int32_t b = (pop_i32 () & (n - 1));
    uint32_t& r = u32 ();
    uint32_t a = r;
#if _MSC_VER
    r = _rotl (a, b);
#else
    r = ((a << b) | (a >> (n - b)));
#endif
}

INTERP (Rotl_i64)
{
    const uint32_t n = 64;
    const uint32_t b = (uint32_t)(pop_u64 () & (n - 1));
    uint64_t& r = u64 ();
    uint64_t a = r;
#if _MSC_VER > 1100 // TODO which version
    r = _rotl64 (a, (int)b);
#else
    r = (a << b) | (a >> (n - b));
#endif
}

INTERP (Rotr_i32)
{
    const uint32_t n = 32;
    const uint32_t b = (uint32_t)(pop_u32 () & (n - 1));
    uint32_t& r = u32 ();
    uint32_t a = r;
#if _MSC_VER
    r = _rotr (a, (int)b);
#else
    r = (a >> b) | (a << (n - b));
#endif
}

INTERP (Rotr_i64)
{
    const uint32_t n = 64;
    const uint32_t b = (uint32_t)(pop_u64 () & (n - 1));
    uint64_t& r = u64 ();
    uint64_t a = r;
#if _MSC_VER > 1100 // TODO which version
    r = _rotr64 (a, (int)b);
#else
    r = (a >> b) | (a << (n - b));
#endif
}

INTERP (Abs_f32)
{
    float& z = f32 ();
    z = fabsf (z);
}

INTERP (Abs_f64)
{
    double& z = f64 ();
    z = fabs (z);
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
    z = wasm_floorf (z);
}

INTERP (Floor_f64)
{
    double& z = f64 ();
    z = floor (z);
}

INTERP (Trunc_f32)
{
    float& z = f32 ();
    z = wasm_truncf (z); // TODO C99
}

INTERP (Trunc_f64)
{
    double& z = f64 ();
    z = wasm_truncd (z);
}

INTERP (Nearest_f32)
{
    float& z = f32 ();
    z = wasm_roundf (z);
}

INTERP (Nearest_f64)
{
    double& z = f64 ();
    z = wasm_roundd (z);
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
    z1 = std::min (z1, z2);
}

INTERP (Min_f64)
{
    const double z2 = pop_f64 ();
    double& z1 = f64 ();
    z1 = std::min (z1, z2);
}

INTERP (Max_f32)
{
    const float z2 = pop_f32 ();
    float& z1 = f32 ();
    z1 = std::max (z1, z2);
}

INTERP (Max_f64)
{
    const double z2 = pop_f64 ();
    double& z1 = f64 ();
    z1 = std::max (z1, z2);
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

INTERP (i32_Wrap_i64_)
{
    set_i32 ((int32_t)(i64 () & 0xFFFFFFFF));
}

INTERP (i32_Trunc_f32s)
{
    set_i32 ((int32_t)f32 ());
}

INTERP (i32_Trunc_f32u)
{
    set_u32 ((uint32_t)f32 ());
}

INTERP (i32_Trunc_f64s)
{
    set_i32 ((int32_t)f64 ());
}

INTERP (i32_Trunc_f64u)
{
    set_u32 ((uint32_t)f64 ());
}

INTERP (i64_Extend_i32s)
{
    set_i64 ((int64_t)i32 ());
}

INTERP (i64_Extend_i32u)
{
    set_u64 ((uint64_t)u32 ());
}

INTERP (i64_Trunc_f32s)
{
    set_i64 ((int64_t)f32 ());
}

INTERP (i64_Trunc_f32u)
{
    set_u64 ((uint64_t)f32 ());
}

INTERP (i64_Trunc_f64s)
{
    set_i64 ((int64_t)f64 ());
}

INTERP (i64_Trunc_f64u)
{
    set_u64 ((uint64_t)f64 ());
}

INTERP (f32_Convert_i32u)
{
    set_f32 ((float)(uint32_t)u32 ());
}

INTERP (f32_Convert_i32s)
{
    set_f32 ((float)i32 ());
}

template <class T>
T uint64_to_float (uint64_t ui64, T * unused = 0 /* old compiler bug workaround */)
{
#if _MSC_VER && _MSC_VER <= 1100 // error C2520: conversion from unsigned __int64 to double not implemented, use signed __int64
    __int64 i64 = (__int64)ui64;
    if (i64 >= 0)
    {
        return (T)(__int64)ui64;
    }
    __int64 low_bit = (__int64)(ui64 & 1);
    unsigned __int64 uhalf = ui64 >> 1;
    __int64 half = (__int64)uhalf;
    Assert (half >= 0);
    T f = (T)half;
    f *= 2;
    f += low_bit;
    return f;
#else
    return (T)ui64;
#endif
}

INTERP (f32_Convert_i64u)
{
    set_f32 (uint64_to_float (u64 (), (float*)0));
}

INTERP (f32_Convert_i64s)
{
    set_f32 ((float)i64 ());
}

INTERP (f32_Demote_f64_)
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
    set_f64 (uint64_to_float (u64 (), (double*)0));
}

INTERP (f64_Promote_f32_)
{
    set_f64 ((double)f32 ());
}

// reinterpret; these could be more automated

INTERP (i32_Reinterpret_f32_)
{
    tag (Tag_f32) = Tag_i32;
}

INTERP (f32_Reinterpret_i32_)
{
    tag (Tag_i32) = Tag_f32;
}

INTERP (i64_Reinterpret_f64_)
{
    tag (Tag_f64) = Tag_i64;
}

INTERP (f64_Reinterpret_i64_)
{
    tag (Tag_i64) = Tag_f64;
}

}

using namespace w3; // TODO C or C++?

int
main (int argc, char** argv)
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
        size_t i {};
        bool run_all_exports {};
        bool rust_gen {};

        Assert(argc >= 0);
        for (i = 1 ; i < (uint32_t)argc; ++i)
        {
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

        if (rust_gen)
        {
            RustGen().interp (&module);
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

#endif
