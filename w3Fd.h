// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

struct w3Fd
{
    int fd;

    uint64_t get_file_size (PCSTR file_name = "");

#if 0 // C++11
    explicit operator bool () { return valid (); } // C++11
#else
    operator explicit_operator_bool::T () const;
#endif

    bool operator ! ();

    operator int ();
    static bool static_valid (int fd);
    int get () const;
    bool valid () const;

    static void static_cleanup (int fd);

    int detach ();

    void cleanup ();

    w3Fd (int a = -1);

    w3Fd& operator = (int a);

    ~w3Fd ();
};
