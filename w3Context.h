// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

struct w3Context_t;
typedef struct w3Context_t w3Context_t;

struct w3Error_t;
typedef struct w3Error_t w3Error_t;

struct w3Error_t
{
    int code;
    int file;
    int line;
    char* message;
    int detail[4];
};

struct w3Context_t
{
    w3Error_t error;
};
