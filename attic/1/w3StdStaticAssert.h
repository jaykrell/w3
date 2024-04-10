// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#if defined (_WIN32) && defined (C_ASSERT) // older compiler
#define static_assert(x, y) C_ASSERT (x)
#endif
