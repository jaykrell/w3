// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#![allow(dead_code)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(non_upper_case_globals)]
#![allow(unused_mut)]
#![allow(unused_imports)]

extern crate libc;
use std::env;
use std::mem;
use std::fs::File;
use std::io::Read;
mod w3Module;

// #[macro_use] extern crate educe;

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
    Memory,     // align:u32 offset:u32
    Type,       // read_varuint32
    Function,   // read_varuint32
    Global,     // read_varuint32
    Local,      // read_varuint32
    Label,      // read_varuint32
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
//include!(concat!(env!("OUT_DIR"), "/wasm_instructions.rs"));

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
pub struct w3File
{
#[cfg(windows)]
    handle: Handle,
#[cfg(not(windows))]
    fd: Fd
// FIXME drop
}

extern "C" {
    pub fn File_Size (file: &w3File) -> i64;
    pub fn File_Cleanup (file: &w3File);
}

impl w3File {
    fn size (self:&w3File) -> i64
    {
        unsafe {
            File_Size (self)
        }
    }
    fn drop (self:&w3File)
    {
        unsafe {
            File_Cleanup (self)
        }
    }
}

#[cfg(windows)]
#[allow(non_snake_case)]
pub mod windows {
    mod kernel32 {
        #[link(name = "kernel32")]
        extern "C" {
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

    println!("{}", std::mem::size_of::<w3File>());

    Xd! (123);
    Xd! (0x123);
    // Xs!
    // Xx!

    let mut args: Vec<String> = env::args().collect();
    let mut file_path = mem::take(&mut args[1]);
    println!("file_path:{}", file_path);
    //let mut file = File::open(file_path).unwrap();

    let mut module = w3Module::T::read_module(file_path);
    match module {
        Err(e) => { println!("Error reading module {}", e) }
        Ok(_) => todo!()
    }
    //let byte = module.read_byte();
    //println!("j:{} k:{}", module.j, module.k);
}
