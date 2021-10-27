// A WebAssembly implementation and experimentation platform.
// portable
// simple? Always striving for the right level of complexity -- not too simple.
// efficient? (not yet)

INSTRUCTION (0x00, 1, 0, Unreach,   Imm_none,     0, 0, Type_none, Type_none, Type_none, Type_none)
INSTRUCTION (0x01, 1, 0, Nop,       Imm_none,     0, 0, Type_none, Type_none, Type_none, Type_none)
INSTRUCTION (0x02, 1, 0, Block,     Imm_sequence, 0, 0, Type_none, Type_none, Type_none, Type_none)
INSTRUCTION (0x03, 1, 0, Loop,      Imm_sequence, 0, 0, Type_none, Type_none, Type_none, Type_none)
INSTRUCTION (0x04, 1, 0, If,        Imm_sequence, 0, 0, Type_none, Type_none, Type_none, Type_none)
/* Else is kind of Imm_sequence but treated custom along with if. */
INSTRUCTION (0x05, 1, 0, Else,      Imm_none, 0, 0, Type_none, Type_none, Type_none, Type_none)

RESERVED (06)
RESERVED (07)
RESERVED (08)
RESERVED (09)
RESERVED (0A)

INSTRUCTION (0x0B, 1, 0, BlockEnd,  Imm_none,       0, 0, Type_none, Type_none, Type_none, Type_none)
INSTRUCTION (0x0C, 1, 0, Br,        Imm_label,      0, 0, Type_none, Type_none, Type_none, Type_none)
INSTRUCTION (0x0D, 1, 0, BrIf,      Imm_label,      0, 0, Type_none, Type_none, Type_none, Type_none)
INSTRUCTION (0x0E, 1, 0, BrTable,   Imm_vecLabel,   0, 0, Type_none, Type_none, Type_none, Type_none)
INSTRUCTION (0x0F, 1, 0, Ret,       Imm_none,       0, 0, Type_none, Type_none, Type_none, Type_none)
INSTRUCTION (0x10, 1, 0, Call,      Imm_function,   0, 0, Type_none, Type_none, Type_none, Type_none)
INSTRUCTION (0x11, 1, 0, Calli,     Imm_type,       0, 0, Type_none, Type_none, Type_none, Type_none)

RESERVED (12)
RESERVED (13)
RESERVED (14)
RESERVED (15)
RESERVED (16)
RESERVED (17)
RESERVED (18)
RESERVED (19)

INSTRUCTION (0x1A, 1, 0, Drop,       Imm_none, 1, 0, Type_any, Type_none, Type_none, Type_none)
INSTRUCTION (0x1B, 1, 0, Select,     Imm_none, 3, 1, Type_any, Type_any, Type_bool, Type_any)

RESERVED (1C)
RESERVED (1D)
RESERVED (1E)
RESERVED (1F)

INSTRUCTION (0x20, 1, 0, Local_get,  Imm_local,  0, 1, Type_none, Type_none, Type_none, Type_any)
INSTRUCTION (0x21, 1, 0, Local_set,  Imm_local,  1, 0, Type_any,  Type_none, Type_none, Type_none)
INSTRUCTION (0x22, 1, 0, Local_tee,  Imm_local,  1, 1, Type_any,  Type_none, Type_none, Type_any)
INSTRUCTION (0x23, 1, 0, Global_get, Imm_global, 0, 1, Type_none, Type_none, Type_none, Type_any)
INSTRUCTION (0x24, 1, 0, Global_set, Imm_global, 1, 0, Type_any,  Type_none, Type_none, Type_none)

RESERVED (25)
RESERVED (26)
RESERVED (27)

LOAD (0x28, i32, _)
LOAD (0x29, i64, _)
LOAD (0x2A, f32, _)
LOAD (0x2B, f64, _)

/* zero or sign extending load from memory to register */
LOAD (0x2C, i32, 8s)
LOAD (0x2D, i32, 8u)
LOAD (0x2E, i32, 16s)
LOAD (0x2F, i32, 16u)
LOAD (0x30, i64, 8s)
LOAD (0x31, i64, 8u)
LOAD (0x32, i64, 16s)
LOAD (0x33, i64, 16u)
LOAD (0x34, i64, 32s)
LOAD (0x35, i64, 32u)

STORE (0x36, i32, _)
STORE (0x37, i64, _)
STORE (0x38, f32, _)
STORE (0x39, f64, _)

/* truncating store from register to memory */
STORE (0x3A, i32, 8)
STORE (0x3B, i32, 16)
STORE (0x3C, i64, 8)
STORE (0x3D, i64, 16)
STORE (0x3E, i64, 32)

INSTRUCTION (0x3F, 2, 0, MemSize, Imm_none, 1, 0, Type_none, Type_none, Type_none, Type_i32)
INSTRUCTION (0x40, 2, 0, MemGrow, Imm_none, 1, 1, Type_i32,  Type_none, Type_none, Type_i32)

CONST (0x41, i32)
CONST (0x42, i64)
CONST (0x43, f32)
CONST (0x44, f64)

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

 IUNOP (0x67, Clz,     32)
 IUNOP (0x68, Ctz,     32)
 IUNOP (0x69, Popcnt,  32)
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

 IUNOP (0x79, Clz,     64)
 IUNOP (0x7A, Ctz,     64)
 IUNOP (0x7B, Popcnt,  64)
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

CVTOP (0xA7,  Wrap, i32, i64, _)
CVTOP (0xA8, Trunc, i32, f32, s)
CVTOP (0xA9, Trunc, i32, f32, u)
CVTOP (0xAA, Trunc, i32, f64, s)
CVTOP (0xAB, Trunc, i32, f64, u)
CVTOP (0xAC, Extend, i64, i32, s)
CVTOP (0xAD, Extend, i64, i32, u)
CVTOP (0xAE, Trunc, i64, f32, s)
CVTOP (0xAF, Trunc, i64, f32, u)
CVTOP (0xB0, Trunc, i64, f64, s)
CVTOP (0xB1, Trunc, i64, f64, u)

CVTOP (0xB2, Convert, f32, i32, u)
CVTOP (0xB3, Convert, f32, i32, s)
CVTOP (0xB4, Convert, f32, i64, u)
CVTOP (0xB5, Convert, f32, i64, s)
CVTOP (0xB6, Demote,  f32, f64, _)
CVTOP (0xB7, Convert, f64, i32, s)
CVTOP (0xB8, Convert, f64, i32, u)
CVTOP (0xB9, Convert, f64, i64, s)
CVTOP (0xBA, Convert, f64, i64, u)
CVTOP (0xBB, Promote, f64, f32, _)

CVTOP (0xBC, Reinterpret, i32, f32, _)
CVTOP (0xBD, Reinterpret, i64, f64, _)
CVTOP (0xBE, Reinterpret, f32, i32, _)
CVTOP (0xBF, Reinterpret, f64, i64, _)

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

