use error::{AnalysisError, AnalysisResult};

mod var_def;

pub trait Pass<T> {
    fn run(&mut self, tree: &mut super::ast::Tree) -> AnalysisResult<T>;
}

mod error {
    pub type AnalysisResult<T> = Result<T, Box<AnalysisError>>;

    pub struct AnalysisError {
        pub cause: String,
    }

    impl AnalysisError {
        pub fn new(cause: String) -> Box<AnalysisError> {
            Box::new(AnalysisError { cause })
        }
    }
}
