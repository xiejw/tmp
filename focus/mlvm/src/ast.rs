use std::error::Error;

use crate::parser::Rule;

use pest::iterators::Pairs;

//
// basic data structure for the ast
//

type AstResult<'a, T> = Result<T, Box<dyn Error>>;

#[derive(Debug)]
pub struct Tree {
    pub statements: Vec<Statement>,
}

#[derive(Debug)]
pub enum Statement {
    Let(Box<LetStatement>),
    Assign(Box<AssignStatement>),
}

#[derive(Debug)]
pub struct LetStatement {
    pub ident: Ident,
    pub expr: Expr,
}

#[derive(Debug)]
pub struct AssignStatement {
    pub ident: Ident,
    pub expr: Expr,
}

#[derive(Debug)]
pub struct Ident {
    pub name: String,
}

#[derive(Debug)]
pub struct FnCall {
    pub name: String,
}

#[derive(Debug)]
pub enum Expr {
    FnCall(Box<FnCall>),
    Ident(Box<Ident>),
    Expr(Box<Expr>),
}

impl Tree {
    fn new() -> Tree {
        Tree {
            statements: Vec::new(),
        }
    }
}

//
// public api
//

pub fn build_ast(program: Pairs<Rule>) -> AstResult<Tree> {
    let mut tree = Tree::new();
    for line in program {
        tree.statements.push(internal::process_statement(line)?);
    }

    Ok(tree)
}

mod internal {
    use super::*;
    use crate::parser::Rule;

    use pest::iterators::Pair;

    pub fn process_statement(pair: Pair<Rule>) -> AstResult<Statement> {
        let statement = pair.into_inner().next().unwrap();
        Ok(match statement.as_rule() {
            Rule::let_statement => process_let_statement(statement)?,
            Rule::assgin_statement => process_assign_statement(statement)?,
            _ => unimplemented!(),
        })
    }

    fn process_let_statement(pair: Pair<Rule>) -> AstResult<Statement> {
        let mut pairs = pair.into_inner();
        Ok(Statement::Let(Box::new(LetStatement {
            ident: process_ident(pairs.next().unwrap())?,
            expr: process_expr(pairs.next().unwrap())?,
        })))
    }

    fn process_assign_statement(pair: Pair<Rule>) -> AstResult<Statement> {
        let mut pairs = pair.into_inner();
        Ok(Statement::Assign(Box::new(AssignStatement {
            ident: process_ident(pairs.next().unwrap())?,
            expr: process_expr(pairs.next().unwrap())?,
        })))
    }

    fn process_expr<'a>(pair: Pair<Rule>) -> AstResult<'a, Expr> {
        debug_assert!(Rule::expr == pair.as_rule());
        let expr = pair.into_inner().next().unwrap();
        match expr.as_rule() {
            Rule::fn_call => Ok(Expr::FnCall(Box::new(process_fncall(expr)?))),
            Rule::ident => Ok(Expr::Ident(Box::new(process_ident(expr)?))),
            Rule::expr => {
                unimplemented!();
            }
            _ => unimplemented!(),
        }
    }

    fn process_ident<'a>(pair: Pair<Rule>) -> AstResult<'a, Ident> {
        debug_assert!(Rule::ident == pair.as_rule());
        Ok(Ident {
            name: pair.as_str().to_string(),
        })
    }

    fn process_fncall<'a>(pair: Pair<Rule>) -> AstResult<'a, FnCall> {
        debug_assert!(Rule::fn_call == pair.as_rule());

        let mut pairs = pair.into_inner();
        Ok(FnCall {
            name: pairs.next().unwrap().as_str().to_string(),
        })
    }
}
