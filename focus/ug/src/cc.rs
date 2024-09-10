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
                println!("{c}");
            }
            return Ok(());
        }
    }
}
