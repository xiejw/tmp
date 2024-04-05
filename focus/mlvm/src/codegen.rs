use std::error::Error;

use crate::ast::*;

pub fn compile(ast: &Tree) -> Result<(), Box<dyn Error>> {
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
        println!("generate expr");
        self.visit_expr(&s.expr);
        println!("and assign to {}", s.ident.name);
    }

    fn visit_assign_statement(&mut self, s: &AssignStatement) {
        println!("generate expr");
        self.visit_expr(&s.expr);
        println!("and assign to {}", s.ident.name);
    }

    fn visit_expr(&mut self, e: &Expr) {
        match e {
            Expr::Ident(ref ident) => println!("lookup the ident {}", ident.name),
            _ => println!("handle other kind of expr"),
        }
    }
}
