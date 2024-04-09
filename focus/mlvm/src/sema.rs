use crate::ast::{FnSig, Tree, Type};
use ctx::Context;
use error::{AnalysisError, AnalysisResult};
use pass::Pass;

mod decl_check;
mod type_check;

pub fn run(tree: &mut Tree) -> AnalysisResult<()> {
    let mut ctx = Context::default();
    ctx.fn_sigs.insert(
        "add".to_string(),
        FnSig {
            ret: Type::Tensor,
            args: vec![Type::Tensor, Type::Tensor],
        },
    );

    decl_check::new(&ctx).run(tree)?;
    type_check::new(&ctx).run(tree)?;
    Ok(())
}

mod pass {
    pub trait Pass<T> {
        fn run(&mut self, tree: &mut super::Tree) -> super::AnalysisResult<T>;
    }
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
