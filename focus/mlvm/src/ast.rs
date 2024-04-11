use crate::parser::Rule;
use pest::iterators::Pairs;
use std::error::Error;

type AstResult<T> = Result<T, Box<dyn Error>>;

//
// type
//

#[derive(Debug, Copy, Clone, PartialEq)]
pub enum Type {
    Tensor,
    Path,
}

pub struct FnSig {
    pub ret: Type,
    pub args: Vec<Type>,
}

//
// basic data structure for the ast
//

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
    pub tp: Option<Type>,
}

#[derive(Debug)]
pub struct FnCall {
    pub name: String,
    pub args: Vec<Box<Expr>>,
    pub tp: Option<Type>,
}

#[derive(Debug)]
pub struct PathLookup {
    pub paths: Vec<Ident>,
    pub tp: Option<Type>,
}

#[derive(Debug)]
pub enum Expr {
    FnCall(Box<FnCall>),
    Ident(Box<Ident>),
    PathLookup(Box<PathLookup>),
    Expr(Box<Expr>),
}

impl Tree {
    fn new() -> Self {
        Tree {
            statements: Vec::new(),
        }
    }
}

impl Ident {
    fn new(name: &str) -> Self {
        Ident {
            name: name.to_string(),
            tp: None,
        }
    }
}

impl PathLookup {
    fn new(paths: Vec<Ident>) -> Self {
        PathLookup { paths, tp: None }
    }
}

impl Expr {
    pub fn get_type(&self) -> Option<&Type> {
        match self {
            Expr::FnCall(ref call) => call.tp.as_ref(),
            Expr::Ident(ref id) => id.tp.as_ref(),
            Expr::PathLookup(ref path) => path.tp.as_ref(),
            Expr::Expr(ref expr) => expr.get_type(),
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

//
// public trait for visitor
//

pub trait Visitor<T> {
    fn visit_statement(&mut self, s: &Statement) -> T;
    fn visit_let_statement(&mut self, s: &LetStatement) -> T;
    fn visit_assign_statement(&mut self, s: &AssignStatement) -> T;
    fn visit_expr(&mut self, s: &Expr) -> T;
    fn visit_ident(&mut self, s: &Ident) -> T;
    fn visit_fn_call(&mut self, s: &FnCall) -> T;
    fn visit_path_lookup(&mut self, s: &PathLookup) -> T;
}

pub trait VisitorMut<T> {
    fn visit_statement(&mut self, s: &mut Statement) -> T;
    fn visit_let_statement(&mut self, s: &mut LetStatement) -> T;
    fn visit_assign_statement(&mut self, s: &mut AssignStatement) -> T;
    fn visit_expr(&mut self, s: &mut Expr) -> T;
    fn visit_ident(&mut self, s: &mut Ident) -> T;
    fn visit_fn_call(&mut self, s: &mut FnCall) -> T;
    fn visit_path_lookup(&mut self, s: &mut PathLookup) -> T;
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

    fn process_expr(pair: Pair<Rule>) -> AstResult<Expr> {
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

    fn process_ident(pair: Pair<Rule>) -> AstResult<Ident> {
        debug_assert!(Rule::ident == pair.as_rule());
        Ok(Ident::new(pair.as_str()))
    }

    fn process_fncall(pair: Pair<Rule>) -> AstResult<FnCall> {
        debug_assert!(Rule::fn_call == pair.as_rule());

        let mut pairs = pair.into_inner();
        let mut r = FnCall {
            name: pairs.next().unwrap().as_str().to_string(),
            args: Vec::new(),
            tp: None,
        };

        for arg in pairs {
            r.args.push(Box::new(process_expr(arg)?));
        }

        Ok(r)
    }

    fn process_path_lookup(pair: Pair<Rule>) -> AstResult<PathLookup> {
        debug_assert!(Rule::path_lookup == pair.as_rule());

        let pairs = pair.into_inner();
        let r = PathLookup::new(pairs.map(|arg| process_ident(arg).unwrap()).collect());
        debug_assert!(!r.paths.is_empty());
        Ok(r)
    }
}
