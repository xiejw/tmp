use super::{AnalysisError, AnalysisResult, Pass};
use crate::ast::{AssignStatement, Expr, LetStatement, Statement, Tree, Visitor};
use std::collections::HashSet;

pub struct VarDefPass {
    var_def: HashSet<String>,
}

impl Pass<()> for VarDefPass {
    fn run(&mut self, tree: &mut Tree) -> AnalysisResult<()> {
        for s in tree.statements.iter() {
            self.visit_statement(s)?;
        }
        Ok(())
    }
}

impl Visitor<AnalysisResult<()>> for VarDefPass {
    fn visit_statement(&mut self, s: &Statement) -> AnalysisResult<()> {
        match s {
            Statement::Let(ref let_s) => self.visit_let_statement(let_s.as_ref()),
            Statement::Assign(ref assign_s) => self.visit_assign_statement(assign_s.as_ref()),
        }
    }

    fn visit_let_statement(&mut self, s: &LetStatement) -> AnalysisResult<()> {
        self.visit_expr(&s.expr)?;

        let name = &s.ident.name;
        if self.var_def.contains(name) {
            Err(AnalysisError::new(format!(
                "var `{}` has been defined already",
                name
            )))
        } else {
            self.var_def.insert(name.to_string());
            Ok(())
        }
    }

    fn visit_assign_statement(&mut self, s: &AssignStatement) -> AnalysisResult<()> {
        self.visit_expr(&s.expr)?;

        let name = &s.ident.name;
        if !self.var_def.contains(name) {
            Err(AnalysisError::new(format!(
                "var `{}` has not been defined yet",
                name
            )))
        } else {
            Ok(())
        }
    }

    fn visit_expr(&mut self, _e: &Expr) -> AnalysisResult<()> {
        // match e {
        //     Expr::FnCall(ref fn_call) => {
        //         for arg in fn_call.args.iter().rev() {
        //             self.visit_expr(arg.as_ref());
        //         }
        //         self.generate_code(format!("lookup the fn: {}", fn_call.name).as_str());
        //         self.generate_code("fn call");
        //     }
        //     Expr::PathLookup(ref lookup) => {
        //         self.generate_code("use path scope: global");
        //         let path = lookup
        //             .paths
        //             .iter()
        //             .map(|x| x.name.clone())
        //             .collect::<Vec<_>>()
        //             .join("/");
        //         self.generate_code(format!("lookup the path: {}", path).as_str());
        //     }
        //     Expr::Ident(ref ident) => {
        //         self.generate_code(format!("lookup the ident: {}", ident.name).as_str())
        //     }
        //     Expr::Expr(ref expr) => self.visit_expr(expr.as_ref()),
        // }
        Ok(())
    }
}
