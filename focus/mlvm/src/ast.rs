use std::error::Error;

use crate::parser::Rule;

use pest::iterators::Pair;
use pest::iterators::Pairs;

type AstResult<'a> = Result<(), Box<dyn Error>>;

fn process_let_statement(_pair: Pair<Rule>) {
    println!("-> see let");
}

fn process_line(pair: Pair<Rule>) -> AstResult {
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
            process_let_statement(statement);
        }
        Rule::assgin_statement => {
            println!("-> see assgin_statement");
        }
        _ => {
            unimplemented!();
        }
    }
    Ok(())
}

pub fn build_ast(program: Pairs<Rule>) -> AstResult {
    for line in program {
        process_line(line)?;
    }

    Ok(())
}
