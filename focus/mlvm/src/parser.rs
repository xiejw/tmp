use std::error::Error;

use pest::iterators::Pairs;
use pest::Parser;
use pest_derive::Parser;

type ParseResult<'a> = Result<Pairs<'a, Rule>, Box<dyn Error>>;

pub fn parse(program_str: &str) -> ParseResult {
    let pairs = ProgramParser::parse(Rule::program, program_str)
        .unwrap()
        .next()
        .unwrap()
        .into_inner();
    Ok(pairs)
}

#[derive(Parser)]
#[grammar_inline = r#"
WHITESPACE = _{ " " | "\t" | "\n" }

alpha = _{ 'a'..'z' | 'A'..'Z' }
digit = _{ '0'..'9' }
ident = @{ alpha ~ (alpha | digit)* }

// keywords
let = _{ "let" }

// expr
fn_call = { ident ~ "("  ~ (expr  ~ ( "," ~ expr)* )?  ~ ")" }
expr = { fn_call | ident | "(" ~ expr ~ ")" }

// statement
let_statement  = { let ~ ident ~ "="  ~ expr ~ ";" }
assgin_statement = { ident ~ "="  ~ expr ~ ";" }

// program
line = { let_statement | assgin_statement }
program_unit = { line+ }
program = _{ SOI ~ program_unit ~ EOI }
"#]
pub struct ProgramParser;
