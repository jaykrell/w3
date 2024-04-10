// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#include "w3.h"
#include "w3Fd.h"
#include "w3Handle.h"
#include "w3MemoryMappedFile.h"
#include "w3Module.h"
#include "ieee.h"
#include "math_private.h"
extern const float wasm_hugef = 1.0e30F;
extern const double wasm_huged = 1.0e300;
#include "w3Value.h"
#include "w3TaggedValue.h"
#include "w3StackValue.h"
#include "w3StackBaseBase.h"
#include "w3StackBase.h"
#include "w3Int.h"

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

#include "w3SourceGenValue.h"
#include "w3SourceGenStack.h"

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

#define NotImplementedYed() (AssertFormat (0, ("not yet implemented %s 0x%08X ", __func__, __LINE__)))

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

struct stderr_stream : Stream
{
    // TODO: Refactor with stdout.
    virtual void write (const void* bytes, size_t size)
    {
        fflush (stderr);
        PCSTR pc = (PCSTR)bytes;
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
        printf ("read_varuint64_2:%X\n", byte);
        abort ();
        result |= (byte & 0x7F) << shift;
        if ((byte & 0x80) == 0)
            return result;
        shift += 7;
    }
}

uint32_t read_varuint32 (uint8_t** cursor, const uint8_t* end)
{
    return (uint32_t)read_varuint64(cursor, end);
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

// struct String { PCH buf; size_t len; };

#if 0

struct Variable
{
    Tag tag;
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

struct Interp;

#include "w3Frame.h"
#include "w3Stack.h"

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

#define ExportTag_Function ImportTag_Function
#define ExportTag_Table ImportTag_Table
#define ExportTag_Memory ImportTag_Memory
#define ExportTag_Global ImportTag_Global

//struct ImportFunction;
//struct ImportTable;
//struct ImportMemory;

#include "w3ExternalValue.h"
#include "w3ExportInstance.h"
#include "w3ModuleInstance.h"
#include "w3FunctionInstance.h"
#include "w3SectionTraits.h"

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

#include "w3Wasm.h"

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
#include "w3Instructions.h"
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
#include "w3Instructions.h"
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
#include "w3Instructions.h"
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
        const size_t magnitude = IntMagnitude (i);
        if (magnitude > offset)
            Overflow ();
        effective_address = offset - magnitude;
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
#include "w3Instructions.h"
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
