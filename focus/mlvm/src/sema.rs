use crate::ast::{FnSig, Type};
pub use ctx::Context;
use error::{AnalysisError, AnalysisResult};

mod decl_check;
mod type_check;

pub trait Pass<T> {
    fn run(&mut self, tree: &mut super::ast::Tree) -> AnalysisResult<T>;
}

pub fn run(tree: &mut super::ast::Tree) -> AnalysisResult<()> {
    let mut ctx = Context::default();
    ctx.fn_sigs.insert(
        "add".to_string(),
        FnSig {
            ret: Type::Tensor,
            args: vec![Type::Tensor, Type::Tensor],
        },
    );
    let mut pass = decl_check::new(&ctx);
    pass.run(tree)
}

mod error {
    pub type AnalysisResult<T> = Result<T, Box<AnalysisError>>;

    #[derive(Debug)]
    pub struct AnalysisError {
        pub cause: String,
    }

    impl AnalysisError {
        pub fn new(cause: String) -> Box<AnalysisError> {
            Box::new(AnalysisError { cause })
        }
    }
}

mod ctx {
    use crate::ast::FnSig;
    use std::collections::HashMap;

    #[derive(Default)]
    pub struct Context {
        pub fn_sigs: HashMap<String, FnSig>,
    }
}
