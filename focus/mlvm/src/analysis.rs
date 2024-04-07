// Loop until all types are found.
// Crash if the iteration cannot make progress.
// Verify signatures
//

use crate::ast::Tree;

mod var_def;

pub struct AnalysisError {
    pub cause: String,
}

impl AnalysisError {
    fn new(cause: String) -> Box<AnalysisError> {
        Box::new(AnalysisError { cause })
    }
}

type AnalysisResult<T> = Result<T, Box<AnalysisError>>;

pub trait Pass<T> {
    fn run(&mut self, tree: &mut Tree) -> AnalysisResult<T>;
}
