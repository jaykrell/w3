#include <stdint.h>
#include <errno.h>
#if _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#define INVALID_HANDLE_VALUE ((void*)(ssize_t)-1)
#endif
#include <sys/stat.h>
#include <string.h>
#include <string>
#include <stdarg.h>

std::string
StringFormatVa (const char* format, va_list va);

std::string
StringFormat (const char* format, ...)
{
    va_list va;
    va_start (va, format);
    std::string a = StringFormatVa (format, va);
    va_end (va);
    return a;
}

// C++98 workaround for what C++11 offers.
struct explicit_operator_bool
{
    typedef void (explicit_operator_bool::*T) () const;
#if 1
    void True () const { } // some compilers require an implementation (Watcom)
#else
    void True () const;
#endif
};

typedef void (explicit_operator_bool::*bool_type) () const;

extern "C"
{

struct Fd
{
    Fd() : fd (-1) { }

    ~Fd()
    {
        static_cleanup (fd);
    }

    // TODO return error
    int64_t size ()
    {
#if __CYGWIN__ || _WIN32
        struct stat st; // TODO test more systems
        memset (&st, 0, sizeof (st));
        int result = fstat (fd, &st);
#else
        struct stat64 st; // TODO test more systems
        memset (&st, 0, sizeof (st));
        int result = fstat64 (fd, &st);
#endif
        return result ? -1 : st.st_size;
    }

    int fd;

#if 0 // C++11
    explicit operator bool () { return valid (); } // C++11
#else
    operator explicit_operator_bool::T () const
    {
        return valid () ? &explicit_operator_bool::True : NULL;
    }
#endif

    bool operator ! ()
    {
        return !valid ();
    }

    operator int ()
    {
        return get ();
    }

    int detach ()
    {
        int f = fd;
        fd = -1;
        return f;
    }

    static bool static_valid (int fd)
    {
        return fd > -1;
    }

    int get () const { return fd; }

    bool valid () const { return static_valid (fd); }

    int cleanup ()
    {
        int h = detach ();
        static_cleanup (h);
        return h;
    }

    static void static_cleanup (int fd)
    {
        if (fd >= 0)
#if _WIN32
            _close (fd);
#else
            close (fd);
#endif
    }
};

struct Handle
{
    Handle () : handle (0) { }

#if 0 // C++11
    explicit operator bool () { return valid (); } // C++11
#else
    operator explicit_operator_bool::T () const
    {
        return valid () ? &explicit_operator_bool::True : NULL;
    }
#endif
    ~Handle ()
    {
        cleanup ();
    }

    Handle (void* h) : handle (h) { }

    void operator = (void* h)
    {
        if (h != handle)
        {
            cleanup ();
            handle = h;
        }
    }

    operator void* () { return handle; }

    void* detach ()
    {
        void* h = handle;
        handle = 0;
        return h;
    }

    void* cleanup ()
    {
        void* h = detach ();
#if _WIN32
        // FIXME templatize
        if (static_valid (h))
            CloseHandle (h);
#endif
        return h;
    }

    bool valid () const { return static_valid (handle); }

    static bool static_valid (void* handle)
    {
        return handle && handle != INVALID_HANDLE_VALUE;
    }

    void* handle;
};

#if _WIN32
struct File : Handle
#else
struct File : Fd
#endif
{
#if _WIN32
    // TODO return error
    int64_t size ()
    {
        BY_HANDLE_FILE_INFORMATION info = { 0 };
        if (GetFileInformationByHandle (handle, &info))
            return (((int64_t)info.nFileSizeHigh) << 32) | info.nFileSizeLow;
        return -1;
    }
#else
    // inheritance
#endif
};

// TODO return error
int64_t File_size (File* file)
{
    return file->size ();
}

void File_cleanup (File* file)
{
	file->cleanup ();
}

struct MemoryMappedFile
{
// TODO allow for redirection to built-in data (i.e. filesystem emulation with builtin BCL)
// TODO allow for systems that must read, not mmap
    void* base;
    size_t size;
    File file;
    MemoryMappedFile () : base (0), size (0) { }

    ~MemoryMappedFile ()
    {
        cleanup ();
    }

    static int Error ()
    {
#if _WIN32
        const int err = GetLastError ();
#else
        const int err = errno;
#endif
        return (err < 0) ? err : -err;
    }

    void cleanup  ()
    {
        if (!base)
            return;
#if _WIN32
        UnmapViewOfFile (base);
#else
        munmap (base, size);
#endif
        base = 0;
    }

    int read (const char* a)
    {
#if _WIN32
#ifndef FILE_SHARE_DELETE // ifndef required due to Watcom 4 vs. 4L.
#define FILE_SHARE_DELETE 0x00000004 // missing in older headers
#endif
        file.handle = CreateFileA (a, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (!file)
            //throw_GetLastError (StringFormat ("CreateFileA (%s)", a).c_str ());
            return Error ();

        // FIXME check for size==0 and >4GB.
        size = (size_t)file.size ();
        Handle h2 = CreateFileMappingW (file, 0, PAGE_READONLY, 0, 0, 0);
        if (!h2)
            //throw_GetLastError (StringFormat ("CreateFileMapping (%s)", a).c_str ());
            return Error ();
        base = MapViewOfFile (h2, FILE_MAP_READ, 0, 0, 0);
        if (!base)
            //throw_GetLastError (StringFormat ("MapViewOfFile (%s)", a).c_str ());
            return Error ();
#else
        file.fd = open (a, O_RDONLY);
        if (!file)
            //ThrowErrno (StringFormat ("open (%s)", a).c_str ());
            return Error ();
        // FIXME check for size==0 and >4GB.
        size = (size_t)file.size ();
        base = mmap (0, size, PROT_READ, MAP_PRIVATE, file, 0);
        if (base == MAP_FAILED)
            //ThrowErrno (StringFormat ("mmap (%s)", a).c_str ());
        return Error ();
#endif
        return 0;
    }
};

}
