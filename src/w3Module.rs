// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

extern crate libc;
use std::io;
use std::fmt;
use std::io::{BufReader, Read, Error, ErrorKind};
use std::fs::File;

pub struct T {
    reader: Option<BufReader<File>>,
}

impl Default for T {
    fn default() -> T {
        T {
            reader: None,
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

    fn read_module (&mut self, file_path: &String) -> io::Result<()> {
        let file = File::open(file_path)?;
        let file_size = file.metadata()?.len();
        if file_size < 8 {
            return Err(io::Error::new(ErrorKind::InvalidData, format!("Wasm too small {} {}", file_path, file_size)));
        }
        let mut reader = io::BufReader::new(file);
        let mut buf = [0; 4];
        reader.buffer().read_exact(&mut buf)?;
        let magic = T::u32le(&buf);
        reader.buffer().read_exact(&mut buf)?;
        let version = T::u32le(&buf);
        let expected_magic = T::u32le(&[0, 'a' as u8, 's' as u8, 'm' as u8]); // "\0wasm"
        if magic != expected_magic {
            return Err(io::Error::new(ErrorKind::InvalidData, format!("Wasm incorrect magic: {} {}", file_path, magic)));
        }
        if version != 1 {
            return Err(io::Error::new(ErrorKind::InvalidData, format!("Wasm incorrect version: {} {}", file_path, version)));
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
