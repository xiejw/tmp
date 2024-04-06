use std::error::Error;

use crate::ast::*;

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

trait Visitor<T> {
    fn visit_statement(&mut self, s: &Statement) -> T;
    fn visit_let_statement(&mut self, s: &LetStatement) -> T;
    fn visit_assign_statement(&mut self, s: &AssignStatement) -> T;
    fn visit_expr(&mut self, s: &Expr) -> T;
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
        self.generate_code(format!("assign to {}", s.ident.name).as_str());
    }

    fn visit_expr(&mut self, e: &Expr) {
        match e {
            Expr::FnCall(ref fn_call) => {
                for arg in fn_call.args.iter().rev() {
                    self.visit_expr(arg.as_ref());
                }
                self.generate_code(format!("lookup the fn: {}", fn_call.name).as_str());
                self.generate_code("fn call");
            }
            Expr::PathLookup(ref lookup) => {
                self.generate_code("use path scope: global");
                let path = lookup
                    .paths
                    .iter()
                    .map(|x| x.name.clone())
                    .collect::<Vec<_>>()
                    .join("/");
                self.generate_code(format!("lookup the path: {}", path).as_str());
            }
            Expr::Ident(ref ident) => {
                self.generate_code(format!("lookup the ident: {}", ident.name).as_str())
            }
            Expr::Expr(ref expr) => self.visit_expr(expr.as_ref()),
        }
    }
}

impl CodeGen {
    fn generate_code(&self, fake_code: &str) {
        println!("-> {}", fake_code)
    }
}
