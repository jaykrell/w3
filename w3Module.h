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

struct w3Function // section3
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
// or just Types that is not w3Section.
struct w3FunctionType
{
    // CONSIDER pointer into mmf
    std::vector <w3Tag> parameters;
    std::vector <w3Tag> results;

    bool operator == (const w3FunctionType& other) const
    {
        return parameters == other.parameters && results == other.results;
    }
};

#include "w3BuiltinString.h"
#include "w3WasmString.h"
#include "w3Section.h"

typedef enum w3ImportTag {    // aka desc
    w3ImportTag_Function = 0, // aka type
    w3ImportTag_Table = 1,
    w3ImportTag_Memory = 2,
    w3ImportTag_Global = 3,
} w3ImportTag, w3ExportTag;

struct w3Limits
{
    // TODO size_t? null?
    uint32_t min {};
    uint32_t max {};
    bool hasMax {};
};

struct w3MemoryType
{
    w3Limits limits {};
};

struct w3GlobalType
{
    w3Tag value_type {};
    bool is_mutable {};
};

struct w3TableType
{
    w3Tag elementType {};
    w3Limits limits {};
};

struct w3Import
{
    WasmString module {};
    WasmString name {};
    w3ImportTag tag {(w3ImportTag)-1};
    // TODO virtual functions to model union
    //union
    //{
        w3TableType table {};
        uint32_t function {};
        w3MemoryType memory {};
        w3GlobalType global {};
    //};
};

struct w3DecodedInstructionZeroInit // ZeroMem-compatible part
{
    w3DecodedInstructionZeroInit()
    {
        static w3DecodedInstructionZeroInit a;
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
    w3InstructionEnum name;
    w3Tag blockType;
    int id; //sourcegen
};

struct w3DecodedInstruction : w3DecodedInstructionZeroInit
{
    size_t Arity() const
    {
        return (blockType == Tag_empty) ? 0u : 1u; // FUTURE
    }

    std::vector <uint32_t> vecLabel {};
};

struct Global
{
    w3GlobalType global_type {};
    std::vector <w3DecodedInstruction> init {};
};

struct w3Element
{
    uint32_t table {};
    std::vector <w3DecodedInstruction> offset_instructions {};
    uint32_t offset {};
    std::vector <uint32_t> functions {};
};

struct w3Export
{
    WasmString name {};
    w3ExportTag tag {};
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

struct w3Data // section11
{
    uint32_t memory {};
    std::vector <w3DecodedInstruction> expr {};
    void* bytes {};
};

struct w3Code
// The code to a function.
// Functions are split between section3 and section10.
// Instructions are in section10.
// w3Function code is decoded upon first (or only) visit.
{
    size_t size {};
    uint8_t* cursor {};
    std::vector <w3Tag> locals {}; // params in w3FunctionType
    std::vector <w3DecodedInstruction> decoded_instructions {}; // section10
    bool import {};
};

struct w3Module
{
    std::string name {};

    virtual ~w3Module();

    w3MemoryMappedFile mmf {};
    uint8_t* base {};
    uint64_t file_size {};
    uint8_t* end {};
    w3Section sections [12] {};
    //std::vector <std::shared_ptr<w3Section>> custom_sections; // FIXME

    // The order can be take advantage of.
    // For example global is read before any code,
    // so the index of any global.get/set can be validated right away.
    std::vector <w3FunctionType> function_types; // section1 function signatures
    std::vector <w3Import> imports; // section2
    std::vector <w3Function> functions; // section3 and section10 function declarations
    std::vector <w3TableType> tables; // section4 indirect tables
    std::vector <Global> globals; // section6
    std::vector <w3Export> exports; // section7
    std::vector <w3Element> elements; // section9 table initialization
    std::vector <w3Code> code; // section10
    std::vector <w3Data> data; // section11 memory initialization
    w3Limits memory_limits;

    int instructionId {}; //sourcegen

    w3Export* start {};
    w3Export* main {};

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
    w3Limits read_limits (uint8_t** cursor);
    w3MemoryType read_memorytype (uint8_t** cursor);
    w3GlobalType read_globaltype (uint8_t** cursor);
    w3TableType read_tabletype (uint8_t** cursor);
    w3Tag read_valuetype (uint8_t** cursor);
    w3Tag read_blocktype(uint8_t** cursor);
    w3Tag read_elementtype (uint8_t** cursor);
    bool read_mutable (uint8_t** cursor);
    void read_section (uint8_t** cursor);
    void read_module (PCSTR file_name);
    void read_vector_ValueType (std::vector <w3Tag>& result, uint8_t** cursor);
    void read_function_type (w3FunctionType& functionType, uint8_t** cursor);

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
