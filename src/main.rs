// A WebAssembly implementation and experimentation platform.
// Full spec TBD.

#![allow(dead_code)]
#![allow(non_camel_case_types)]
#![allow(non_upper_case_globals)]

extern crate libc;

#[repr(u8)]
enum Imm // Immediate
{
  None = 0,
  I32,
  I64,
  F32,
  F64,
  Sequence,
  VecLabel,
  //u32,
  Memory,    // align:u32 offset:u32
  Type,      // read_varuint32
  Function,  // read_varuint32
  Global,    // read_varuint32
  Local,	 // read_varuint32
  Label,	 // read_varuint32
}

#[repr(u8)]
enum Type
{
  None,
  Bool, // i32
  Any, // often has some constraints
  I32 = 0x7F,
  I64 = 0x7E,
  F32 = 0x7D,
  F64 = 0x7C,
}

// TODO put this in .rs.
//struct InstructionEncoding
//enum InstructionEnum;
include!(concat!(env!("OUT_DIR"), "/wasm_instructions.rs"));

const hugef: f32 = 1.0e30f32;
const huged: f64 = 1.0e300f64;

// This should probabably be combined with ResultType, and called Tag.
#[repr(u8)]
enum ValueType
{
    I32 = 0x7F,
    I64 = 0x7E,
    F32 = 0x7D,
    F64 = 0x7C,
}

#[repr(C)]
struct Fd
{
    fd: i32
}

#[repr(C)]
struct Handle
{
    handle: *mut libc::c_void
}

#[repr(C)]
pub struct File
{
#[cfg(windows)]
	handle: Handle,
#[cfg(not(windows))]
	fd: Fd
// FIXME drop
}

extern "C" {
    pub fn File_Size (file: &File) -> i64;
    pub fn File_Cleanup (file: &File);
}

impl File {
	fn size (self:&File) -> i64
	{
		unsafe {
			File_Size (self)
		}
	}
	fn drop (self:&File)
	{
		unsafe {
			File_Cleanup (self)
		}
	}
}

#[cfg(windows)]
#[allow(non_snake_case)]
pub mod windows {
	#[link(name = "kernel32")]
	mod kernel32 {
		extern {
			pub fn IsDebuggerPresent () -> u32;
			pub fn DebugBreak();
		}
	}
	pub fn IsDebuggerPresent() -> bool {
		unsafe { kernel32::IsDebuggerPresent () != 0 }
	}
	pub fn DebugBreak() {
		unsafe { kernel32::DebugBreak() }
	}
}

#[cfg(windows)]
use windows::*;

#[cfg(not(windows))]
#[allow(non_snake_case)]
mod posix {
	pub fn IsDebuggerPresent() -> bool
	{
		false // TODO
	}
	pub fn DebugBreak() {
		// TODO
	}
}

#[cfg(not(windows))]
use posix::*;

use std::env;

macro_rules! Xd {
	($x:expr) => {
		println!("{} {}", stringify!($x), $x);
	}
}

#[allow(unused_variables)]
fn main()
{
	if IsDebuggerPresent () { DebugBreak () }

	let argv = env::args();
	let mut argc = 0;
	for _ in argv {
		argc += 1
	}

	println!("{}", std::mem::size_of::<File>());

	Xd! (123);
	Xd! (0x123);
	// Xs!
	// Xx!
}
