// A WebAssembly codebase by Jay Krell

#pragma once

#ifdef _WIN32

struct Handle
{
    // TODO Handle vs. win32file_t, etc.

    uint64_t get_file_size (PCSTR file_name = "");

    void* h;

    Handle (void* a);
    Handle ();

    void* get ();

    bool valid () const;

    static bool static_valid (void* h);

    operator void* ();

    static void static_cleanup (void* h);

    void* detach ();

    void cleanup ();

    Handle& operator= (void* a);

#if 0 // C++11
    explicit operator bool (); // C++11
#else
    operator explicit_operator_bool::T () const;
#endif

    bool operator ! ();

    ~Handle ();
};
#endif

