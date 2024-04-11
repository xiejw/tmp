use crate::ast::Tree;
use std::error::Error;

pub fn compile(ast: &Tree) -> Result<(), Box<dyn Error>> {
    // check type
    // check fn signature
    // check var def
    let mut codegen = internal::CodeGen {};
    codegen.run(ast)
}

mod internal {
    use crate::ast::{
        AssignStatement, Expr, FnCall, Ident, LetStatement, PathLookup, Statement, Tree, Type,
        Visitor,
    };
    use std::error::Error;

    pub struct CodeGen {}

    impl CodeGen {
        pub fn run(&mut self, ast: &Tree) -> Result<(), Box<dyn Error>> {
            for s in ast.statements.iter() {
                self.visit_statement(s);
            }
            Ok(())
        }
    }

    impl Visitor<()> for CodeGen {
        fn visit_statement(&mut self, s: &Statement) {
            match s {
                Statement::Let(ref let_s) => self.visit_let_statement(let_s.as_ref()),
                Statement::Assign(ref assign_s) => self.visit_assign_statement(assign_s.as_ref()),
                Statement::Expr(ref e) => {
                    self.visit_expr(e.as_ref());
                    self.generate_code_only(Code::Pop)
                }
            }
        }

        fn visit_let_statement(&mut self, s: &LetStatement) {
            self.visit_expr(&s.expr);
            self.generate_code(Code::Tstor, &s.ident.name);
        }

        fn visit_assign_statement(&mut self, s: &AssignStatement) {
            self.visit_expr(&s.expr);
            self.visit_ident(&s.ident);
            self.generate_code(Code::Tstor, &s.ident.name);
        }

        fn visit_expr(&mut self, e: &Expr) {
            match e {
                Expr::FnCall(ref f) => self.visit_fn_call(f),
                Expr::PathLookup(ref lookup) => self.visit_path_lookup(lookup),
                Expr::Ident(ref ident) => self.visit_ident(ident),
                Expr::Expr(ref expr) => self.visit_expr(expr.as_ref()),
            }
        }

        fn visit_fn_call(&mut self, f: &FnCall) {
            for arg in f.args.iter().rev() {
                self.visit_expr(arg.as_ref());
            }
            self.generate_code(Code::Fcall, &f.name);
        }

        fn visit_ident(&mut self, i: &Ident) {
            match i.tp.unwrap() {
                Type::Tensor => self.generate_code(Code::Tload, &i.name),
                Type::Path => panic!("should not reach"),
            }
        }

        fn visit_path_lookup(&mut self, lookup: &PathLookup) {
            let path = lookup
                .paths
                .iter()
                .map(|x| x.name.clone())
                .collect::<Vec<_>>()
                .join("/");
            self.generate_code(Code::Pload, &path);
        }
    }

    #[derive(Debug)]
    enum Code {
        Tload, // tensor load
        Tstor, // tensor store
        Pload, // path load
        Fcall,
        Pop, // Pop the stack
    }

    impl CodeGen {
        fn generate_code(&self, c: Code, arg: &str) {
            println!("-> {:6} '{}'", format!("{:?}", c), arg)
        }

        fn generate_code_only(&self, c: Code) {
            println!("-> {:6}", format!("{:?}", c))
        }
    }
}
