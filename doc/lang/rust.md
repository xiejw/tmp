## Rust

## Rustup and Install

- https://www.rust-lang.org/tools/install
- https://rust-lang.github.io/rustup/basics.html

```
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
rustup component remove rust-docs

rustup component list | grep installed     # identify the installed components.
rustup update                              # upgrade rust.
rustup doc                                 # open doc, if installed.
```

## Cargo

```
cargo new <name> --bin --vcs none
cargo update            # updates all dependencies
cargo update -p regex   # updates just “regex”
cargo doc --open
```

Update `target-dir` in
[build](https://doc.rust-lang.org/cargo/reference/config.html) section of
config.
```
# $PROJECT/.cargo/config
[build]
target-dir = ".build"
```


## LLVM-IR

```
rustc --emit=llvm-ir --crate-type=lib -O main.rs
cargo rustc -- --emit=llvm-ir
```

## Mdbook

- https://rust-lang.github.io/mdBook/index.html

```
cargon install mdbook
mdbook serve --open
mdbook init <name_folder)
mdbook build
```

## Books
- https://doc.rust-lang.org/cargo/index.html
- https://rust-lang.github.io/rustup/
