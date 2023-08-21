compile: example_a


# linker flag is -ll not -lfl.
#
# debian: sudo apt install libfl-dev
# macOS: default library is installed,
example_a: lex.yy.c
	${CC} -o example_a $< -ll

lex.yy.c: example_a.l
	flex $<

run_example_a: example_a
	echo "begin hello thanks end" | ./example_a
