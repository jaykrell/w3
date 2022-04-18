// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#include "w3.h"
#include "w3Fd.h"

#ifndef _WIN32

uint64_t Fd::get_file_size (PCSTR file_name)
{
#if __CYGWIN__
    struct stat st = { 0 }; // TODO test more systems
    if (fstat (fd, &st))
#else
    struct stat64 st = { 0 }; // TODO test more systems
    if (fstat64 (fd, &st))
#endif
        ThrowErrno (StringFormat ("fstat (%s)", file_name).c_str ());
    return st.st_size;
}
#endif

#if 0 // C++11

Fd::operator bool ()
{
    return valid ();
}

#else

Fd::operator explicit_operator_bool::T () const
{
    return valid () ? &explicit_operator_bool::True : 0;
}

#endif

bool Fd::operator ! ()
{
    return !valid ();
}

Fd::operator int ()
{
    return get ();
}

bool Fd::static_valid (int fd)
{
    return fd != -1;
}

int Fd::get () const
{
    return fd;
}

bool Fd::valid () const
{
    return static_valid (fd);
}

void Fd::static_cleanup (int fd)
{
    if (!static_valid (fd))
        return;
#if _WIN32
    _close (fd);
#else
    close (fd);
#endif
}

int Fd::detach ()
{
    int const a = fd;
    fd = -1;
    return a;
}

void Fd::cleanup ()
{
    static_cleanup (detach ());
}

Fd::Fd (int a) : fd (a)
{
}

Fd& Fd::operator = (int a)
{
    if (fd == a) return *this;
    cleanup ();
    fd = a;
    return *this;
}

Fd::~Fd ()
{
    cleanup ();
}
