// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

struct w3ModuleInstance;
struct w3Module;
struct w3Interp;
struct w3StackValue;
struct w3Code;

struct w3Frame
{
    // FUTURE spec return_arity
    size_t function_index; // replace with pointer?
    w3ModuleInstance* module_instance;
    w3Module* module;
//    w3Frame* next; // TODO remove this; it is on stack
    w3Code* code;
    size_t param_count;
    size_t local_only_count;
    size_t param_and_local_count;
    w3Tag* local_only_types;
    w3Tag* param_types;
    w3FunctionType* function_type;
    // TODO locals/params
    // This should just be stack pointer, to another stack,
    // along with type information (module->module->locals_types[])

    w3Interp* interp;
    size_t locals; // index in stack to start of params and locals, params first

    w3Frame ()
    {
        ZeroMem(this, sizeof(*this));
    }

    w3StackValue& Local (size_t index);
};
