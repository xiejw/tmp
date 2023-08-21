compile: example_a

# macOS: -ll not -lfl
example_a: lex.yy.c
	${CC} -o example_a $< -ll

lex.yy.c: example_a.l
	flex $<

run_example_a: example_a
	echo "begin hello thanks end" | ./example_a
