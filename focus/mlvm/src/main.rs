use pest::Parser;
use pest_derive::Parser;

#[derive(Parser)]
#[grammar_inline = r#"
WHITESPACE = _{ " " | "\t" | "\n" }

alpha = _{ 'a'..'z' | 'A'..'Z' }
digit = _{ '0'..'9' }
ident = @{ alpha ~ (alpha | digit)* }

expr = { ident }

line = { "let"? ~ ident ~ "="  ~ expr ~ ";" }
program_unit = { line+ }
program = _{ SOI ~ program_unit ~ EOI }
"#]
pub struct ProgramParser;

static SAMPLE: &str = "let a = b; \n c = a; c = b;\n ";

fn main() {
    let pairs = ProgramParser::parse(Rule::program, SAMPLE)
        .unwrap()
        .next()
        .unwrap()
        .into_inner();

    let lines = pairs
        .map(|pair| (pair.as_str(), pair.as_rule()))
        .collect::<Vec<(&str, Rule)>>();

    println!("lines total: {}", lines.len());

    for l in &lines {
        println!("program line: {:?}, {}", l.1, l.0);
    }
}
