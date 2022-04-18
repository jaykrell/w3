// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#include "w3.h"
#include "w3Handle.h"
#include "w3Fd.h"
#include "w3MemoryMappedFile.h"

MemoryMappedFile::MemoryMappedFile () :
    base (0), size (0)
{
}

MemoryMappedFile::~MemoryMappedFile ()
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

void MemoryMappedFile::read (PCSTR a)
{
#if _WIN32
    file = CreateFileA (a, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (!file) throw_GetLastError (StringFormat ("CreateFileA (%s)", a).c_str ());
    // FIXME check for size==0 and >4GB.
    size = (size_t)file.get_file_size (a);
    Handle h2 = CreateFileMappingW (file, 0, PAGE_READONLY, 0, 0, 0);
    if (!h2) throw_GetLastError (StringFormat ("CreateFileMapping (%s)", a).c_str ());
    base = MapViewOfFile (h2, FILE_MAP_READ, 0, 0, 0);
    if (!base)
        throw_GetLastError (StringFormat ("MapViewOfFile (%s)", a).c_str ());
#else
    file = open (a, O_RDONLY);
    if (!file) ThrowErrno (StringFormat ("open (%s)", a).c_str ());
    // FIXME check for size==0 and >4GB.
    size = (size_t)file.get_file_size (a);
    base = mmap (0, size, PROT_READ, MAP_PRIVATE, file, 0);
    if (base == MAP_FAILED)
        ThrowErrno (StringFormat ("mmap (%s)", a).c_str ());
#endif
}
