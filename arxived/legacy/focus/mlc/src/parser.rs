use pest::error::LineColLocation;
use pest::iterators::Pairs;
use pest::Parser;
use pest_derive::Parser;
use std::error::Error;

type ParseResult<'a> = Result<Pairs<'a, Rule>, Box<dyn Error>>;

pub fn parse(program_str: &str) -> ParseResult {
    let mut r = ProgramParser::parse(Rule::program, program_str);
    match r {
        Ok(ref mut r) => Ok(r.next().unwrap().into_inner()),

        Err(ref e) => {
            dbg!(&e.line_col);
            if let LineColLocation::Pos((line, c)) = e.line_col {
                let l = program_str.lines().nth(line - 1).unwrap();
                println!("parsing error");
                println!("```");
                println!("line {:<4}| {}", line, l);
                print!("           ");
                for _ in 0..c {
                    print!(" ");
                }
                println!("^");
                println!("```");
            }
            Err(Box::new(e.clone()))
        }
    }
}

#[derive(Parser)]
#[grammar_inline = r#"
WHITESPACE = _{ " " | "\t" | "\n" }

alpha = _{ 'a'..'z' | 'A'..'Z' }
digit = _{ '0'..'9' }
ident = @{ alpha ~ (alpha | digit)* }
path_lookup = { "@" ~ (ident ~ "." )* ~ ident }

// keywords
let = _{ "let" }

// expr
fn_call = { ident ~ "("  ~ (expr  ~ ( "," ~ expr)* )?  ~ ")" }
expr = { fn_call | ident | path_lookup | "(" ~ expr ~ ")" }

// statement
let_statement  = { let ~ ident ~ "="  ~ expr ~ ";" }
assgin_statement = { ident ~ "="  ~ expr ~ ";" }

// program
line = { let_statement | assgin_statement | expr ~ ";" }
program_unit = { line+ }
program = _{ SOI ~ program_unit ~ EOI }
"#]
struct ProgramParser;
