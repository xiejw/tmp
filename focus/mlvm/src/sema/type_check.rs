// Algorithm:
// Loop until all types are found.
// Crash if the iteration cannot make progress.
// Verify signatures
//
use super::{AnalysisError, AnalysisResult, Context, Pass};
use crate::ast::VisitorMut;
use crate::ast::{
    AssignStatement, Expr, FnCall, Ident, LetStatement, PathLookup, Statement, Tree, Type,
};
use std::borrow::BorrowMut;
use std::collections::HashMap;

pub struct TypeCheck<'a> {
    scope: HashMap<String, Type>,
    ctx: &'a Context,
    all_good: bool,
    any_progress: bool,
}

pub fn new(ctx: &Context) -> TypeCheck {
    TypeCheck {
        scope: HashMap::new(),
        ctx,
        all_good: false,
        any_progress: false,
    }
}

impl<'a> Pass<()> for TypeCheck<'a> {
    fn run(&mut self, tree: &mut Tree) -> AnalysisResult<()> {
        loop {
            self.all_good = false;
            self.any_progress = false;
            for s in tree.statements.iter_mut() {
                self.visit_statement(s)?;
            }

            if self.all_good {
                break Ok(());
            }

            if !self.any_progress {
                break Err(AnalysisError::new("type inf cannot be done".to_string()));
            }
        }
    }
}

type TypeResult = Option<Type>;

impl<'a> VisitorMut<AnalysisResult<TypeResult>> for TypeCheck<'a> {
    fn visit_statement(&mut self, s: &mut Statement) -> AnalysisResult<TypeResult> {
        match s {
            Statement::Let(ref mut let_s) => self.visit_let_statement(let_s),
            Statement::Assign(ref mut assign_s) => self.visit_assign_statement(assign_s),
        }
    }

    fn visit_let_statement(&mut self, s: &mut LetStatement) -> AnalysisResult<TypeResult> {
        let rhs_type = self.visit_expr(&mut s.expr)?;

        //let name = &s.ident.name;
        //if self.scope.contains_key(name) {
        //    Err(AnalysisError::new(format!(
        //        "var `{}` has been defined already",
        //        name
        //    )))
        //} else {
        //    self.scope.insert(name.to_string(), Type::Tensor);
        //    Ok(())
        //}
        Ok(None)
    }

    fn visit_assign_statement(&mut self, s: &mut AssignStatement) -> AnalysisResult<TypeResult> {
        // self.visit_expr(&mut s.expr)?;
        // self.visit_ident(&mut s.ident)
        Ok(None)
    }

    fn visit_ident(&mut self, i: &mut Ident) -> AnalysisResult<TypeResult> {
        //let name = &i.name;
        //if !self.scope.contains_key(name) {
        //    Err(AnalysisError::new(format!(
        //        "var `{}` has not been defined yet",
        //        name
        //    )))
        //} else {
        //    Ok(())
        //}
        Ok(None)
    }

    fn visit_fn_call(&mut self, f: &mut FnCall) -> AnalysisResult<TypeResult> {
        Ok(None)
        //for arg in f.args.iter_mut().rev() {
        //    self.visit_expr(arg.borrow_mut())?;
        //}
        //if !self.ctx.fn_sigs.contains_key(&f.name) {
        //    Err(AnalysisError::new(format!(
        //        "fn_name `{}` has not been defined yet",
        //        f.name
        //    )))
        //} else {
        //    Ok(())
        //}
    }

    fn visit_path_lookup(&mut self, _p: &mut PathLookup) -> AnalysisResult<TypeResult> {
        Ok(None)
    }

    fn visit_expr(&mut self, e: &mut Expr) -> AnalysisResult<TypeResult> {
        match e {
            Expr::FnCall(ref mut fn_call) => self.visit_fn_call(fn_call),
            Expr::PathLookup(ref mut path) => self.visit_path_lookup(path),
            Expr::Ident(ref mut ident) => self.visit_ident(ident),
            Expr::Expr(ref mut expr) => self.visit_expr(expr.borrow_mut()),
        }
    }
}
