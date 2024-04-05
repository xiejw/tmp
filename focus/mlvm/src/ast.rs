use std::error::Error;

use pest::iterators::Pairs;

use crate::parser::Rule;

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
    pub args: Vec<Box<Expr>>,
}

#[derive(Debug)]
pub struct PathLookup {
    pub paths: Vec<Box<Ident>>,
}

#[derive(Debug)]
pub enum Expr {
    FnCall(Box<FnCall>),
    Ident(Box<Ident>),
    PathLookup(Box<PathLookup>),
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
            Rule::expr => Ok(Expr::Expr(Box::new(process_expr(expr)?))),
            Rule::path_lookup => Ok(Expr::PathLookup(Box::new(process_path_lookup(expr)?))),
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
        let mut r = FnCall {
            name: pairs.next().unwrap().as_str().to_string(),
            args: Vec::new(),
        };

        for arg in pairs {
            r.args.push(Box::new(process_expr(arg)?));
        }

        Ok(r)
    }

    fn process_path_lookup<'a>(pair: Pair<Rule>) -> AstResult<'a, PathLookup> {
        debug_assert!(Rule::path_lookup == pair.as_rule());

        let pairs = pair.into_inner();
        let r = PathLookup {
            paths: pairs
                .map(|arg| Box::new(process_ident(arg).unwrap()))
                .collect(),
        };
        debug_assert!(!r.paths.is_empty());
        Ok(r)
    }
}
