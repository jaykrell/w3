// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#include "w3.h"
#include "w3Handle.h"

#ifdef _WIN32

// TODO w3Handle vs. Win32File etc.

uint64_t w3Handle::get_file_size (PCSTR file_name)
{
    DWORD hi = 0;
    DWORD lo = GetFileSize (h, &hi);
    if (lo == INVALID_FILE_SIZE)
    {
        DWORD err = GetLastError ();
        if (err != NO_ERROR)
            throw_Win32Error ((int)err, StringFormat ("GetFileSize (%s)", file_name).c_str ());
    }
    return (((uint64_t)hi) << 32) | lo;
}

w3Handle::w3Handle (void* a) : h (a)
{
}

w3Handle::w3Handle () : h (0)
{
}

void* w3Handle::get ()
{
    return h;
}

bool w3Handle::valid () const
{
    return static_valid (h);
}

bool w3Handle::static_valid (void* h)
{
    return h && h != INVALID_HANDLE_VALUE;
}

w3Handle::operator void* ()
{
    return get ();
}

void w3Handle::static_cleanup (void* h)
{
    if (static_valid (h))
        CloseHandle (h);
}

void* w3Handle::detach ()
{
    void* const a = h;
    h = 0;
    return a;
}

void w3Handle::cleanup ()
{
    static_cleanup (detach ());
}

w3Handle& w3Handle::operator= (void* a)
{
    if (h == a) return *this;
    cleanup ();
    h = a;
    return *this;
}

#if 0 // C++11

w3Handle::operator bool ()
{
    return valid ();
}

#else

w3Handle::operator explicit_operator_bool::T () const
{
    return valid () ? &explicit_operator_bool::True : NULL;
}

#endif

bool w3Handle::operator ! () { return !valid (); }

w3Handle::~w3Handle ()
{
    if (valid ()) CloseHandle (h);
    h = 0;
}

#endif

