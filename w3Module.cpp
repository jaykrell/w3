// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

// todo split read and write, or read from interp, etc.

#include "w3.h"
#include "w3Module.h"

w3Module::~w3Module()
{
}

void w3Module::read_start (uint8_t** cursor)
{
    ThrowString ("Start::read not yet implemented");
}

w3InstructionEnum DecodeInstructions (w3Module* module, std::vector <w3DecodedInstruction>& instructions, uint8_t** cursor, w3Code* code);

void w3Module::read_data (uint8_t** cursor)
{
    const size_t size1 = read_varuint32 (cursor);
    printf ("reading data11 size:%" FORMAT_SIZE "X\n", size1);
    data.resize (size1);
    for (size_t i = 0; i < size1; ++i)
    {
        w3Data& a = data [i];
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

void w3Module::read_code (uint8_t** cursor)
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
        w3Code& a = code [old + i];
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

void w3Module::read_elements (uint8_t** cursor)
{
    const size_t size1 = read_varuint32 (cursor);
    printf ("reading section9 elements size1:%" FORMAT_SIZE "X\n", size1);
    elements.resize (size1);
    for (size_t i = 0; i < size1; ++i)
    {
        w3Element& a = elements [i];
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

void w3Module::read_exports (uint8_t** cursor)
{
    printf ("reading section 7\n");
    const size_t size = read_varuint32 (cursor);
    printf ("reading exports7 count:%" FORMAT_SIZE "X\n", size);
    exports.resize (size);
    for (size_t i = 0; i < size; ++i)
    {
        w3Export& a = exports [i];
        a.name = read_string (cursor);
        a.tag = (w3ExportTag)read_byte (cursor);
        a.function = read_varuint32 (cursor);
        a.is_main = a.name.builtin == w3BuiltinString_main;
        a.is_start = a.name.builtin == w3BuiltinString_start;
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

void w3Module::read_globals (uint8_t** cursor)
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

void w3Module::read_functions (uint8_t** cursor)
{
    printf ("reading section 3 offset:%" FORMAT_SIZE "X\n", (long_t)(*cursor - this->base));
    const size_t old = functions.size ();
    Assert (old == import_function_count);
    const size_t size = read_varuint32 (cursor);
    functions.resize (old + size);
    for (size_t i = 0; i < size; ++i)
    {
        printf ("read_function %" FORMAT_SIZE "X:%" FORMAT_SIZE "X\n", i, size);
        w3Function& a = functions [old + i];
        a.function_type_index = read_varuint32 (cursor);
        a.function_index = i + old; // TODO probably not needed
        a.import = false; // TODO probably not needed
    }
    printf ("read section 3\n");
}

void w3Module::read_imports (uint8_t** cursor)
{
    printf ("reading section 2 offset:%" FORMAT_SIZE "X\n", (long_t)(*cursor - this->base));
    const size_t size = read_varuint32 (cursor);
    imports.resize (size);
    // TODO two passes to limit realloc?
    for (size_t i = 0; i < size; ++i)
    {
        w3Import& r = imports [i];
        r.module = read_string (cursor);
        r.name = read_string (cursor);
        w3ImportTag tag = r.tag = (w3ImportTag)read_byte (cursor);
        printf ("import %s.%s %X\n", r.module.c_str (), r.name.c_str (), (uint32_t)tag);
        switch (tag)
        {
            // TODO more specific import type and vtable?
        case w3ImportTag_Function:
            r.function = read_varuint32 (cursor); // TODO probably not needed
            ++import_function_count;
            // TODO for each import type
            functions.resize (functions.size () + 1);
            functions.back ().function_index = functions.size () - 1; // TODO remove this field
            functions.back ().function_type_index = r.function;
            functions.back ().import = true; // TODO needed?
            break;
        case w3ImportTag_Table:
            r.table = read_tabletype (cursor);
            ++import_table_count;
            break;
        case w3ImportTag_Memory:
            r.memory = read_memorytype (cursor);
            ++import_memory_count;
            break;
        case w3ImportTag_Global:
            r.global = read_globaltype (cursor);
            ++import_global_count;
            break;
        default:
            ThrowString ("invalid w3ImportTag");
        }
    }
    printf ("read section 2 import_function_count:%" FORMAT_SIZE "X import_table_count:%" FORMAT_SIZE "X import_memory_count:%" FORMAT_SIZE "X import_global_count:%" FORMAT_SIZE "X\n",
        (long_t)import_function_count,
        (long_t)import_table_count,
        (long_t)import_memory_count,
        (long_t)import_global_count);

    // TODO fill in more about imports?
    w3Code imported_code;
    imported_code.import = true;
    code.resize (import_function_count, imported_code);
}

void w3Module::read_vector_ValueType (std::vector <w3Tag>& result, uint8_t** cursor)
{
    const size_t size = read_varuint32 (cursor);
    result.resize (size);
    for (size_t i = 0; i < size; ++i)
        result [i] = read_valuetype (cursor);
}

void w3Module::read_function_type (w3FunctionType& functionType, uint8_t** cursor)
{
    read_vector_ValueType (functionType.parameters, cursor);
    read_vector_ValueType (functionType.results, cursor);
}

void w3Module::read_types (uint8_t** cursor)
{
    printf ("read_types1 offset:%" FORMAT_SIZE "X\n", (long_t)(*cursor - this->base));
    const size_t size = read_varuint32 (cursor);
    function_types.resize (size);
    for (size_t i = 0; i < size; ++i)
    {
        printf ("read_types2 before marker offset:%" FORMAT_SIZE "X\n", (long_t)(*cursor - this->base));
        const uint32_t marker = read_byte (cursor);
        printf ("read_types3 marker:%X offset:%" FORMAT_SIZE "X\n", marker, (long_t)(*cursor - this->base));
        if (marker != 0x60)
            ThrowString ("malformed2 in Types::read");
        read_function_type (function_types [i], cursor);
    }
    printf ("read section 1\n");
}

//todo m3InstructionDecode.cpp?
w3InstructionEnum DecodeInstructions (w3Module* module, std::vector <w3DecodedInstruction>& instructions, uint8_t** cursor, w3Code* code)
{
    uint32_t b0 = (uint32_t)Block;
    size_t index {};
    uint32_t b1 {};
    size_t pc = ~(size_t)0;

    while (b0 != (uint32_t)BlockEnd && b0 != (uint32_t)Else)
    {
        ++pc;
        w3InstructionEncoding e {};
        w3DecodedInstruction i {};
        i.id = ++(module->instructionId);
        b0 = module->read_byte (cursor); // TODO multi-byte instructions
        e = instructionEncode [b0];
        if (e.fixed_size == 0)
        {
#ifdef _WIN32
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
            w3InstructionEnum next;
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
    return (w3InstructionEnum)b0;
}

void DecodeFunction (w3Module* module, w3Code* code, uint8_t** cursor)
{
    // read count of types
    // for each type
    //   read type and count for that type
    const size_t local_type_count = module->read_varuint32 (cursor);
    printf ("local_type_count:%" FORMAT_SIZE "X\n", local_type_count);
    for (size_t i = 0; i < local_type_count; ++i)
    {
        const size_t j = module->read_varuint32 (cursor);
        w3Tag value_type = module->read_valuetype (cursor);
        printf ("local_type_count %" FORMAT_SIZE "X-of-%" FORMAT_SIZE "X count:%" FORMAT_SIZE "X type:%X\n", i, local_type_count, j, value_type);
        code->locals.resize (code->locals.size () + j, value_type);
    }
    DecodeInstructions (module, code->decoded_instructions, cursor, code);
}

//todo: no pointers to member functions? No C++?
struct w3SectionTraits
{
    void (w3Module::*read)(uint8_t** cursor);
    PCSTR name;
};

const
w3SectionTraits section_traits [ ] =
{
    { 0 },
#define SECTIONS                                \
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
#define SECTION(x, read) {&w3Module::read, #x},
SECTIONS

};

int32_t w3Module::read_i32 (uint8_t** cursor)
// Unspecified signedness is unsigned. Spec is unclear.
{
    return ::read_varint32 (cursor, end);
}

int64_t w3Module::read_i64 (uint8_t** cursor)
// Unspecified signedness is unsigned. Spec is unclear.
{
    return (int64_t)::read_varint64 (cursor, end);
}

#if _MSC_VER
#pragma warning (push)
#pragma warning (disable:4701) // uninitialized variable
#endif

float w3Module::read_f32 (uint8_t** cursor)
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

double w3Module::read_f64 (uint8_t** cursor)
// floats are not variably sized? Spec is unclear due to fancy notation
// getting in the way.
{
    union
    {
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

uint8_t w3Module::read_varuint7 (uint8_t** cursor)
{
    // TODO move implementation here, i.e. for context, for errors
    return ::read_varuint7 (cursor, end);
}

uint8_t w3Module::read_byte (uint8_t** cursor)
{
    // TODO move implementation here, i.e. for context, for errors
    return ::read_byte (cursor, end);
}

// TODO efficiency
// i.e. string_view or such pointing right into the mmap
WasmString w3Module::read_string (uint8_t** cursor)
{
    const uint32_t size = read_varuint32 (cursor);
    if (size + *cursor > end)
        ThrowString ("malformed in read_string");
    // TODO UTF8 handling
    WasmString a;
    a.data = (PCH)*cursor;
    a.size = size;

    // TODO string recognizer?
    if (size <= INT_MAX)
        printf ("read_string %X:%.*s\n", size, (int)size, *cursor);
    if (size == 7 && !memcmp (*cursor, "$_start", 7))
    {
        a.builtin = w3BuiltinString_start;
    }
    else if (size == 5 && !memcmp (*cursor, "_main", 5))
    {
        a.builtin = w3BuiltinString_main;
    }
    *cursor += size;
    return a;
}

void w3Module::read_vector_varuint32 (std::vector <uint32_t>& result, uint8_t** cursor)
{
    const size_t size = read_varuint32 (cursor);
    result.resize (size);
    for (size_t i = 0; i < size; ++i)
        result [i] = read_varuint32 (cursor);
}

uint32_t w3Module::read_varuint32 (uint8_t** cursor)
{
    // TODO move implementation here, i.e. for context, for errors
    return ::read_varuint32 (cursor, end);
}

w3Limits w3Module::read_limits (uint8_t** cursor)
{
    w3Limits limits;
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

w3MemoryType w3Module::read_memorytype (uint8_t** cursor)
{
    w3MemoryType m {};
    m.limits = read_limits (cursor);
    return m;
}

bool w3Module::read_mutable (uint8_t** cursor)
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

w3Tag w3Module::read_valuetype (uint8_t** cursor)
{
    const uint32_t value_type = read_byte (cursor);
    switch (value_type)
    {
    default:
        ThrowString (StringFormat ("invalid w3Tag:%X", value_type));
        break;
    case Tag_i32:
    case Tag_i64:
    case Tag_f32:
    case Tag_f64:
        break;
    }
    return (w3Tag)value_type;
}

w3Tag w3Module::read_blocktype(uint8_t** cursor)
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
    case Tag_empty:
        break;
    }
    return (w3Tag)block_type;
}

w3GlobalType w3Module::read_globaltype (uint8_t** cursor)
{
    w3GlobalType globalType;
    globalType.value_type = read_valuetype (cursor);
    globalType.is_mutable = read_mutable (cursor);
    return globalType;
}

w3Tag w3Module::read_elementtype (uint8_t** cursor)
{
    w3Tag elementType = (w3Tag)read_byte (cursor);
    if (elementType != Tag_FuncRef)
        ThrowString ("invalid elementType");
    return elementType;
}

w3TableType w3Module::read_tabletype (uint8_t** cursor)
{
    w3TableType tableType;
    tableType.elementType = read_elementtype (cursor);
    tableType.limits = read_limits (cursor);
    printf ("read_tabletype:type:%X min:%X hasMax:%X max:%X\n", tableType.elementType, tableType.limits.min, tableType.limits.hasMax, tableType.limits.max);
    return tableType;
}

void w3Module::read_memory (uint8_t** cursor)
{
    printf ("reading section5\n");
    const size_t size = read_varuint32 (cursor);
    AssertFormat (size <= 1, ("%" FORMAT_SIZE "X", size)); // FUTURE
    for (size_t i = 0; i < size; ++i)
        memory_limits = read_limits (cursor);
    printf ("read section5 min:%X hasMax:%X max:%X\n", memory_limits.min, memory_limits.hasMax, memory_limits.max);
}

void w3Module::read_tables (uint8_t** cursor)
{
    const size_t size = read_varuint32 (cursor);
    printf ("reading tables size:%" FORMAT_SIZE "X\n", size);
    AssertFormat (size == 1, ("%" FORMAT_SIZE "X", size));
    tables.resize (size);
    for (size_t i = 0; i < size; ++i)
        tables [0] = read_tabletype (cursor);
}

void w3Module::read_section (uint8_t** cursor)
{
    uint8_t* payload = *cursor;
    const uint32_t id = read_varuint7 (cursor);

    if (id > 11)
        ThrowString (StringFormat ("malformed line:%d id:%X payload:%p base:%p end:%p", __LINE__, id, payload, base, end)); // UNDONE context

    printf ("%s(%d) read_section offset:%" FORMAT_SIZE "X\n", __FILE__, __LINE__, (long_t)(*cursor - this->base));

    const size_t payload_size = read_varuint32 (cursor);
    printf ("%s payload_size:%" FORMAT_SIZE "X\n", __func__, (long_t)payload_size);
    payload = *cursor;
    uint32_t name_size = 0;
    PCH local_name = 0;
    if (id == 0)
    {
        name_size = read_varuint32 (cursor);
        local_name = (PCH)*cursor;
        if (local_name + name_size > (PCH)end)
            ThrowString (StringFormat ("malformed %d", __LINE__)); // UNDONE context (move to module or section)
    }
    if (payload + payload_size > end)
        ThrowString (StringFormat ("malformed line:%d id:%X payload:%p payload_size:%" FORMAT_SIZE "X base:%p end:%p", __LINE__, id, payload, (long_t)payload_size, base, end)); // UNDONE context

    printf ("%s(%d) read_section offset:%" FORMAT_SIZE "X\n", __FILE__, __LINE__, (long_t)(*cursor - this->base));

    *cursor = payload + payload_size;

    if (id == 0)
    {
        if (name_size < INT_MAX)
            printf ("skipping custom section:.%.*s\n", (int)name_size, local_name);
        // UNDONE custom sections
        return;
    }

    printf("%s(%d)\n", __FILE__, __LINE__);

    w3Section& section = sections [id];
    section.id = id;
    section.name.data = local_name;
    section.name.size = name_size;
    section.payload_size = payload_size;
    section.payload = payload;

    printf("%s(%d) %d\n", __FILE__, __LINE__, (int)id);
    //DebugBreak ();

    (this->*section_traits [id].read) (&payload);

    if (payload != *cursor)
        ThrowString (StringFormat ("failed to read section:%X payload:%p cursor:%p\n", id, payload, *cursor));
}

void w3Module::read_module (PCSTR file_name)
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
        ThrowString (StringFormat ("incorrect magic: %X", magic));

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
