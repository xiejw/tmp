use std::collections::HashSet;
use std::process::Command;

fn main() {
    println!("Hello, world!");
    let output = Command::new("brew")
        .arg("leaves")
        .output()
        .expect("failed to execute process");

    let o = std::str::from_utf8(&output.stdout).unwrap().trim();
    println!("output: ###\n{}\n###\n", o);

    for item in o.lines() {
        println!("output: -> `{}`", item);
    }

    let mut expect = HashSet::new();
}
