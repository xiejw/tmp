use fancy_regex::Regex;

fn main() {
    println!("Rust Code\n");
    print_match_or_not(r"a[bc]", "ab");
    print_match_or_not(r"a[bc]", "ad");
    print_match_or_not(r"a(?!b)", "a");
    print_match_or_not(r"a(?!b)", "ab");
    print_match_or_not(r"a(?!b)", "ac");
    print_match_or_not(r"a+(?=bc)", "aaaadabcaadbc");
    print_match_or_not(r"a+(?!bc)", "abcaadbc");
    print_match_or_not(r"\s+(?!\S)", "World   ");
    print_match_or_not(r"\s+(?:[^\S])", "World   ");
    print_match_or_not(r"\s+(?!\S)", "   World");
    print_match_or_not(r"\s+(?:[^\S])", "   World");
    print_match_or_not(r"\s+(?!\S)", "Hello   World  ");
    print_match_or_not(
        r" ?[^\s\p{L}\p{N}]+[\r\n]*|\s*[\r\n]+|\s+(?!\S)|\s+",
        "Hello   World  ",
    );
    print_match_or_not(r"\s+(?:[^\S])", "Hello   World  ");
    print_match_or_not(r"\s+", "Hello   World  ");
}

fn print_match_or_not(pat: &'static str, s: &'static str) {
    let re = Regex::new(pat).unwrap();
    let result = re.find(s);

    let match_option = result.unwrap();

    if !match_option.is_some() {
        println!("false With pat `{}` and str `{}`", pat, s);
        return;
    }
    let m = match_option.unwrap();

    println!(
        " true With pat `{}` and str `{}`. Group: `{}` idx: {} len: {}",
        pat,
        s,
        m.as_str(),
        m.start(),
        m.end() - m.start()
    );
}
