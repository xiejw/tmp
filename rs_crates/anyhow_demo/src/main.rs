// See https://antoinerr.github.io/blog-website/2023/01/28/rust-anyhow.html
//
#![allow(dead_code)]
use anyhow::Result;
use anyhow::anyhow;

// === --- Demo how to use Result ---------------------------------------------
fn string_error() -> Result<()> {
    Ok(())
}

fn io_error() -> Result<()> {
    Ok(())
}

fn any_error() -> Result<()> {
    string_error()?;
    io_error()?;
    Ok(())
}

// === --- Demo how to use anyhow! ---------------------------------------------

#[derive(Debug)]
struct MyError;

impl std::fmt::Display for MyError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Bad :(")
    }
}

#[derive(Debug)]
enum MyErrors {
    LetsFixThisTomorrowError,
    ThisDoesntLookGoodError,
    ImmaGetFiredError,
}

impl std::fmt::Display for MyErrors {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::LetsFixThisTomorrowError => write!(f, "Doesn't look too bad"),
            Self::ThisDoesntLookGoodError => write!(f, "Let's get to work"),
            Self::ImmaGetFiredError => write!(f, "Wish me luck"),
        }
    }
}

fn failing_function() -> Result<String> {
    // MyError must implement Debug and Display
    let _err1: Result<String> = Err(anyhow!("Oh no!"));
    let _err2: Result<String> = Err(anyhow!(MyError));
    Err(anyhow!(MyErrors::LetsFixThisTomorrowError))
}

fn main() {
    println!("Hello, anyhow!");

    // === --- Demo how to downcast_ref ----------------------------------------
    match failing_function() {
        Ok(s) => println!("{s}"),
        Err(e) => match e.downcast_ref() {
            Some(MyErrors::LetsFixThisTomorrowError) => println!("bingo"),
            Some(MyErrors::ThisDoesntLookGoodError) => (),
            Some(MyErrors::ImmaGetFiredError) => (),
            None => (),
        },
    }
    // === --- Demo how to panic -----------------------------------------------
    failing_function().expect("abc");
}
