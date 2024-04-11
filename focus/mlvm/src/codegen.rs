use crate::ast::{
    AssignStatement, Expr, FnCall, Ident, LetStatement, PathLookup, Statement, Tree, Visitor,
};
use std::error::Error;

pub fn compile(ast: &Tree) -> Result<(), Box<dyn Error>> {
    // check type
    // check fn signature
    // check var def
    let mut codegen = CodeGen {};
    for s in ast.statements.iter() {
        codegen.visit_statement(s);
    }
    Ok(())
}

struct CodeGen {}

impl Visitor<()> for CodeGen {
    fn visit_statement(&mut self, s: &Statement) {
        match s {
            Statement::Let(ref let_s) => self.visit_let_statement(let_s.as_ref()),
            Statement::Assign(ref assign_s) => self.visit_assign_statement(assign_s.as_ref()),
        }
    }

    fn visit_let_statement(&mut self, s: &LetStatement) {
        self.visit_expr(&s.expr);
        self.generate_code(format!("assign to: {}", s.ident.name).as_str());
    }

    fn visit_assign_statement(&mut self, s: &AssignStatement) {
        self.visit_expr(&s.expr);
        self.visit_ident(&s.ident);
        self.generate_code(format!("assign to: {}", s.ident.name).as_str());
    }

    fn visit_expr(&mut self, e: &Expr) {
        match e {
            Expr::FnCall(ref f) => self.visit_fn_call(f),
            Expr::PathLookup(ref lookup) => self.visit_path_lookup(lookup),
            Expr::Ident(ref ident) => self.visit_ident(ident),
            Expr::Expr(ref expr) => self.visit_expr(expr.as_ref()),
        }
    }

    fn visit_ident(&mut self, i: &Ident) {
        self.generate_code(format!("lookup the ident: {}", i.name).as_str())
    }

    fn visit_fn_call(&mut self, f: &FnCall) {
        for arg in f.args.iter().rev() {
            self.visit_expr(arg.as_ref());
        }
        self.generate_code(format!("lookup the fn: {}", f.name).as_str());
        self.generate_code("fn call");
    }

    fn visit_path_lookup(&mut self, lookup: &PathLookup) {
        self.generate_code("use path scope: global");
        let path = lookup
            .paths
            .iter()
            .map(|x| x.name.clone())
            .collect::<Vec<_>>()
            .join("/");
        self.generate_code(format!("lookup the path: {}", path).as_str());
    }
}

impl CodeGen {
    fn generate_code(&self, fake_code: &str) {
        println!("-> {}", fake_code)
    }
}
