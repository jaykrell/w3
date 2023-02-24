cmake_minimum_required(VERSION 2.4)

project(w3)

add_executable(w3

w3Fd.cpp
w3Handle.cpp
w3InstructionEncoding.cpp
w3InstructionNames.cpp
w3Int.cpp
w3MemoryMappedFile.cpp
w3Module.cpp
w3rt.cpp
w3Unpack.cpp
isinf.c
isnan.c
s_floor.c
s_floorf.c
s_round.c
s_roundf.c
s_trunc.c
s_truncf.c

add.c
w3.cpp
#w3cgen.cpp # TODO: included by w3.cpp currently
#w3interp.cpp # TODO: included by w3.cpp currently
)
