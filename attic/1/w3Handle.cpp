// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#include "w3.h"
#include "w3Handle.h"

#ifdef _WIN32

// TODO Handle vs. Win32File etc.

uint64_t Handle::get_file_size (PCSTR file_name)
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

Handle::Handle (void* a) : h (a)
{
}

Handle::Handle () : h (0)
{
}

void* Handle::get ()
{
    return h;
}

bool Handle::valid () const
{
    return static_valid (h);
}

bool Handle::static_valid (void* h)
{
    return h && h != INVALID_HANDLE_VALUE;
}

Handle::operator void* ()
{
    return get ();
}

void Handle::static_cleanup (void* h)
{
    if (static_valid (h))
        CloseHandle (h);
}

void* Handle::detach ()
{
    void* const a = h;
    h = 0;
    return a;
}

void Handle::cleanup ()
{
    static_cleanup (detach ());
}

Handle& Handle::operator= (void* a)
{
    if (h == a) return *this;
    cleanup ();
    h = a;
    return *this;
}

#if 0 // C++11

Handle::operator bool ()
{
    return valid ();
}

#else

Handle::operator explicit_operator_bool::T () const
{
    return valid () ? &explicit_operator_bool::True : NULL;
}

#endif

bool Handle::operator ! () { return !valid (); }

Handle::~Handle ()
{
    if (valid ()) CloseHandle (h);
    h = 0;
}

#endif

