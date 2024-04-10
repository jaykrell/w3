// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

// integer | float, unary | binary | test | relation
#define IUNOP(b0, name, size)   INSTRUCTION (b0, 1, 0, name ## _i ## size, Imm_none, 1, 1, Tag_i ## size, Tag_none, Tag_none, Tag_i ## size)
#define FUNOP(b0, name, size)   INSTRUCTION (b0, 1, 0, name ## _f ## size, Imm_none, 1, 1, Tag_f ## size, Tag_none, Tag_none, Tag_f ## size)
#define IBINOP(b0, name, size)  INSTRUCTION (b0, 1, 0, name ## _i ## size, Imm_none, 2, 1, Tag_i ## size, Tag_i ## size, Tag_none, Tag_i ## size)
#define FBINOP(b0, name, size)  INSTRUCTION (b0, 1, 0, name ## _f ## size, Imm_none, 2, 1, Tag_f ## size, Tag_f ## size, Tag_none, Tag_f ## size)
#define ITESTOP(b0, name, size) INSTRUCTION (b0, 1, 0, name ## _i ## size, Imm_none, 1, 1, Tag_i ## size, Tag_none,     Tag_none, Tag_bool)
#define IRELOP(b0, name, size, sign)  INSTRUCTION (b0, 1, 0, name ## _i ## size ## sign, Imm_none, 2, 1, Tag_i ## size, Tag_i ## size, Tag_none, Tag_bool)
#define FRELOP(b0, name, size)  INSTRUCTION (b0, 1, 0, name ## _f ## size, Imm_none, 2, 1, Tag_f ## size, Tag_f ## size, Tag_none, Tag_bool)

// convert; TODO make ordering more sensible?
#define CVTOP(b0, name, to, from, sign) INSTRUCTION (b0, 1, 0, to ## _ ## name ## _ ## from ## sign, Imm_none, 1, 1, Tag_ ## from, Tag_none, Tag_none, Tag_ ## to)

#undef CONST
#define CONST(b0, type) INSTRUCTION (b0, 1, 0, type ## _Const, Imm_ ## type, 0, 1, Tag_none, Tag_none, Tag_none, Tag_ ## type)

#define LOAD(b0, to, from)  INSTRUCTION (b0, 1, 0, to ## _Load ## from,  Imm_memory, 0, 1, Tag_none,     Tag_none, Tag_none, Tag_ ## to)
#define STORE(b0, from, to) INSTRUCTION (b0, 1, 0, from ## _Store ## to, Imm_memory, 1, 0, Tag_ ## from, Tag_none, Tag_none, Tag_none)

#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) name,

#undef RESERVED
#define RESERVED(b0) INSTRUCTION (0x ## b0, 0, 0, Reserved ## b0, Imm_none, 0, 0, Tag_none, Tag_none, Tag_none, Tag_none)
