// A WebAssembly codebase by Jay Krell

#pragma once

struct Fd
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

    Fd (int a = -1);

    Fd& operator = (int a);

    ~Fd ();
};
