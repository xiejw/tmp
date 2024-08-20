use crossterm::terminal::disable_raw_mode;
use crossterm::terminal::enable_raw_mode;
use std::io::{self, Read};

struct Term {}

impl Term {
    fn default() -> Term {
        enable_raw_mode().unwrap();
        Term {}
    }
}

impl Drop for Term {
    fn drop(&mut self) {
        disable_raw_mode().unwrap();
    }
}

fn main() {
    let _term = Term::default();

    println!("Type q to quit.\r");

    for b in io::stdin().bytes() {
        let b = b.unwrap();
        let c = b as char;
        if c.is_control() {
            println!("Binary: {0:08b} ASCII: {0:#03} \r", b);
        } else {
            println!("Binary: {0:08b} ASCII: {0:#03} Character: {1:#?}\r", b, c);
        }
        if c == 'q' {
            break;
        }
    }
}
