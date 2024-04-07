// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

extern crate libc;
use std::io;
use std::io::{BufReader, Read, ErrorKind};
use std::fs::File;
use std::error::Error;
use std::convert::TryFrom;

struct SectionKind;
impl SectionKind {
    const custom     : u64 = 0;
    const types      : u64 = 1;
    const import     : u64 = 2;
    const function   : u64 = 3;
    const table      : u64 = 4;
    const memory     : u64 = 5;
    const global     : u64 = 6;
    const export     : u64 = 7;
    const start      : u64 = 8;
    const element    : u64 = 9;
    const code       : u64 = 10;
    const data       : u64 = 11;
    const data_count : u64 = 12;
}

#[derive(Debug, Clone)] // Derive the Clone trait
enum Tag {
    none = 0, // allow for zero-init
    bool = 1, // aka i32
    any  = 2, // often has constraints

    // These are from the file format. These are the primary
    // types in WebAssembly, prior to the addition of SIMD.
    i32 = 0x7F,
    i64 = 0x7E,
    f32 = 0x7D,
    f64 = 0x7C,

    //todo: comment
    empty = 0x40, // ResultType, void

    //internal, any value works..not clearly needed
    // A heterogenous conceptual WebAssembly stack contains Values, Frames, and Labels.
    Value = 0x80, // i32, i64, f32, f64
    Label = 0x81, // branch target
    Frame = 0x82, // return address + locals + params

    //todo: comment
    FuncRef = 0x70,

    //codegen temp
    //intptr = 3,
    //uintptr = 4,
    //pch = 5,
}

// TODO
//WasmError { }

macro_rules! trace {
    () => {
        println!("{}({})", file!(), line!());
    };
}

#[derive(Default)]
#[derive(Clone)]
struct FunctionType {
    // CONSIDER pointer into memory mapped file
    parameters: std::vec::Vec<Tag>,
    results: std::vec::Vec<Tag>,

/*    bool operator == (const FunctionType& other) const
    {
        return parameters == other.parameters && results == other.results;
    }*/
}

impl FunctionType {
    fn new() -> Self {
        Default::default()
    }
}

pub struct T {
    reader: BufReader<File>,
    file_path: String,
    file_size: i64,
    function_type: std::vec::Vec<FunctionType>,
    offset: i64
}

impl T {
    fn read_byte(&mut self) -> io::Result<u64> {
        //trace!();
        let mut buffer = [0; 1];
        //trace!();
        self.reader.buffer().read_exact(&mut buffer)?;
        self.offset += 1;
        //trace!();
        Ok(buffer[0] as u64)
    }

    fn read_varuint7 (&mut self) -> io::Result<u64> {
        //trace!();
        let result = self.read_byte ();
        //trace!();
        match result {
            Ok(i) => {
                if (i & 0x80) != 0 {
                    return Err(io::Error::new(ErrorKind::InvalidData, format!("invalid {}", i)));
                }
                result
            },
            _ => result
        }
    }

    fn read_varuint32 (&mut self) -> io::Result<u64> {
        self.read_varuint64 ()
    }

    fn read_varuint64 (&mut self) -> io::Result<u64> {
        let mut result: u64 = 0;
        let mut shift: u32 = 0;
        loop {
            let byte = self.read_byte().unwrap();
            result |= ((byte & 0x7F) as u64) << shift;
            if (byte & 0x80) == 0 {
                return Ok(result);
            }
            shift += 7;
        }
    }

    fn read_varint (&mut self, size: u32) -> io::Result<i64> {
        let mut result: i64 = 0;
        let mut shift: u32 = 0;
        let mut byte: u64;
        loop {
            byte = self.read_byte().unwrap();
            result |= ((byte & 0x7F) as i64) << shift;
            shift += 7;
            if (byte & 0x80) == 0 {
                break;
            }
        }

        // sign bit of byte is second high order bit (0x40)
        if shift < size && (byte & 0x40) != 0 {
            result |= !0 << shift; // sign extend
        }

        Ok(result)
    }

    fn read_varint32 (&mut self) -> io::Result<i64> {
        self.read_varint(64)
    }

    fn read_varint64 (&mut self) -> io::Result<i64> {
        self.read_varint(64)
    }

    // array of 4 bytes to little endian u32
    fn u32le(a: &[u8]) -> u32 {
        let mut b: u32 = 0;
        b |= a[3] as u32; b <<= 8;
        b |= a[2] as u32; b <<= 8;
        b |= a[1] as u32; b <<= 8;
        b |= a[0] as u32;
        b
    }

    fn read_section_custom (&mut self) -> Result<(), Box<dyn Error>> {
        println! ("reading custom 0");
        todo!();
    }

    fn read_valuetype (&mut self) -> Result<Tag, Box<dyn Error>> {
        let value_type = self.read_byte ()? as u8;
        const a: u8 = Tag::i32 as u8;
        const b: u8 = Tag::i64 as u8;
        const c: u8 = Tag::f32 as u8;
        const d: u8 = Tag::f64 as u8;
        match value_type {
            a => return Ok(Tag::i32),
            b => return Ok(Tag::i64),
            c => return Ok(Tag::i32),
            d => return Ok(Tag::f64),
            _ => return Err(Box::<dyn Error>::from(format!("invalid Tag:{}", value_type)))
        }
    }

    fn read_vector_ValueType (&mut self) -> Result<std::vec::Vec<Tag>, Box<dyn Error>> {
        println! ("read_vector_ValueType1 offset:{}", self.offset);
        let size = self.read_varuint32 ().unwrap () as usize;
        let mut result = vec![Tag::none; size];
        for i in 0..size {
            result [i] = self.read_valuetype ().unwrap ();
        }
        println! ("read_vector_ValueType2 offset:{}", self.offset);
		Ok(result)
    }

    fn read_function_type (&mut self, i: usize) -> Result<(), Box<dyn Error>> {
        println! ("reading read_function_type i:{} offset:{}", i, self.offset);
		let parameters = self.read_vector_ValueType ()?;
        println! ("reading read_function_type i:{} offset:{}", i, self.offset);
		let results = self.read_vector_ValueType ()?;
        println! ("reading read_function_type i:{} offset:{}", i, self.offset);
	    self.function_type[i] = FunctionType {parameters: parameters, results: results};
        println! ("read_function_type ok i:{} offset:{}", i, self.offset);
		Ok(())
    }

    fn read_section_types (&mut self) -> Result<(), Box<dyn Error>> {
        println! ("read_section_types1 offset:{}", self.offset);
        let size = self.read_varuint32 ()? as usize;
        println! ("read_section_types2 offset:{} size:{}", self.offset, size);
        //trace!();
        self.function_type.resize(size, FunctionType::new());
        //trace!();
        for i in 0..size {
            //trace!();
            let marker = self.read_byte ()?;
            //trace!();
            if marker != 0x60 {
              println! ("read_section_types4 offset:{} size:{} marker:{}", self.offset, size, marker);
              return Err(Box::<dyn Error>::from(format!("malformed2 in Types::read {} {}", self.file_path, marker)));
            }
            trace!();
            self.read_function_type (i)?;
            trace!();
        }
        trace!();
        println! ("read_section_types3 ok id:1 offset:{}", self.offset);
        Ok(())
    }

    fn read_section (&mut self) -> Result<(), Box<dyn Error>> {
        let id = self.read_varuint7 ()?;
        println! ("reading section id:{} offset:{}", id, self.offset);
        //trace!();
        if id > 11 {
            //trace!();
            return Err(Box::<dyn Error>::from(format!("malformed file:{} section-id:{}", self.file_path, id)));
        }
        //trace!();
        match id {
            SectionKind::custom => self.read_section_custom (),
            SectionKind::types => self.read_section_types (),
            _ =>
              return Err(Box::new(io::Error::new(ErrorKind::InvalidData, format!("Wasm unknown section kind: {} {}", self.file_path, id)))),
        }
        /*
    const size_t payload_size = read_varuint32 (cursor);
    printf ("%s payload_size:%" FORMAT_SIZE "X\n", __func__, (long_t)payload_size);
    payload = *cursor;
    uint32_t name_size = 0;
    PCH local_name = 0;
    if (id == 0)
    {
        name_size = read_varuint32 (cursor);
        local_name = (PCH)*cursor;
        if (local_name + name_size > (PCH)end)
            ThrowString (StringFormat ("malformed %d", __LINE__)); // UNDONE context (move to module or section)
    }
    if (payload + payload_size > end)
        ThrowString (StringFormat ("malformed line:%d id:%X payload:%p payload_size:%" FORMAT_SIZE "X base:%p end:%p", __LINE__, id, payload, (long_t)payload_size, base, end)); // UNDONE context

    printf("%s(%d)\n", __FILE__, __LINE__);

    *cursor = payload + payload_size;

    if (id == 0)
    {
        if (name_size < INT_MAX)
            printf ("skipping custom section:.%.*s\n", (int)name_size, local_name);
        // UNDONE custom sections
        return;
    }

    printf("%s(%d)\n", __FILE__, __LINE__);

    Section& section = sections [id];
    section.id = id;
    section.name.data = local_name;
    section.name.size = name_size;
    section.payload_size = payload_size;
    section.payload = payload;

    printf("%s(%d) %d\n", __FILE__, __LINE__, (int)id);
    //DebugBreak ();

    (self->*section_traits [id].read) (&payload);

    if (payload != *cursor)
        ThrowString (StringFormat ("failed to read section:%X payload:%p cursor:%p\n", id, payload, *cursor));
    */
    }

    pub fn read_module (file_path: String) -> io::Result<T> {
        let file = File::open(&file_path).unwrap();
        let file_size = file.metadata().unwrap().len() as i64;
        println! ("file_size:{}", file_size);
        if file_size < 8 {
            return Err(io::Error::new(ErrorKind::InvalidData, format!("Wasm too small {} {}", &file_path, file_size)));
        }
        let mut this = T {
            reader: io::BufReader::new(file),
            file_path,
            file_size,
            function_type: std::vec::Vec::<FunctionType>::new(),
            offset: 0 };
        let mut buf = [0; 4];
        this.reader.read_exact(&mut buf)?;
        let magic = T::u32le(&buf);
        this.reader.buffer().read_exact(&mut buf)?;
        let version = T::u32le(&buf);
        let expected_magic = T::u32le(&[0, 'a' as u8, 's' as u8, 'm' as u8]); // "\0asm"
        if magic != expected_magic {
            return Err(io::Error::new(ErrorKind::InvalidData, format!("Wasm incorrect magic: {} {}", this.file_path, magic)));
        }
        if version != 1 {
            return Err(io::Error::new(ErrorKind::InvalidData, format!("Wasm incorrect version: {} {}", this.file_path, version)));
        }
        if this.file_size == 8 {
            // Valid module with no sections.
            println! ("no sections");
        } else {
            while this.offset < this.file_size {
                let _ = this.read_section ();
            }
            //assert_eq!(this.offset == this.file_size, "");
        }
        Ok(this)
    }
}
