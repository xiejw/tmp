pub mod ast;
pub mod codegen;
pub mod parser;
pub mod sema;

static SAMPLE: &str = "let a = @weights.b; \n let c = add(a, (a)); let b = c;\n ";

fn main() {
    println!("===== PROGRAM =====");
    println!("```\n{}\n```\n", SAMPLE);
    let pairs = parser::parse(SAMPLE).unwrap();

    println!("===== AST     =====");
    let mut tree = ast::build_ast(pairs).unwrap();
    // DEBUG: println!("{:#?}", tree);

    println!("===== SEMA    =====");
    sema::run(&mut tree).unwrap();
    // DEBUG: println!("{:#?}", tree);

    println!("===== CODEGEN =====");
    codegen::compile(&tree).unwrap();
}
