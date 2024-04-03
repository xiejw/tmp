use pest::iterators::Pair;
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
fn_call = { ident ~ "("  ~ (expr  ~ ( "," ~ expr)* )?  ~ ")" }
term = { fn_call | ident | "(" ~ expr ~ ")" }
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

static SAMPLE: &str = "let a = b; \n c = add(a, b); c = b;\n ";

fn main() {
    let pairs = ProgramParser::parse(Rule::program, SAMPLE)
        .unwrap()
        .next()
        .unwrap()
        .into_inner();

    for line in pairs {
        process_line(line);
    }
}

fn process_line(pair: Pair<Rule>) {
    let rule = &pair.as_rule();
    let str_str = &pair.as_str().to_string();
    let statement = pair.into_inner().next().unwrap();
    println!(
        "program line: {:?} / ({:<16}), {}",
        rule,
        format!("{:?}", statement.as_rule()),
        str_str
    );

    match statement.as_rule() {
        Rule::let_statement => {
            println!("-> see let");
        }
        Rule::assgin_statement => {
            println!("-> see assgin_statement");
        }
        _ => {
            unimplemented!();
        }
    }
}
