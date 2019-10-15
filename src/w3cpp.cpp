#include <stdint.h>
#include <errno.h>
#if _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif
#include <string.h>

extern "C" {

struct Fd
{
	int fd;
};

struct Handle
{
	void* handle;
};

struct File
{
#if _WIN32
	void* handle;
#else
	int fd;
#endif
};

// TODO return error
int64_t File_size (File* file)
{
#if _WIN32
	BY_HANDLE_FILE_INFORMATION info;
	if (GetFileInformationByHandle (file->handle, &info))
		return (((uint64_t)info.nFileSizeHigh) << 32) | info.nFileSizeLow;
	return 0;
#else
    int result = 0;
#if __CYGWIN__
    struct stat st; // TODO test more systems
    memset (&st, 0, sizeof (st));
    result = fstat (file->fd, &st);
#else
    struct stat64 st; // TODO test more systems
    memset (&st, 0, sizeof (st));
    result = fstat64 (file->fd, &st);
#endif
	if (result)
		return 0;
	else
		return st.st_size;
#endif
}

}