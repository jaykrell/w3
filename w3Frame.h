// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

struct ModuleInstance;
struct Module;
struct Interp;
struct StackValue;
struct Code;

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
