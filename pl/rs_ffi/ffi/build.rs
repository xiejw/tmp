fn main() {
    // Tell Cargo that if the given file changes, to rerun this build script.
    println!("cargo:rustc-link-lib=stdc++");
    println!("cargo:rustc-link-search=../cc/.build");
    println!("cargo:rustc-link-arg=-Wl,-rpath=/home/xiejw/Workspace/build/torch/install/lib/");
    println!("cargo:rustc-link-search=/home/xiejw/Workspace/build/torch/install/lib/");
    println!("cargo:rustc-link-lib=torch");
    println!("cargo:rustc-link-lib=torch_cpu");
    println!("cargo:rustc-link-lib=c10");
    println!("cargo:rustc-link-lib=kineto");
    println!("cargo:rerun-if-changed=build.rs");
}
