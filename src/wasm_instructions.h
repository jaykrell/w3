// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

// Rust (C preprocesssor'ed, granted)

#ifdef INSTRUCTION

INSTRUCTION (0x00, 1, 0, Unreach,   Imm::None,     0, 0, Type::None, Type::None, Type::None, Type::None)
INSTRUCTION (0x01, 1, 0, Nop,       Imm::None,     0, 0, Type::None, Type::None, Type::None, Type::None)
INSTRUCTION (0x02, 1, 0, Block,     Imm::Sequence, 0, 0, Type::None, Type::None, Type::None, Type::None)
INSTRUCTION (0x03, 1, 0, Loop,      Imm::Sequence, 0, 0, Type::None, Type::None, Type::None, Type::None)
INSTRUCTION (0x04, 1, 0, If,        Imm::Sequence, 0, 0, Type::None, Type::None, Type::None, Type::None)
/* Else is kind of Imm::Sequence but treated custom along with if. */
INSTRUCTION (0x05, 1, 0, Else,      Imm::None, 0, 0, Type::None, Type::None, Type::None, Type::None)

RESERVED (06)
RESERVED (07)
RESERVED (08)
RESERVED (09)
RESERVED (0A)

INSTRUCTION (0x0B, 1, 0, BlockEnd,  Imm::None,       0, 0, Type::None, Type::None, Type::None, Type::None)
INSTRUCTION (0x0C, 1, 0, Br,        Imm::Label,      0, 0, Type::None, Type::None, Type::None, Type::None)
INSTRUCTION (0x0D, 1, 0, BrIf,      Imm::Label,      0, 0, Type::None, Type::None, Type::None, Type::None)
INSTRUCTION (0x0E, 1, 0, BrTable,   Imm::VecLabel,   0, 0, Type::None, Type::None, Type::None, Type::None)
INSTRUCTION (0x0F, 1, 0, Ret,       Imm::None,       0, 0, Type::None, Type::None, Type::None, Type::None)
INSTRUCTION (0x10, 1, 0, Call,      Imm::Function,   0, 0, Type::None, Type::None, Type::None, Type::None)
INSTRUCTION (0x11, 1, 0, Calli,     Imm::Type,       0, 0, Type::None, Type::None, Type::None, Type::None)

RESERVED (12)
RESERVED (13)
RESERVED (14)
RESERVED (15)
RESERVED (16)
RESERVED (17)
RESERVED (18)
RESERVED (19)

INSTRUCTION (0x1A, 1, 0, Drop,       Imm::None, 1, 0, Type::Any, Type::None, Type::None, Type::None)
INSTRUCTION (0x1B, 1, 0, Select,     Imm::None, 3, 1, Type::Any, Type::Any, Type::Bool, Type::Any)

RESERVED (1C)
RESERVED (1D)
RESERVED (1E)
RESERVED (1F)

INSTRUCTION (0x20, 1, 0, Local_get,  Imm::Local,  0, 1, Type::None, Type::None, Type::None, Type::Any)
INSTRUCTION (0x21, 1, 0, Local_set,  Imm::Local,  1, 0, Type::Any,  Type::None, Type::None, Type::None)
INSTRUCTION (0x22, 1, 0, Local_tee,  Imm::Local,  1, 1, Type::Any,  Type::None, Type::None, Type::Any)
INSTRUCTION (0x23, 1, 0, Global_get, Imm::Global, 0, 1, Type::None, Type::None, Type::None, Type::Any)
INSTRUCTION (0x24, 1, 0, Global_set, Imm::Global, 1, 0, Type::Any,  Type::None, Type::None, Type::None)

RESERVED (25)
RESERVED (26)
RESERVED (27)

LOAD (0x28, I32, _)
LOAD (0x29, I64, _)
LOAD (0x2A, F32, _)
LOAD (0x2B, F64, _)

/* zero or sign extending load from memory to register */
LOAD (0x2C, I32, 8s)
LOAD (0x2D, I32, 8u)
LOAD (0x2E, I32, 16s)
LOAD (0x2F, I32, 16u)
LOAD (0x30, I64, 8s)
LOAD (0x31, I64, 8u)
LOAD (0x32, I64, 16s)
LOAD (0x33, I64, 16u)
LOAD (0x34, I64, 32s)
LOAD (0x35, I64, 32u)

STORE (0x36, I32, _)
STORE (0x37, I64, _)
STORE (0x38, F32, _)
STORE (0x39, F64, _)

/* truncating store from register to memory */
STORE (0x3A, I32, 8)
STORE (0x3B, I32, 16)
STORE (0x3C, I64, 8)
STORE (0x3D, I64, 16)
STORE (0x3E, I64, 32)

INSTRUCTION (0x3F, 2, 0, MemSize, Imm::None, 1, 0, Type::None, Type::None, Type::None, Type::I32)
INSTRUCTION (0x40, 2, 0, MemGrow, Imm::None, 1, 1, Type::I32,  Type::None, Type::None, Type::I32)

CONST (0x41, I32)
CONST (0x42, I64)
CONST (0x43, F32)
CONST (0x44, F64)

ITESTOP (0x45, Eqz, 32)
IRELOP (0x46, Eq, 32, _)
IRELOP (0x47, Ne, 32, _)
IRELOP (0x48, Lt, 32, s)
IRELOP (0x49, Lt, 32, u)
IRELOP (0x4A, Gt, 32, s)
IRELOP (0x4B, Gt, 32, u)
IRELOP (0x4C, Le, 32, s)
IRELOP (0x4D, Le, 32, u)
IRELOP (0x4E, Ge, 32, s)
IRELOP (0x4F, Ge, 32, u)

ITESTOP (0x50, Eqz, 64)
IRELOP (0x51, Eq, 64, _)
IRELOP (0x52, Ne, 64, _)
IRELOP (0x53, Lt, 64, s)
IRELOP (0x54, Lt, 64, u)
IRELOP (0x55, Gt, 64, s)
IRELOP (0x56, Gt, 64, u)
IRELOP (0x57, Le, 64, s)
IRELOP (0x58, Le, 64, u)
IRELOP (0x59, Ge, 64, s)
IRELOP (0x5A, Ge, 64, u)

FRELOP (0x5B, Eq, 32)
FRELOP (0x5C, Ne, 32)
FRELOP (0x5D, Lt, 32)
FRELOP (0x5E, Gt, 32)
FRELOP (0x5F, Le, 32)
FRELOP (0x60, Ge, 32)

FRELOP (0x61, Eq, 64)
FRELOP (0x62, Ne, 64)
FRELOP (0x63, Lt, 64)
FRELOP (0x64, Gt, 64)
FRELOP (0x65, Le, 64)
FRELOP (0x66, Ge, 64)

 IUNOP (0x67, CountLeadingZeros,  32)
 IUNOP (0x68, CountTrailingZeros, 32)
 IUNOP (0x69, CountSetBits,  32)
IBINOP (0x6A, Add,     32)
IBINOP (0x6B, Sub,     32)
IBINOP (0x6C, Mul,     32)
IBINOP (0x6D, Div_s,   32)
IBINOP (0x6E, Div_u,   32)
IBINOP (0x6F, Rem_s,   32)
IBINOP (0x70, Rem_u,   32)
IBINOP (0x71, And,     32)
IBINOP (0x72, Or,      32)
IBINOP (0x73, Xor,     32)
IBINOP (0x74, Shl,     32)
IBINOP (0x75, Shr_s,   32)
IBINOP (0x76, Shr_u,   32)
IBINOP (0x77, Rotl,    32)
IBINOP (0x78, Rotr,    32)

 IUNOP (0x79, CountLeadingZeros,  64)
 IUNOP (0x7A, CountTrailingZeros, 64)
 IUNOP (0x7B, CountSetBits, 64)
IBINOP (0x7C, Add,     64)
IBINOP (0x7D, Sub,     64)
IBINOP (0x7E, Mul,     64)
IBINOP (0x7F, Div_s,   64)
IBINOP (0x80, Div_u,   64)
IBINOP (0x81, Rem_s,   64)
IBINOP (0x82, Rem_u,   64)
IBINOP (0x83, And,     64)
IBINOP (0x84, Or,      64)
IBINOP (0x85, Xor,     64)
IBINOP (0x86, Shl,     64)
IBINOP (0x87, Shr_s,   64)
IBINOP (0x88, Shr_u,   64)
IBINOP (0x89, Rotl,    64)
IBINOP (0x8A, Rotr,    64)

 FUNOP (0x8B, Abs,      32)
 FUNOP (0x8C, Neg,      32)
 FUNOP (0x8D, Ceil,     32)
 FUNOP (0x8E, Floor,    32)
 FUNOP (0x8F, Trunc,    32)
 FUNOP (0x90, Nearest,  32)
 FUNOP (0x91, Sqrt,     32)
FBINOP (0x92, Add,      32)
FBINOP (0x93, Sub,      32)
FBINOP (0x94, Mul,      32)
FBINOP (0x95, Div,      32)
FBINOP (0x96, Min,      32)
FBINOP (0x97, Max,      32)
FBINOP (0x98, Copysign, 32)

 FUNOP (0x99, Abs,      64)
 FUNOP (0x9A, Neg,      64)
 FUNOP (0x9B, Ceil,     64)
 FUNOP (0x9C, Floor,    64)
 FUNOP (0x9D, Trunc,    64)
 FUNOP (0x9E, Nearest,  64)
 FUNOP (0x9F, Sqrt,     64)
FBINOP (0xA0, Add,      64)
FBINOP (0xA1, Sub,      64)
FBINOP (0xA2, Mul,      64)
FBINOP (0xA3, Div,      64)
FBINOP (0xA4, Min,      64)
FBINOP (0xA5, Max,      64)
FBINOP (0xA6, Copysign, 64)

CVTOP (0xA7,  Wrap, I32, I64, _)
CVTOP (0xA8, Trunc, I32, F32, s)
CVTOP (0xA9, Trunc, I32, F32, u)
CVTOP (0xAA, Trunc, I32, F64, s)
CVTOP (0xAB, Trunc, I32, F64, u)
CVTOP (0xAC, Extend, I64, I32, s)
CVTOP (0xAD, Extend, I64, I32, u)
CVTOP (0xAE, Trunc, I64, F32, s)
CVTOP (0xAF, Trunc, I64, F32, u)
CVTOP (0xB0, Trunc, I64, F64, s)
CVTOP (0xB1, Trunc, I64, F64, u)

CVTOP (0xB2, Convert, F32, I32, u)
CVTOP (0xB3, Convert, F32, I32, s)
CVTOP (0xB4, Convert, F32, I64, u)
CVTOP (0xB5, Convert, F32, I64, s)
CVTOP (0xB6, Demote,  F32, F64, _)
CVTOP (0xB7, Convert, F64, I32, s)
CVTOP (0xB8, Convert, F64, I32, u)
CVTOP (0xB9, Convert, F64, I64, s)
CVTOP (0xBA, Convert, F64, I64, u)
CVTOP (0xBB, Promote, F64, F32, _)

CVTOP (0xBC, Reinterpret, I32, F32, _)
CVTOP (0xBD, Reinterpret, I64, F64, _)
CVTOP (0xBE, Reinterpret, F32, I32, _)
CVTOP (0xBF, Reinterpret, F64, I64, _)

RESERVED (C0)
RESERVED (C1)
RESERVED (C2)
RESERVED (C3)
RESERVED (C4)
RESERVED (C5)
RESERVED (C6)
RESERVED (C7)
RESERVED (C8)
RESERVED (C9)
RESERVED (CA)
RESERVED (CB)
RESERVED (CC)
RESERVED (CD)
RESERVED (CE)
RESERVED (CF)

RESERVED (D0)
RESERVED (D1)
RESERVED (D2)
RESERVED (D3)
RESERVED (D4)
RESERVED (D5)
RESERVED (D6)
RESERVED (D7)
RESERVED (D8)
RESERVED (D9)
RESERVED (DA)
RESERVED (DB)
RESERVED (DC)
RESERVED (DD)
RESERVED (DE)
RESERVED (DF)

RESERVED (E0)
RESERVED (E1)
RESERVED (E2)
RESERVED (E3)
RESERVED (E4)
RESERVED (E5)
RESERVED (E6)
RESERVED (E7)
RESERVED (E8)
RESERVED (E9)
RESERVED (EA)
RESERVED (EB)
RESERVED (EC)
RESERVED (ED)
RESERVED (EE)
RESERVED (EF)

RESERVED (F0)
RESERVED (F1)
RESERVED (F2)
RESERVED (F3)
RESERVED (F4)
RESERVED (F5)
RESERVED (F6)
RESERVED (F7)
RESERVED (F8)
RESERVED (F9)
RESERVED (FA)
RESERVED (FB)
RESERVED (FC)
RESERVED (FD)
RESERVED (FE)
RESERVED (FF)

#else

// integer | float, unary | binary | test | relation
#define IUNOP(b0, name, size)   INSTRUCTION (b0, 1, 0, name ## _i ## size, Imm::None, 1, 1, Type::I ## size, Type::None, Type::None, Type::I ## size)
#define FUNOP(b0, name, size)   INSTRUCTION (b0, 1, 0, name ## _f ## size, Imm::None, 1, 1, Type::F ## size, Type::None, Type::None, Type::F ## size)
#define IBINOP(b0, name, size)  INSTRUCTION (b0, 1, 0, name ## _i ## size, Imm::None, 2, 1, Type::I ## size, Type::I ## size, Type::None, Type::I ## size)
#define FBINOP(b0, name, size)  INSTRUCTION (b0, 1, 0, name ## _f ## size, Imm::None, 2, 1, Type::F ## size, Type::F ## size, Type::None, Type::F ## size)
#define ITESTOP(b0, name, size) INSTRUCTION (b0, 1, 0, name ## _i ## size, Imm::None, 1, 1, Type::I ## size, Type::None,     Type::None, Type::Bool)
#define FRELOP(b0, name, size)  INSTRUCTION (b0, 1, 0, name ## _f ## size, Imm::None, 2, 1, Type::F ## size, Type::F ## size, Type::None, Type::Bool)
#define IRELOP(b0, name, size, sign)  INSTRUCTION (b0, 1, 0, name ## _i ## size ## sign, Imm::None, 2, 1, Type::I ## size, Type::I ## size, Type::None, Type::Bool)

// convert; TODO make ordering more sensible?
#define CVTOP(b0, name, to, from, sign) INSTRUCTION (b0, 1, 0, to ## _ ## name ## _ ## from ## sign, Imm::None, 1, 1, Type:: from, Type::None, Type::None, Type :: to)

#define RESERVED(b0) INSTRUCTION (0x ## b0, 0, 0, Reserved ## b0, Imm::None, 0, 0, Type::None, Type::None, Type::None, Type::None)

#undef CONST
#define CONST(b0, type) INSTRUCTION (b0, 1, 0, type ## _Const, Imm :: type, 0, 1, Type::None, Type::None, Type::None, Type :: type)

#define LOAD(b0, to, from)  INSTRUCTION (b0, 1, 0, to ## _Load ## from,  Imm::Memory, 0, 1, Type::None,     Type::None, Type::None, Type :: to)
#define STORE(b0, from, to) INSTRUCTION (b0, 1, 0, from ## _Store ## to, Imm::Memory, 1, 0, Type :: from, Type::None, Type::None, Type::None)

#undef INSTRUCTION
#define INSTRUCTION(byte0, fixed_size, byte1, name, imm, push, pop, in0, in1, in2, out0) name,

// TODO
//#[repr(u8)]
enum InstructionEnum
{
#include __FILE__
}

// TODO put this in .rs.
// In the C+ version, the instruction names are all concatenated into one string,
// and the offset computed and stored.
//
// For now Rust will forgo that optimization.
// Though it appears not difficult.

struct InstructionEncoding
{
// In the C++ version these are bitfields.
// For now Rust will forgo that optimization.
	byte0: u8,
	fixed_size: u8,
	imm: Imm,
	pop: u8,
	push: u8,
	name: InstructionEnum,
	stack_in0: Type,
	stack_in1: Type,
	stack_in2: Type,
	stack_out0: Type,
	str: &'static str,
// TODO void (*interp) (Module*); // Module* wrong
}

#undef INSTRUCTION
#define INSTRUCTION(xbyte0, xfixed_size, xbyte1, xname, ximm, xpop, xpush, xin0, xin1, xin2, xout0)	\
InstructionEncoding{                \
    byte0: xbyte0,                  \
    fixed_size: xfixed_size,        \
    imm: ximm,                      \
    pop: xpop,                      \
    push: xpush,                    \
    name: InstructionEnum::xname,   \
    stack_in0: xin0,                \
    stack_in1: xin1,                \
    stack_in2: xin2,                \
    stack_out0: xout0,              \
    str: #xname \
},
const instructionEncode : [ InstructionEncoding ; 256 ] = [
#include __FILE__
];

#endif
