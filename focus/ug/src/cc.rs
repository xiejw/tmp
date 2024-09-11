//------------------------------------------------------------------------------
// Define a TrivialCompiler

#[derive(Debug)]
pub struct Error {}

pub trait Compiler {
    fn compile(&mut self, program: &str) -> Result<(), Error>;
}

pub fn new_compiler() -> impl Compiler {
    trivial::TrivialCompiler {}
}

mod trivial {
    use crate::cc::{Compiler, Error};

    //------------------------------------------------------------------------------
    // Define a TrivialCompiler

    pub struct TrivialCompiler {}

    impl Compiler for TrivialCompiler {
        fn compile(&mut self, program: &str) -> Result<(), Error> {
            for c in program.chars() {
                println!(
                    "{:width$} a /{:5}/ d /{:5}/ s /{:5}/",
                    if c.is_whitespace() { ' ' } else { c },
                    c.is_ascii_alphabetic(),
                    c.is_ascii_digit(),
                    c.is_whitespace(),
                    width = match c.len_utf8() {
                        1 => 2,
                        2 | 3 => 1,
                        0 => 2,
                        _ => panic!(),
                    }
                );
            }
            return Ok(());
        }
    }
}
