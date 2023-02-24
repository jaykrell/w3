// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

INTERP (Call)
{
    // FIXME In the instruction table
    const size_t function_index = instr->u32;
    Assert (function_index < module->functions.size ());
    Function* function = &module->functions [function_index];
    function->function_index = function_index; // TODO remove this
    Invoke (module->functions [function_index]);
}

INTERP (Block)
{
    // Label is end.
    StackValue stack_value (Tag_Label);
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
    StackValue stack_value (Tag_Label);
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

    while (j > 0 && p [j - 1].tag == Tag_Value)
        --j;

    // FIXME mark earlier if end of block or function
    // FIXME And then assert?

    Assert (j > 0 && (p [j - 1].tag == Tag_Label || p [j - 1].tag == Tag_Frame));

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
    while (!empty () && top ().tag != Tag_Frame)
        pop ();

    Assert (!empty ());
    Assert (top ().tag == Tag_Frame);
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
        while (j > 0 && p [j - 1].tag == Tag_Value)
        {
            initial_values += (i == 0);
            --j; // Pop_values.
        }
        Assert (j > 0 && p [j - 1].tag == Tag_Label);
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
#if !_MSC_VER || _MSC_VER > 900
static
#endif
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
#if !_MSC_VER || _MSC_VER > 900
static
#endif
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
#if !_MSC_VER || _MSC_VER > 900
static
#endif
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
