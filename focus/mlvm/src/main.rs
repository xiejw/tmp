pub mod ast;
pub mod parser;

static SAMPLE: &str = "let a = b; \n c = add(a, b); c = b;\n ";

fn main() {
    let pairs = parser::parse(SAMPLE).unwrap();
    ast::build_ast(pairs).unwrap();
}
