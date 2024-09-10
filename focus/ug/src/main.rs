mod cc;

use crate::cc::Compiler;

fn main() {
    let s = "Hello, 世界!";
    cc::new_compiler().compile(s).unwrap();
}
