pub mod ast;
pub mod codegen;
pub mod parser;

static SAMPLE: &str = "let a = @weights.b; \n c = add(a, (b)); c = b;\n ";

fn main() {
    println!("===== PROGRAM =====");
    println!("```\n{}\n```\n", SAMPLE);
    let pairs = parser::parse(SAMPLE).unwrap();

    let tree = ast::build_ast(pairs).unwrap();
    // println!("===== AST     =====");
    // DEBUG: println!("{:#?}", tree);

    println!("===== CODEGEN =====");
    codegen::compile(&tree).unwrap();
}
