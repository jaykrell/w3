#include <stdint.h>
#include <errno.h>
#if _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

extern "C" {

struct Fd
{
	union {
		int64_t i64; // consistent size across all platforms
		int fd; // posix
		HANDLE handle; // win32
	};
};

int w3cpp_fstat_size (Fd* fd, uint64_t* size)
{
#if _WIN32
	BY_HANDLE_FILE_INFORMATION info;
	if (GetFileInformationByHandle (fd->handle, &info))
	{
		*size = (((uint64_t)info.nFileSizeHigh) << 32) | info.nFileSizeLow;
		return 0;
	}
	*size = 0;
	return GetLastError();
}
#else
    int result = 0;
#if __CYGWIN__
    struct stat st = { 0 }; // TODO test more systems
    result = fstat (fd->fd, &st);
#else
    struct stat64 st = { 0 }; // TODO test more systems
    result = fstat64 (fd->fd, &st);
#endif
	if (result)
	{
		*size = 0;
		return errno;
	}
	else
	{
	    *size = st.st_size;
		return 0;
	}
#endif
}

