use std::env;
use std::fs::File;
use std::io::Write;
use std::path::Path;
extern crate cc;

fn main() {
	let out_dir = env::var("OUT_DIR").unwrap();
	let dest_path = Path::new(&out_dir).join("wasm_instructions.rs");
	let mut f = File::create(&dest_path).unwrap();

	let c = &mut cc::Build::new();
	c.flag("-I.").file("src/wasm_instructions.h");
    
	// Preprocess w/o line numbers
	// -EP for Visual C++
	// -E -P for gcc
	//
	// TODO Compiler detection.
	//
	// Nothing works, so remove line directives ourselves.

	for s in String::from_utf8(c.expand()).unwrap().lines() {
		let bytes = s.as_bytes();
		if bytes.len() > 0 && bytes [0] != b'#' {
			f.write(bytes).unwrap();
			f.write(b"\n").unwrap();
		}
	}
}

/*
The .h file looks like this:

#ifdef FOO

FOO(...)
FOO(...)

#else

#undef FOO
#define FOO(...) ...
#include __FILE__


#undef FOO
#define FOO(...) ...
#include __FILE__

#endif
*/
