// Algorithm:
// - Loop until all types are found for all identities
// - Crash if the iteration cannot make progress.
// - Propogate and verify
//
//   - The fn signature is the anchor point, which can propagate the type to other
//     identities. The steps are
//     1. For ident, type must be same if type is present, otherwise, the type is
//        set in scope. The visit_ident will pick it up.
//     2. For path_lookup, type must be same if type is present; otherwise, the
//        type is set immediately.
//     1. For expr, type must be same if type is present; otherwise, keep
//        propagating.
//   - Assignments are another point to propagate types in both ways.
//
use super::{AnalysisError, AnalysisResult, Context, Pass};
use crate::ast::VisitorMut;
use crate::ast::{
    AssignStatement, Expr, FnCall, Ident, LetStatement, PathLookup, Statement, Tree, Type,
};
use std::borrow::BorrowMut;
use std::collections::HashMap;

pub struct TypeCheck<'a> {
    ctx: &'a Context,
    scope: HashMap<String, Type>,
    all_good: bool,
    any_progress: bool,
}

pub fn new(ctx: &Context) -> TypeCheck {
    TypeCheck {
        ctx,
        scope: HashMap::new(),
        all_good: false,
        any_progress: false,
    }
}

impl<'a> Pass<()> for TypeCheck<'a> {
    fn run(&mut self, tree: &mut Tree) -> AnalysisResult<()> {
        loop {
            self.all_good = true;
            self.any_progress = false;

            for s in tree.statements.iter_mut() {
                self.visit_statement(s)?;
            }

            if self.all_good {
                break Ok(());
            }

            if !self.any_progress {
                // TODO improve the error reporting for the first missing type.
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
        self.visit_ident(&mut s.ident)?;
        self.visit_expr(&mut s.expr)?;
        self.propogate_assignment(&mut s.ident, &mut s.expr)
    }

    fn visit_assign_statement(&mut self, s: &mut AssignStatement) -> AnalysisResult<TypeResult> {
        self.visit_ident(&mut s.ident)?;
        self.visit_expr(&mut s.expr)?;
        self.propogate_assignment(&mut s.ident, &mut s.expr)
    }

    fn visit_ident(&mut self, i: &mut Ident) -> AnalysisResult<TypeResult> {
        match (i.tp, self.scope.get(&i.name)) {
            (None, None) => {
                self.all_good = false;
                Ok(None)
            }
            (Some(ref ident_tp), Some(recorded_tp)) => {
                if *ident_tp == *recorded_tp {
                    Ok(Some(*ident_tp))
                } else {
                    Err(error(&format!(
                        "mismatch type for {:?} vs {:?}",
                        ident_tp, recorded_tp
                    )))
                }
            }
            (Some(ref tp), None) => {
                self.all_good = false;
                self.any_progress = true;
                self.scope.insert(i.name.clone(), *tp);
                Ok(Some(*tp))
            }
            (None, Some(tp)) => {
                self.all_good = false;
                self.any_progress = true;
                i.tp = Some(*tp);
                Ok(Some(*tp))
            }
        }
    }

    fn visit_fn_call(&mut self, f: &mut FnCall) -> AnalysisResult<TypeResult> {
        // Algorithrm
        // - look up fn_sig
        // - check ret type
        // - propagating to all args.
        let fn_sigs = &self.ctx.fn_sigs;
        let sig = fn_sigs.get(&f.name);
        if let None = sig {
            return Err(error(&format!("fn {} is not defined", f.name)));
        }
        let sig = sig.unwrap();

        // handle return type
        match f.tp {
            None => {
                f.tp = Some(sig.ret);
                self.all_good = false;
                self.any_progress = true;
            }
            Some(tp_in_f) if tp_in_f != sig.ret => {
                return Err(error(&format!(
                    "fn {} return type mismatch {:?} vs {:?}",
                    f.name, sig.ret, tp_in_f
                )));
            }
            _ => (),
        }

        if f.args.len() != sig.args.len() {
            return Err(error(&format!("fn call {} arg count mismatch", f.name)));
        }

        // pass visit all args first
        for arg in f.args.iter_mut() {
            self.visit_expr(arg)?;
        }

        for (expected, arg) in sig.args.iter().zip(f.args.iter_mut()) {
            self.propagate_type_to_expr(arg, *expected)?;
        }

        Ok(Some(sig.ret))
    }

    fn visit_path_lookup(&mut self, p: &mut PathLookup) -> AnalysisResult<TypeResult> {
        if let None = p.tp {
            self.all_good = false;
        }
        Ok(p.tp)
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
impl<'a> TypeCheck<'a> {
    fn propagate_type_to_ident(&mut self, i: &mut Ident, tp: Type) -> AnalysisResult<TypeResult> {
        match i.tp {
            None => {
                self.all_good =  false;
                self.any_progress = true;
                i.tp = Some(tp);
            },
            Some(i_tp) if i_tp != tp => {
                return Err(error(&format!(
                    "ident {} type mismatch {:?} vs {:?}",
                    i.name, tp, i_tp
                )));
            },
            _ => (),
        }
        Ok(Some(tp))
    }

    fn propagate_type_to_fn_call(&mut self, f: &mut FnCall, tp: Type) -> AnalysisResult<()> {
        match f.tp {
            None => {
                panic!("should not happen");
            },
            Some(f_tp) if f_tp != tp => {
                return Err(error(&format!(
                    "fn {} return type mismatch {:?} vs {:?}",
                    f.name, tp, f_tp
                )));
            },
            _ => Ok(()),
        }
    }

    fn propagate_type_to_expr(&mut self, e: &mut Expr, tp: Type) -> AnalysisResult<()> {
        match e {
            Expr::FnCall(ref mut fn_call) => self.propagate_type_to_fn_call(fn_call, tp)?,
            Expr::PathLookup(ref mut path) => (),
            Expr::Ident(ref mut ident) => {
                self.propagate_type_to_ident(ident, tp)?;
            }
            Expr::Expr(ref mut expr) => {
                self.propagate_type_to_expr(expr, tp)?;
            }
        }
        Ok(())
    }

    fn propogate_assignment(&mut self, i: &mut Ident, e: &mut Expr) -> AnalysisResult<TypeResult> {
        let lhs_type = i.tp;
        let rhs_type = e.get_type();

        match (lhs_type, rhs_type) {
            (None, None) => {
                self.all_good = false;
                return Ok(None);
            }
            (Some(lhs_type), Some(rhs_type)) => {
                if lhs_type == *rhs_type {
                    return Ok(Some(lhs_type));
                } else {
                    return Err(error(&format!(
                        "let statements lhs and rhs type mismatch: {:?} vs {:?}",
                        lhs_type, rhs_type
                    )));
                }
            }
            (None, Some(rhs_type)) => {
                self.all_good = false;
                self.propagate_type_to_ident(i, *rhs_type)?;
                return Ok(Some(*rhs_type));
            }
            (Some(lhs_type), None) => {
                self.all_good = false;
                self.propagate_type_to_expr(e, lhs_type)?;
                return Ok(Some(lhs_type));
            }
        }
    }
}

use std::error::Error;
fn error(msg: &str) -> Box<AnalysisError> {
    AnalysisError::new(msg.to_string())
}
