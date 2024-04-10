// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#ifdef _WIN32

struct w3Handle
{
    // TODO w3Handle vs. win32file_t, etc.

    uint64_t get_file_size (PCSTR file_name = "");

    void* h {};

    w3Handle (void* a);
    w3Handle ();

    void* get ();

    bool valid () const;

    static bool static_valid (void* h);

    operator void* ();

    static void static_cleanup (void* h);

    void* detach ();

    void cleanup ();

    w3Handle& operator= (void* a);

#if 0 // C++11
    explicit operator bool (); // C++11
#else
    operator explicit_operator_bool::T () const;
#endif

    bool operator ! ();

    ~w3Handle ();
};
#endif

