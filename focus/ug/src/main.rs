mod cc;

use crate::cc::Compiler;

fn main() {
    let s = r##"Hello, 世界! 123

        Program = Layer*

        "##;
    cc::new_compiler().compile(s).unwrap();
}
