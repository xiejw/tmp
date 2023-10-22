/// dag module which is used to build the computation IR.
pub mod dag {

    use derivative::Derivative;
    use std::fmt;

    #[derive(Derivative)]
    #[derivative(Debug)]
    struct Scope {
        name: String,
        level: isize,
        #[derivative(Debug = "ignore")]
        parent: Option<Box<Scope>>,
    }

    pub struct Builder {
        scope_cur: Box<Scope>,
    }

    impl Builder {
        pub fn new() -> Builder {
            Builder {
                scope_cur: Box::new(Scope {
                    name: "root".to_string(),
                    level: 0,
                    parent: None,
                }),
            }
        }
    }

    /// provide the scope support.
    impl Builder {
        pub fn enter_scope(&mut self, name: &str) {
            let new_name = format!("{}/{}", &self.scope_cur.name, name);
            let new_level = self.scope_cur.level + 1;
            let mut scope = Box::new(Scope {
                name: new_name,
                level: new_level,
                parent: None,
            });

            std::mem::swap(&mut scope, &mut self.scope_cur);
            self.scope_cur.parent = Some(scope);
        }

        pub fn pop_scope(&mut self) {
            let mut scope = None;
            std::mem::swap(&mut scope, &mut self.scope_cur.parent);
            self.scope_cur = scope.unwrap();
        }
    }

    impl fmt::Display for Builder {
        fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
            write!(f, "Builder({:?})", self.scope_cur)
        }
    }
}

fn main() {
    println!("=========");
    println!("llama.rs!");
    println!("");

    let mut b = dag::Builder::new();
    println!("{}", b);

    b.enter_scope("a");
    println!("{}", b);

    b.enter_scope("b");
    println!("{}", b);

    b.pop_scope();
    println!("{}", b);

    b.pop_scope();
    println!("{}", b);
}
