#[link(name="test_app")] // "../cc/.build/libtest_app.a")]
extern {
fn hello();
}

fn main() {
    println!("Hello, world!");
    unsafe { hello(); }
}
