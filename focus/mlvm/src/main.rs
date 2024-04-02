use pest::Parser;
use pest_derive::Parser;

#[derive(Parser)]
#[grammar_inline = r#"
WHITESPACE = _{ " " | "\t" | "\n" }

alpha = _{ 'a'..'z' | 'A'..'Z' }
digit = _{ '0'..'9' }
ident = @{ alpha ~ (alpha | digit)* }

// keywords
let = { "let" }

// expr
term = { ident | "(" ~ expr ~ ")" }
expr = { term }

// statement
let_statement  = { let ~ ident ~ "="  ~ expr ~ ";" }
assgin_statement = { ident ~ "="  ~ expr ~ ";" }

// program
line = { let_statement | assgin_statement }
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
        .map(|pair| {
            (
                pair.as_str(),
                pair.as_rule(),
                pair.into_inner().next().unwrap().as_rule(),
            )
        })
        .collect::<Vec<(&str, Rule, Rule)>>();

    println!("lines total: {}", lines.len());

    for l in &lines {
        println!(
            "program line: {:?} / ({:<16}), {}",
            l.1,
            format!("{:?}", l.2),
            l.0
        );
    }
}
