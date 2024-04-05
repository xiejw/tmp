pub mod ast;
pub mod parser;

static SAMPLE: &str = "let a = @weights.b; \n c = add(a, (b)); c = b;\n ";

fn main() {
    println!("program is ```\n{}\n```\n", SAMPLE);
    let pairs = parser::parse(SAMPLE).unwrap();
    let tree = ast::build_ast(pairs).unwrap();
    println!("{:#?}", tree);
}
