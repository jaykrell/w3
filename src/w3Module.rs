// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

extern crate libc;
use std::io;
use std::io::{BufReader, Read, ErrorKind};
use std::fs::File;
use std::error::Error;

macro_rules! trace {
    () => {
        println!("{}({})", file!(), line!());
    };
}

pub struct T {
    reader: Option<BufReader<File>>,
    file_path: String,
}

impl Default for T {
    fn default() -> T {
        T {
            reader: None,
            file_path: String::new(),
        }
    }
}

impl T {
    fn read_byte(&mut self) -> io::Result<u64> {
        let mut buffer = [0; 1];
        self.reader.as_mut().unwrap().buffer().read_exact(&mut buffer)?;
        Ok(buffer[0] as u64)
    }

    fn read_varuint7 (&mut self) -> io::Result<u64> {
        let result = self.read_byte ();
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
        self.read_varuint64()
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

    fn read_section (&mut self) -> Result<(), Box<dyn Error>> {
        let id = self.read_varuint7()?;
        trace!();
        if id > 11 {
            return Err(Box::<dyn Error>::from(format!("malformed file:{} section-id:{}", self.file_path, id)));
        }
        trace!();
        return Err(Box::<dyn Error>::from(""));

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

    (this->*section_traits [id].read) (&payload);

    if (payload != *cursor)
        ThrowString (StringFormat ("failed to read section:%X payload:%p cursor:%p\n", id, payload, *cursor));
    */
    }

    fn read_module (&mut self, file_path: String) -> io::Result<()> {
        self.file_path = file_path;
        let file = File::open(&self.file_path)?;
        let file_size = file.metadata()?.len();
        if file_size < 8 {
            return Err(io::Error::new(ErrorKind::InvalidData, format!("Wasm too small {} {}", self.file_path, file_size)));
        }
        let mut reader = io::BufReader::new(file);
        let mut buf = [0; 4];
        reader.buffer().read_exact(&mut buf)?;
        let magic = T::u32le(&buf);
        reader.buffer().read_exact(&mut buf)?;
        let version = T::u32le(&buf);
        let expected_magic = T::u32le(&[0, 'a' as u8, 's' as u8, 'm' as u8]); // "\0wasm"
        if magic != expected_magic {
            return Err(io::Error::new(ErrorKind::InvalidData, format!("Wasm incorrect magic: {} {}", self.file_path, magic)));
        }
        if version != 1 {
            return Err(io::Error::new(ErrorKind::InvalidData, format!("Wasm incorrect version: {} {}", self.file_path, version)));
        }
        self.reader = Some(reader);
        if file_size == 8 {
            // Valid module with no sections.
        } else {
        /*
            uint8_t* cursor = base + 8;
            while (cursor < end)
                read_section (&cursor);

            Assert (cursor == end);
        */
        }
        Ok(())
    }
}
