// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#define CGEN(x) void WasmCGen::x ()

using std::string;

char* WasmCGenTemp(char* buf, long id)
{
    sprintf (buf, "temp%ld", id);
    return buf;
}

#define TEMP() WasmCGenTemp((char*)alloca(99), ++(this->temp))

CGEN (Call)
{
    printf ("/*%s*/\n", __func__);

    // FIXME In the instruction table
    const size_t function_index = instr->u32;
    Assert (function_index < module->functions.size ());
    Function* function = &module->functions [function_index];
    function->function_index = function_index; // TODO remove this

    size_t param_count = function->param_count;

    string call = StringFormat("function%d(", (int)function_index);

    // todo of course this might be backwards

    for (size_t i = 0; i < param_count; ++i)
    {
        if (i != param_count - 1)
            call += ",";
        call += pop ();
    }
    call += ")";
    push (call);
}

CGEN (Local_get)
{
    printf ("/*%s*/\n", __func__);
    push (StringFormat("local%d", (int)instr->u32));
}

CGEN (Block)
{
    printf ("/*%s*/\n", __func__);
    Label label;
    label.arity = instr->Arity ();
    label.continuation = instr->label;
    stack.push_label (label);
}

CGEN (Loop)
{
    printf ("/*%s*/\n", __func__);
    Block ();
    labels.top ().arity = 0;
}

CGEN (MemGrow)
{
    printf ("/*%s*/\n", __func__);
    top ().str = "memgrow(" + top ().str + ")";
}

CGEN (MemSize)
{
    printf ("/*%s*/\n", __func__);
    push ("memsize()");
}

CGEN (Global_set)
{
    printf ("/*%s*/\n", __func__);
    printf ("%sglobal%u = (%s)\n", module->name.c_str(), instr->u32, pop().c_str());
}

CGEN (Global_get)
{
    printf ("/*%s*/\n", __func__);
    push (StringFormat("%sglobal%u", module->name.c_str(), instr->u32));
}

CGEN (Local_set)
{
    printf ("/*%s*/\n", __func__);
    Local_tee ();
    pop ();
}

CGEN (Local_tee)
{
    printf ("/*%s*/\n", __func__);
    printf ("local%u = (%s)\n", instr->u32, top ().cstr());
    top ().str = string_format("local%u", instr->u32);
}

CGEN (If)
{
    printf ("/*%s*/\n", __func__);
    printf ("if (%s) {\n", pop().c_str());
}

CGEN (Else)
{
    printf ("/*%s*/\n", __func__);
    printf ("\n} else {\n");
}

CGEN (BlockEnd)
{
    printf ("/*%s*/\n", __func__);
    printf ("\n}\n");
}

CGEN (BrIf)
{
    printf ("/*%s*/\n", __func__);
    printf ("if (%s) goto label%u;\n", cstr(), instr->u32);
    pop();
}

CGEN (BrTable)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Ret)
{
    printf ("/*%s*/\n", __func__);
    if (function_type->results.size ())
    {
        printf ("return (%s);\n", top ().cstr());
    }
    else
    {
        printf ("return;\n");
    }
    stack.clear ();
}

CGEN (Br)
{
    printf ("/*%s*/\n", __func__);
    // This is specified oddly, and presumably for execution.
    // Let's make a simple guess and see how it goes.
    printf ("goto label%u;\n", instr->u32 + 1);
}

CGEN (Select)
{
    // presently like ternary but might be extended
    printf ("/*%s*/\n", __func__);
}

CGEN (Calli)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Unreach)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (i32_Const)
{
    printf ("/*%s*/\n", __func__);
    push_i32 (instr->i32);
}

CGEN (i64_Const)
{
    printf ("/*%s*/\n", __func__);
    push_i64 (instr->i64);
}

CGEN (f32_Const)
{
    printf ("/*%s*/\n", __func__);
    push_f32 (instr->f32);
}

CGEN (f64_Const)
{
    printf ("/*%s*/\n", __func__);
    push_f64 (instr->f64);
}

void WasmCGen::LoadStore (PCSTR stack_type, PCSTR mem_type, bool loadOrStore)
{
    printf ("/*%s*/\n", __func__);
    PCSTR offset = TEMP();
    PCSTR i = TEMP();
    PCSTR result = loadOrStore ? TEMP() : 0;
    PCSTR effective_address = TEMP();
    PCSTR u = TEMP();

    printf ("const long %s = %u;\n", offset, instr->offset);
    printf ("const long %s = %s;\n", i, pop().c_str());
    printf ("long %s;\n", effective_address);
    printf ("%s %s;\n", stack_type, result);

    printf ("if (%s >= 0)\n{\n", i);
    printf ("  %s = %s + (uint32_t)%s;\n", effective_address, offset, i);
    printf ("  if (%s < %s) Overflow();", effective_address, offset);
    printf ("\n}\nelse\n{\n");
    printf ("  const size_t %s = int_magnitude (%s);\n", u, i);
    printf ("  if (%s > %s) Overflow();\n", u, offset);
    printf ("  %s = %s - %s;", effective_address, offset, u);
    printf ("\n}\n");

    printf ("if (%s > UINT_MAX - sizeof(%s)) Overflow();\n", effective_address, mem_type);

    PCSTR load[4] = {result, mem_type, effective_address, module->name.c_str()};

    if (loadOrStore)
    {
        printf ("%s = *(%s*)(%s + (PCH)%s_mem;\n", load[0], load[1], load[2], load[3]);
        push(result);
    }
    else
    {
        printf ("*(%s*)(%s + (PCH)%s_mem = %s;\n", load[1], load[2], load[3], pop().c_str());
    }
}

void WasmCGen::Load (PCSTR stack_type, PCSTR mem_type)
{
    LoadStore(stack_type, mem_type, 1);
}

void WasmCGen::Store (PCSTR stack_type, PCSTR mem_type)
{
    LoadStore(stack_type, mem_type, 0);
}

void WasmCGen::Prefix ()
{
    printf(
"#if _MSC_VER\n"
"typedef signed __int8     int8_t;\n"
"typedef signed __int16    int16_t;\n"
"typedef signed __int32    int32_t;\n"
"typedef signed __int64    int64_t;\n"
"typedef unsigned __int8  uint8_t;\n"
"typedef unsigned __int16 uint16_t;\n"
"typedef unsigned __int32 uint32_t;\n"
"typedef unsigned __int64 uint64_t;\n"
"#include <stddef.h>\n"
"typedef size_t uintptr_t;\n"
"typedef ptrdiff intptr_t;\n"
"typedef ptrdiff ssize_t;\n"
"#else\n"
"#include <stdint.h>\n"
"#endif\n"
"\n"
"uint32_t PopulationCount64 (uint64_t a);\n"
"uint32_t PopulationCount32 (uint32_t a);\n"
"uint32_t CountTrailingZeros64 (uint64_t a);\n"
"uint32_t CountTrailingZeros32 (uint32_t a);\n"
"uint32_t CountLeadingZeros64 (uint64_t a);\n"
"uint32_t CountLeadingZeros32 (uint32_t a);\n"
"void Overflow(void);\n"
"#include <limits.h>\n"
);
}

CGEN (i32_Load_)
{
    Load("int32_t", "int32_t");
}

CGEN (i32_Load8s)
{
    Load("int32_t", "int8_t");
}

CGEN (i32_Load16s)
{
    Load("int32_t", "int16_t");
}

CGEN (i32_Load8u)
{
    Load("int32_t", "uint8_t");
}

CGEN (i32_Load16u)
{
    Load("int32_t", "uint16_t");
}

CGEN (i64_Load_)
{
    Load("int64_t", "int64_t");
}

CGEN (i64_Load8s)
{
    Load("int64_t", "int8_t");
}

CGEN (i64_Load16s)
{
    Load("int64_t", "int16_t");
}

CGEN (i64_Load8u)
{
    Load("int64_t", "uint8_t");
}

CGEN (i64_Load16u)
{
    Load("int64_t", "uint16_t");
}

CGEN (i64_Load32s)
{
    Load("int64_t", "int32_t");
}

CGEN (i64_Load32u)
{
    Load("int64_t", "uint32_t");
}

CGEN (f32_Load_)
{
    Load("float", "float");
}

CGEN (f64_Load_)
{
    Load("double", "double");
}

CGEN (i32_Store_)
{
    Store("int32_t", "int32_t");
}

CGEN (i32_Store8)
{
    Store("int32_t", "int8_t");
}

CGEN (i32_Store16)
{
    Store("int32_t", "int16_t");
}

CGEN (i64_Store8)
{
    Store("int64_t", "int8_t");
}

CGEN (i64_Store16)
{
    Store("int64_t", "int16_t");
}

CGEN (i64_Store32)
{
    Store("int64_t", "int32_t");
}

CGEN (i64_Store_)
{
    Store("int64_t", "int64_t");
}

CGEN (f32_Store_)
{
    Store("float", "float");

}

CGEN (f64_Store_)
{
    Store("double", "double");
}

CGEN (Nop)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Drop)
{
    pop();
}

string WasmCGenCast (PCSTR type, const string& value)
{
    string result = "((";
    result += type;
    result += ")(";
    result += value;
    result += "))";
    return result;
}

void WasmCGenUnary (WasmCGen* cgen, PCSTR function)
{
    printf ("/*%s*/\n", __func__);
    cgen->push (string (function) + "(" + cgen->pop () + ")");
}

void WasmCGenBinary (WasmCGen* cgen, PCSTR function)
{
    printf ("/*%s*/\n", __func__);
    cgen->push (string (function) + "(" + cgen->pop () + "," + cgen->pop () + ")");
}

void WasmCGenCompare (WasmCGen* cgen, PCSTR type, PCSTR cmp, PCSTR b = 0)
{
    printf ("/*%s*/\n", __func__);
    cgen->push (string ("(") + WasmCGenCast (type, cgen->pop ()) + cmp + WasmCGenCast (type, b ? b : cgen->pop ()) + ")");
}

void WasmCGenEq (WasmCGen* cgen, PCSTR type, PCSTR b = 0) { WasmCGenCompare (cgen, type, "==", b); }
void WasmCGenNe (WasmCGen* cgen, PCSTR type) { WasmCGenCompare (cgen, type, "!="); }
void WasmCGenLt (WasmCGen* cgen, PCSTR type) { WasmCGenCompare (cgen, type, "<"); }
void WasmCGenLe (WasmCGen* cgen, PCSTR type) { WasmCGenCompare (cgen, type, "<="); }
void WasmCGenGt (WasmCGen* cgen, PCSTR type) { WasmCGenCompare (cgen, type, ">"); }
void WasmCGenGe (WasmCGen* cgen, PCSTR type) { WasmCGenCompare (cgen, type, ">="); }

CGEN (Eqz_i32) { WasmCGenEq (this, "int32_t", "0"); }
CGEN (Eqz_i64) { WasmCGenEq (this, "int64_t", "0"); }

CGEN (Eq_i32_) { WasmCGenEq (this, "int32_t"); }
CGEN (Eq_i64_) { WasmCGenEq (this, "int64_t"); }
CGEN (Eq_f32) { WasmCGenEq (this, "float"); }
CGEN (Eq_f64) { WasmCGenEq (this, "double"); }
CGEN (Ne_i32_) { WasmCGenNe (this, "int32_t"); }
CGEN (Ne_i64_) { WasmCGenNe (this, "int64_t"); }
CGEN (Ne_f32) { WasmCGenNe (this, "float"); }
CGEN (Ne_f64) { WasmCGenNe (this, "double"); }

CGEN (Lt_i32s) { WasmCGenLt (this, "int32_t"); }
CGEN (Lt_i32u) { WasmCGenLt (this, "uint32_t"); }
CGEN (Lt_i64s) { WasmCGenLt (this, "int64_t"); }
CGEN (Lt_i64u) { WasmCGenLt (this, "uint64_t"); }
CGEN (Lt_f32) { WasmCGenLt (this, "float"); }
CGEN (Lt_f64) { WasmCGenLt (this, "double"); }

CGEN (Gt_i32s) { WasmCGenGt (this, "int32_t"); }
CGEN (Gt_i32u) { WasmCGenGt (this, "uint32_t"); }
CGEN (Gt_i64s) { WasmCGenGt (this, "int64_t"); }
CGEN (Gt_i64u) { WasmCGenGt (this, "uint64_t"); }
CGEN (Gt_f32) { WasmCGenGt (this, "float"); }
CGEN (Gt_f64) { WasmCGenGt (this, "double"); }

CGEN (Le_i32s) { WasmCGenLe (this, "int32_t"); }
CGEN (Le_i64s) { WasmCGenLe (this, "int64_t"); }
CGEN (Le_i32u) { WasmCGenLe (this, "uint32_t"); }
CGEN (Le_i64u) { WasmCGenLe (this, "uint64_t"); }
CGEN (Le_f32) { WasmCGenLe (this, "float"); }
CGEN (Le_f64) { WasmCGenLe (this, "double"); }

CGEN (Ge_i32s) { WasmCGenGe (this, "int32_t"); }
CGEN (Ge_i64s) { WasmCGenGe (this, "int64_t"); }
CGEN (Ge_i32u) { WasmCGenGe (this, "uint32_t"); }
CGEN (Ge_i64u) { WasmCGenGe (this, "uint64_t"); }
CGEN (Ge_f32) { WasmCGenGe (this, "float"); }
CGEN (Ge_f64) { WasmCGenGe (this, "double"); }

CGEN (CountSetBits_i32)
{
    WasmCGenUnary (this, "WasmCountSetBits32");
}

CGEN (CountSetBits_i64)
{
    WasmCGenUnary (this, "WasmCountSetBits64");
}

CGEN (CountTrailingZeros_i32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (CountTrailingZeros_i64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (CountLeadingZeros_i32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (CountLeadingZeros_i64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Add_i32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Add_i64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Sub_i32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Sub_i64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Mul_i32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Mul_i64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Div_s_i32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Div_u_i32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Rem_s_i32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Rem_u_i32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Div_s_i64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Div_u_i64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Rem_s_i64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Rem_u_i64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (And_i32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (And_i64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Or_i32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Or_i64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Xor_i32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Xor_i64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Shl_i32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Shl_i64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Shr_s_i32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Shr_s_i64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Shr_u_i32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Shr_u_i64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Rotl_i32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Rotl_i64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Rotr_i32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Rotr_i64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Abs_f32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Abs_f64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Neg_f32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Neg_f64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Ceil_f32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Ceil_f64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Floor_f32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Floor_f64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Trunc_f32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Trunc_f64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Nearest_f32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Nearest_f64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Sqrt_f32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Sqrt_f64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Add_f32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Add_f64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Sub_f32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Sub_f64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Mul_f32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Mul_f64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Div_f32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Div_f64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Min_f32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Min_f64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Max_f32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Max_f64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Copysign_f32)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (Copysign_f64)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (i32_Wrap_i64_)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (i32_Trunc_f32s)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (i32_Trunc_f32u)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (i32_Trunc_f64s)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (i32_Trunc_f64u)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (i64_Extend_i32s)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (i64_Extend_i32u)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (i64_Trunc_f32s)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (i64_Trunc_f32u)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (i64_Trunc_f64s)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (i64_Trunc_f64u)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (f32_Convert_i32u)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (f32_Convert_i32s)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (f32_Convert_i64u)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (f32_Convert_i64s)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (f32_Demote_f64_)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (f64_Convert_i32s)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (f64_Convert_i32u)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (f64_Convert_i64s)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (f64_Convert_i64u)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (f64_Promote_f32_)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (i32_Reinterpret_f32_)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (f32_Reinterpret_i32_)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (i64_Reinterpret_f64_)
{
    printf ("/*%s*/\n", __func__);
}

CGEN (f64_Reinterpret_i64_)
{
    printf ("/*%s*/\n", __func__);
}
