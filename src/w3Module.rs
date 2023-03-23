// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

use std::io;
use std::fmt;
use std::io::{BufReader, Read, Error, ErrorKind};

pub struct T {
	reader: Option<BufReader<u8>>,
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

	fn read_module (_file_name: String)
	{
	/*
		mmf.read (file_name);
		base = (uint8_t*)mmf.base;
		file_size = mmf.file.get_file_size ();
		end = file_size + (uint8_t*)base;

		if (file_size < 8)
			ThrowString (StringFormat ("too small %s", file_name));

		uintLE32& magic = (uintLE32&)*base;
		uintLE32& version = (uintLE32&)*(base + 4);
		printf ("magic: %X\n", (uint32_t)magic);
		printf ("version: %X\n", (uint32_t)version);

		if (memcmp (&magic,"\0asm", 4))
			ThrowString (StringFormat ("incorrect magic: %X", (uint32_t)magic));

		if (version != 1)
			ThrowString (StringFormat ("incorrect version: %X", (uint32_t)version));

		// Valid module with no sections.
		if (file_size == 8)
			return;

		uint8_t* cursor = base + 8;
		while (cursor < end)
			read_section (&cursor);

		Assert (cursor == end);
	*/
	}
}
