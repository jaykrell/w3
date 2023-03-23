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
    fn get_byte(&mut self) -> io::Result<u64> {
		let mut buffer = [0; 1];
		self.reader.as_mut().unwrap().buffer().read_exact(&mut buffer)?;
		Ok(buffer[0] as u64)
	}

    fn get_varuint7 (&mut self) -> io::Result<u64> {
		let result = self.get_byte ();
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

    fn get_varuint32 (&mut self) -> io::Result<u64> {
		self.get_varuint64()
    }

    fn get_varuint64 (&mut self) -> io::Result<u64> {
		let mut result: u64 = 0;
		let mut shift: u32 = 0;
		loop {
			let byte = self.get_byte().unwrap();
			result |= ((byte & 0x7F) as u64) << shift;
			if (byte & 0x80) == 0 {
				return Ok(result);
			}
			shift += 7;
		}
	}

    fn get_varint (&mut self, size: u32) -> io::Result<i64> {
		let mut result: i64 = 0;
		let mut shift: u32 = 0;
		let mut byte: u64;
		loop {
			byte = self.get_byte().unwrap();
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

    fn get_varint32 (&mut self) -> io::Result<i64> {
		self.get_varint(64)
    }

    fn get_varint64 (&mut self) -> io::Result<i64> {
		self.get_varint(64)
    }
}
