#[no_mangle]
pub extern "C" fn rust_fn(i: cty::c_int) {
    println!("hello {}", i)
}

