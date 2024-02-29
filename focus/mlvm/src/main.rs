use pest::Parser;
use pest_derive::Parser;

#[derive(Parser)]
//#[grammar = "csv.pest"]
#[grammar_inline = r#"
number = { ASCII_DIGIT+ }                // one or more decimal digits
enclosed = { "(.." ~ number ~ "..)" }    // for instance, "(..6472..)"
sum = { number ~ " + " ~ number }        // for instance, "1362 + 12"
"#]
pub struct CSVParser;

fn main() {
    let pairs = CSVParser::parse(Rule::sum, "1773 + 1362")
        .unwrap()
        .next()
        .unwrap()
        .into_inner();

    let numbers = pairs
        .clone()
        .map(|pair| str::parse(pair.as_str()).unwrap())
        .collect::<Vec<i32>>();
    assert_eq!(vec![1773, 1362], numbers);

    for (found, expected) in pairs.zip(vec!["1773", "1362"]) {
        assert_eq!(Rule::number, found.as_rule());
        println!("str => {}", found.as_str());
        assert_eq!(expected, found.as_str());
    }
}
