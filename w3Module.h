// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#include "w3.h"
#include "w3MemoryMappedFile.h"
//#include <string> TODO: namespace
//#include <vector>
#include "w3InstructionEnum.h"
#include "w3InstructionEncoding.h"

struct Function // section3
{
    // Functions are split between two sections: types in section3, locals/body in section10
    size_t function_index {}; // TODO needed?
    size_t function_type_index {};
    size_t local_only_count {};
    size_t param_count {};
    bool import {}; // TODO needed?
};

// Initial representation of X and XSection are the same.
// This might evolve, i.e. into separate TypesSection and Types,
// or just Types that is not Section.
struct FunctionType
{
    // CONSIDER pointer into mmf
    std::vector <Tag> parameters;
    std::vector <Tag> results;

    bool operator == (const FunctionType& other) const
    {
        return parameters == other.parameters && results == other.results;
    }
};

#include "w3BuiltinString.h"
#include "w3WasmString.h"
#include "w3Section.h"

typedef enum ImportTag {    // aka desc
    ImportTag_Function = 0, // aka type
    ImportTag_Table = 1,
    ImportTag_Memory = 2,
    ImportTag_Global = 3,
} ImportTag, ExportTag;

struct Limits
{
    // TODO size_t? null?
    uint32_t min {};
    uint32_t max {};
    bool hasMax {};
};

struct MemoryType
{
    Limits limits {};
};

struct GlobalType
{
    Tag value_type {};
    bool is_mutable {};
};

struct TableType
{
    Tag elementType {};
    Limits limits {};
};

struct Import
{
    WasmString module {};
    WasmString name {};
    ImportTag tag {(ImportTag)-1};
    // TODO virtual functions to model union
    //union
    //{
        TableType table {};
        uint32_t function {};
        MemoryType memory {};
        GlobalType global {};
    //};
};

enum InstructionEnum;

struct DecodedInstructionZeroInit // ZeroMem-compatible part
{
    DecodedInstructionZeroInit()
    {
        static DecodedInstructionZeroInit a;
        *this = a;
    }
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
    int id; //sourcegen
};

struct DecodedInstruction : DecodedInstructionZeroInit
{
    size_t Arity() const
    {
        return (blockType == Tag_empty) ? 0u : 1u; // FUTURE
    }

    std::vector <uint32_t> vecLabel {};
};

struct Global
{
    GlobalType global_type {};
    std::vector <DecodedInstruction> init {};
};

struct Element
{
    uint32_t table {};
    std::vector <DecodedInstruction> offset_instructions {};
    uint32_t offset {};
    std::vector <uint32_t> functions {};
};

struct Export
{
    WasmString name {};
    ExportTag tag {};
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
    uint32_t memory {};
    std::vector <DecodedInstruction> expr {};
    void* bytes {};
};

struct Code
// The code to a function.
// Functions are split between section3 and section10.
// Instructions are in section10.
// Function code is decoded upon first (or only) visit.
{
    size_t size {};
    uint8_t* cursor {};
    std::vector <Tag> locals {}; // params in FunctionType
    std::vector <DecodedInstruction> decoded_instructions {}; // section10
    bool import {};
};

struct Module
{
    std::string name {};

    virtual ~Module();

    MemoryMappedFile mmf {};
    uint8_t* base {};
    uint64_t file_size {};
    uint8_t* end {};
    Section sections [12] {};
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

    int instructionId {}; //sourcegen

    Export* start {};
    Export* main {};

    size_t import_function_count {};
    size_t import_table_count {};
    size_t import_memory_count {};
    size_t import_global_count {};

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
    Tag read_valuetype (uint8_t** cursor);
    Tag read_blocktype(uint8_t** cursor);
    Tag read_elementtype (uint8_t** cursor);
    bool read_mutable (uint8_t** cursor);
    void read_section (uint8_t** cursor);
    void read_module (PCSTR file_name);
    void read_vector_ValueType (std::vector <Tag>& result, uint8_t** cursor);
    void read_function_type (FunctionType& functionType, uint8_t** cursor);

    virtual void read_types (uint8_t** cursor);
    virtual void read_imports (uint8_t** cursor);
    virtual void read_functions (uint8_t** cursor);
    virtual void read_tables (uint8_t** cursor);
    virtual void read_memory (uint8_t** cursor);
    virtual void read_globals (uint8_t** cursor);
    virtual void read_exports (uint8_t** cursor);
    virtual void read_start (uint8_t** cursor);
    virtual void read_elements (uint8_t** cursor);
    virtual void read_code (uint8_t** cursor);
    virtual void read_data (uint8_t** cursor);
};
