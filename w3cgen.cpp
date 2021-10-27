#define CGEN(x) void CGen::x ()

char* temp(char* buf, long id)
{
    sprintf(buf, "temp%ld", id);
    return buf;
}

#define TEMP() temp((char*)_alloca(99, ++(this->temp)))

CGEN (Call)
{
}

CGEN (Local_get)
{
}

CGEN (Block)
{
    Label label {};
    label.arity = instr->Arity ();
    label.continuation = instr->label;
    push_label (label);
}

CGEN (Loop)
{
    Block ();
    labels.top ().arity = 0;
}

CGEN (MemGrow)
{
    top().str = "memgrow(" + top().str + ")";
}

CGEN (MemSize)
{
    push ("memsize()");
}

CGEN (Global_set)
{
    print("%sglobal%u = (%s)\n", module->name.c_str(), instr->u32, pop().cstr());
}

CGEN (Global_get)
{
    push("%sglobal%u", module->name.c_str(), instr->u32);
}

CGEN (Local_set)
{
    Local_tee ();
    pop();
}

CGEN (Local_tee)
{
    print("local%u = (%s)\n", instr->u32, top().cstr());
    top().str = string_format("local%u", instr->u32);
}

CGEN (If)
{
    // If condition is false, skip ahead, to just past the Else.
    // The Else actually marks the end of If, more than the start of Else.

    print("if (%s) {\n", pop().cstr());
}

CGEN (Else)
{
    print("\n} else {\n");
}

CGEN (BlockEnd)
{
    print("\n}\n");
}

CGEN (BrIf)
{
    print("if (%s) goto label%u\n", cstr(), instr->u32);
    pop();
}

CGEN (BrTable)
{
}

CGEN (Ret)
{
    if (function_type->results.size ())
    {
        print("return (%s);\n", stack.top().cstr());
    }
    else
    {
        print("return;\n");
    }
    stack.clear();
}

CGEN (Br)
{
    // This is specified oddly, and presumably for execution.
    // Let's make a simple guess and see how it goes.
    print("goto label%u;\n", instr->u32 + 1);
}

CGEN (Select)
{
    // switch
}

CGEN (Calli)
{
}

CGEN (Unreach)
{
}

CGEN (i32_Const)
{
    push_i32 (instr->i32);
}

CGEN (i64_Const)
{
    push_i64 (instr->i64);
}

CGEN (f32_Const)
{
    push_f32 (instr->f32);
}

CGEN (f64_Const)
{
    push_f64 (instr->f64);
}

void CGen::Load (const char* push_type, const char* load_type, unsigned size)
{
    char* offset = TEMP();
    char* i = TEMP();
    char* result = TEMP();
    char* effective_address = TEMP();
    char* u = TEMP();

    printf("const long %s = %ld\n", offset, instr->offset);
    printf("const long %s = %s\n", i, pop().cstr());
    printf("long %s;\n", effective_address);
    printf("%s %s;\n", push_type, result);

    printf("if (%s >= 0)\n{\n", i);
    printf("  %s = %s + (uint32_t)%s;\n", effective_address, offset, i);
    printf("  if (%s < %s) Overflow();", effective_address, offset);
    printf("\n}\nelse\n{\n");
    printf("  const size_t %s = int_magnitude (%s);\n", u, i);
    printf("  if (%s > %s) Overflow()\n", u, offset);
    printf("  %s = %s - %s;", effective_address, offset, u);
    printf("\n}\n");

    printf("if (%s > UINT_MAX - %u) Overflow();\n", effective_address, size);
    printf("%s = *(%s*)(%s + (char*)%s_mem;\n", result, load_type, effecitve_address, module->name);

    push(result);
}

CGEN (i32_Load_)
{
    Load("int32_t", "int32_t", 4);
}

CGEN (i32_Load8s)
{
    Load("int32_t", "int8_t", 1);
}

CGEN (i32_Load16s)
{
    Load("int32_t", "int16_t", 2);
}

CGEN (i32_Load8u)
{
    Load("int32_t", "uint8_t", 1);
}

CGEN (i32_Load16u)
{
    Load("int32_t", "uint16_t", 2);
}

CGEN (i64_Load_)
{
    Load("int64_t", "int64_t", 8);
}

CGEN (i64_Load8s)
{
    Load("int64_t", "int8_t", 1);
}

CGEN (i64_Load16s)
{
    Load("int64_t", "int16_t", 2);
}

CGEN (i64_Load8u)
{
    Load("int64_t", "uint8_t", 1);
}

CGEN (i64_Load16u)
{
    Load("int64_t", "uint16_t", 2);
}

CGEN (i64_Load32s)
{
    Load("int64_t", "int32_t", 4);
}

CGEN (i64_Load32u)
{
    Load("int64_t", "uint32_t", 4);
}

CGEN (f32_Load_)
{
}

CGEN (f64_Load_)
{
}

CGEN (i32_Store_)
{
}

CGEN (i32_Store8)
{
}

CGEN (i32_Store16)
{
}

CGEN (i64_Store8)
{
}

CGEN (i64_Store16)
{
}

CGEN (i64_Store32)
{
}

CGEN (i64_Store_)
{
}

CGEN (f32_Store_)
{

}

CGEN (f64_Store_)
{
}

CGEN (Nop)
{
}

CGEN (Drop)
{
}

CGEN (Eqz_i32)
{
}

CGEN (Eqz_i64)
{
}

CGEN (Eq_i32_)
{
}

CGEN (Eq_i64_)
{
}

CGEN (Eq_f32)
{
}

CGEN (Eq_f64)
{
}

CGEN (Ne_i32_)
{
}

CGEN (Ne_i64_)
{
}

CGEN (Ne_f32)
{
}

CGEN (Ne_f64)
{
}

CGEN (Lt_i32s)
{
}

CGEN (Lt_i32u)
{
}

CGEN (Lt_i64s)
{
}

CGEN (Lt_i64u)
{
}

CGEN (Lt_f32)
{
}

CGEN (Lt_f64)
{
}

CGEN (Gt_i32s)
{
}

CGEN (Gt_i32u)
{
}

CGEN (Gt_i64s)
{
}

CGEN (Gt_i64u)
{
}

CGEN (Gt_f32)
{
}

CGEN (Gt_f64)
{
}

CGEN (Le_i32s)
{
}

CGEN (Le_i64s)
{
}

CGEN (Le_i32u)
{
}

CGEN (Le_i64u)
{
}

CGEN (Le_f32)
{
}

CGEN (Le_f64)
{
}

CGEN (Ge_i32u)
{
}

CGEN (Ge_i64u)
{
}

CGEN (Ge_i32s)
{
}

CGEN (Ge_i64s)
{

}

CGEN (Ge_f32)
{
}

CGEN (Ge_f64)
{
}

CGEN (Popcnt_i32)
{
}

CGEN (Popcnt_i64)
{
}

CGEN (Ctz_i32)
{
}

CGEN (Ctz_i64)
{
}

CGEN (Clz_i32)
{
}

CGEN (Clz_i64)
{
}

CGEN (Add_i32)
{
}

CGEN (Add_i64)
{
}

CGEN (Sub_i32)
{
}

CGEN (Sub_i64)
{
}

CGEN (Mul_i32)
{
}

CGEN (Mul_i64)
{
}

CGEN (Div_s_i32)
{
}

CGEN (Div_u_i32)
{
}

CGEN (Rem_s_i32)
{
}

CGEN (Rem_u_i32)
{
}

CGEN (Div_s_i64)
{
}

CGEN (Div_u_i64)
{
}

CGEN (Rem_s_i64)
{
}

CGEN (Rem_u_i64)
{
}

CGEN (And_i32)
{
}

CGEN (And_i64)
{
}

CGEN (Or_i32)
{
}

CGEN (Or_i64)
{
}

CGEN (Xor_i32)
{
}

CGEN (Xor_i64)
{
}

CGEN (Shl_i32)
{
}

CGEN (Shl_i64)
{

}

CGEN (Shr_s_i32)
{
}

CGEN (Shr_s_i64)
{
}

CGEN (Shr_u_i32)
{
}

CGEN (Shr_u_i64)
{
}

CGEN (Rotl_i32)
{
}

CGEN (Rotl_i64)
{
}

CGEN (Rotr_i32)
{
}

CGEN (Rotr_i64)
{
}

CGEN (Abs_f32)
{
}

CGEN (Abs_f64)
{
}

CGEN (Neg_f32)
{
}

CGEN (Neg_f64)
{
}

CGEN (Ceil_f32)
{
}

CGEN (Ceil_f64)
{
}

CGEN (Floor_f32)
{

}

CGEN (Floor_f64)
{

}

CGEN (Trunc_f32)
{
}

CGEN (Trunc_f64)
{
}

CGEN (Nearest_f32)
{
}

CGEN (Nearest_f64)
{
}

CGEN (Sqrt_f32)
{
}

CGEN (Sqrt_f64)
{
}

CGEN (Add_f32)
{
}

CGEN (Add_f64)
{
}

CGEN (Sub_f32)
{
}

CGEN (Sub_f64)
{
}

CGEN (Mul_f32)
{
}

CGEN (Mul_f64)
{
}

CGEN (Div_f32)
{
}

CGEN (Div_f64)
{
}

CGEN (Min_f32)
{
}

CGEN (Min_f64)
{
}

CGEN (Max_f32)
{
}

CGEN (Max_f64)
{
}

CGEN (Copysign_f32)
{
}

CGEN (Copysign_f64)
{
}

CGEN (i32_Wrap_i64_)
{
}

CGEN (i32_Trunc_f32s)
{
}

CGEN (i32_Trunc_f32u)
{
}

CGEN (i32_Trunc_f64s)
{
}

CGEN (i32_Trunc_f64u)
{

}

CGEN (i64_Extend_i32s)
{

}

CGEN (i64_Extend_i32u)
{
}

CGEN (i64_Trunc_f32s)
{
}

CGEN (i64_Trunc_f32u)
{
}

CGEN (i64_Trunc_f64s)
{
}

CGEN (i64_Trunc_f64u)
{
}

CGEN (f32_Convert_i32u)
{
}

CGEN (f32_Convert_i32s)
{
}

CGEN (f32_Convert_i64u)
{

}

CGEN (f32_Convert_i64s)
{
}

CGEN (f32_Demote_f64_)
{
}

CGEN (f64_Convert_i32s)
{
}

CGEN (f64_Convert_i32u)
{
}

CGEN (f64_Convert_i64s)
{
}

CGEN (f64_Convert_i64u)
{
}

CGEN (f64_Promote_f32_)
{
}

CGEN (i32_Reinterpret_f32_)
{
}

CGEN (f32_Reinterpret_i32_)
{
}

CGEN (i64_Reinterpret_f64_)
{
}

CGEN (f64_Reinterpret_i64_)
{
}
