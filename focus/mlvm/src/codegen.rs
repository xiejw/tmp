use crate::ast::Tree;
use std::error::Error;

pub fn compile(ast: &Tree) -> Result<(), Box<dyn Error>> {
    // check type
    // check fn signature
    // check var def
    let mut codegen = internal::CodeGen::default();
    codegen.run(ast)
}

mod internal {
    use crate::ast::{
        AssignStatement, Expr, FnCall, Ident, LetStatement, PathLookup, Statement, Tree, Type,
        Visitor,
    };
    use std::collections::HashMap;
    use std::default::Default;
    use std::error::Error;

    #[derive(Default)]
    pub struct CodeGen {
        id_assignments: HashMap<String, i32>,
        str_assignments: HashMap<String, i32>,

        rev_str_assignments: Vec<String>,
        code_bytes: Vec<CodeBytes>,
    }

    #[repr(u8)]
    #[derive(Debug, Clone, Copy)]
    pub enum Code {
        Pop = 0, // Pop the stack
        Tload,   // tensor load
        Tstor,   // tensor store
        Pload,   // path load
        Fcall,
    }

    #[derive(Clone, Debug)]
    pub enum CodeBytes {
        C0(Code),
        C1T(Code, i32),
        C1S(Code, i32),
    }

    impl CodeGen {
        pub fn run(&mut self, ast: &Tree) -> Result<(), Box<dyn Error>> {
            for s in ast.statements.iter() {
                self.visit_statement(s);
            }
            for (i, s) in self.rev_str_assignments.iter().enumerate() {
                println!("=(str)>  {:3}: '{}'", i, s);
            }
            for (i, s) in self.code_bytes.iter().enumerate() {
                println!("=(ins)>  {:3}: {:?}", i, s);
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
            self.generate_code_for_tensor(Code::Tstor, &s.ident.name);
        }

        fn visit_assign_statement(&mut self, s: &AssignStatement) {
            self.visit_expr(&s.expr);
            self.visit_ident(&s.ident);
            self.generate_code_for_tensor(Code::Tstor, &s.ident.name);
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
            self.generate_code_for_str(Code::Fcall, &f.name);
        }

        fn visit_ident(&mut self, i: &Ident) {
            match i.tp.unwrap() {
                Type::Tensor => self.generate_code_for_tensor(Code::Tload, &i.name),
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
            self.generate_code_for_str(Code::Pload, &path);
        }
    }

    impl CodeGen {
        fn generate_code_for_tensor(&mut self, c: Code, ident: &str) {
            let id = self.gen_or_assign_id_for_tensor(ident);
            self.save_code(CodeBytes::C1T(c, id))
        }

        fn generate_code_for_str(&mut self, c: Code, arg: &str) {
            let id = self.gen_or_assign_id_for_str(arg);
            self.save_code(CodeBytes::C1S(c, id))
        }

        fn generate_code_only(&mut self, c: Code) {
            self.save_code(CodeBytes::C0(c))
        }

        fn gen_or_assign_id_for_tensor(&mut self, ident: &str) -> i32 {
            let new_id = self.id_assignments.len().try_into().unwrap();
            *self
                .id_assignments
                .entry(ident.to_string())
                .or_insert(new_id)
        }

        fn gen_or_assign_id_for_str(&mut self, ident: &str) -> i32 {
            let new_id = self.str_assignments.len().try_into().unwrap();
            let id = *self
                .str_assignments
                .entry(ident.to_string())
                .or_insert(new_id);
            if id == self.rev_str_assignments.len().try_into().unwrap() {
                self.rev_str_assignments.push(ident.to_string());
            }
            id
        }

        fn save_code(&mut self, code_byte: CodeBytes) {
            self.code_bytes.push(code_byte);
        }
    }
}
