#include <sys/stat.h>
#include <stdint.h>

extern "C" {

#if !_WIN32
int w3cpp_fstat_size (int fd, uint64* size)
{
    int result;
#if __CYGWIN__
    struct stat st = { 0 }; // TODO test more systems
    result = fstat (fd, &st);
#else
    struct stat64 st = { 0 }; // TODO test more systems
    result = fstat64 (fd, &st);
#endif
    *size = st.st_size;
    return result;
}
#endif

}