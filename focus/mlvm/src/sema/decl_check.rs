use super::{AnalysisError, AnalysisResult, Context, Pass};
use crate::ast::Visitor;
use crate::ast::{AssignStatement, Expr, FnCall, Ident, LetStatement, PathLookup, Statement, Tree};
use std::collections::HashSet;

pub struct DeclCheck<'a> {
    scope: HashSet<String>,
    ctx: &'a Context,
}

pub fn new(ctx: &Context) -> DeclCheck {
    DeclCheck {
        scope: HashSet::new(),
        ctx,
    }
}

impl<'a> Pass<()> for DeclCheck<'a> {
    fn run(&mut self, tree: &mut Tree) -> AnalysisResult<()> {
        for s in tree.statements.iter() {
            self.visit_statement(s)?;
        }
        Ok(())
    }
}

impl<'a> Visitor<AnalysisResult<()>> for DeclCheck<'a> {
    fn visit_statement(&mut self, s: &Statement) -> AnalysisResult<()> {
        match s {
            Statement::Let(ref let_s) => self.visit_let_statement(let_s.as_ref()),
            Statement::Assign(ref assign_s) => self.visit_assign_statement(assign_s.as_ref()),
            Statement::Expr(ref e) => self.visit_expr(e),
        }
    }

    fn visit_let_statement(&mut self, s: &LetStatement) -> AnalysisResult<()> {
        self.visit_expr(&s.expr)?;

        let name = &s.ident.name;
        if self.scope.contains(name) {
            Err(AnalysisError::new(format!(
                "var `{}` has been defined already",
                name
            )))
        } else {
            self.scope.insert(name.to_string());
            Ok(())
        }
    }

    fn visit_assign_statement(&mut self, s: &AssignStatement) -> AnalysisResult<()> {
        self.visit_expr(&s.expr)?;
        self.visit_ident(&s.ident)
    }

    fn visit_ident(&mut self, i: &Ident) -> AnalysisResult<()> {
        let name = &i.name;
        if !self.scope.contains(name) {
            Err(AnalysisError::new(format!(
                "var `{}` has not been defined yet",
                name
            )))
        } else {
            Ok(())
        }
    }

    fn visit_fn_call(&mut self, f: &FnCall) -> AnalysisResult<()> {
        for arg in f.args.iter().rev() {
            self.visit_expr(arg.as_ref())?;
        }
        if !self.ctx.fn_sigs.contains_key(&f.name) {
            Err(AnalysisError::new(format!(
                "fn_name `{}` has not been defined yet",
                f.name
            )))
        } else {
            Ok(())
        }
    }

    fn visit_path_lookup(&mut self, _p: &PathLookup) -> AnalysisResult<()> {
        Ok(())
    }

    fn visit_expr(&mut self, e: &Expr) -> AnalysisResult<()> {
        match e {
            Expr::FnCall(ref fn_call) => self.visit_fn_call(fn_call)?,
            Expr::PathLookup(ref path) => self.visit_path_lookup(path)?,
            Expr::Ident(ref ident) => self.visit_ident(ident)?,
            Expr::Expr(ref expr) => self.visit_expr(expr.as_ref())?,
        }
        Ok(())
    }
}
