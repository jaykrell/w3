// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

//#include <vector> TODO: namespace
#include "w3StackValue.h"

// TODO consider a vector instead, but it affects frame.locals staying valid across push/pop
//typedef std::deque <w3StackValue> w3StackBaseBase;
typedef std::vector <w3StackValue> w3StackBaseBase;
