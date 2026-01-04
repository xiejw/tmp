run:
	cargo build && \
	mkdir -p .build && \
	clang -std=c11 -o .build/a.out main.c .build/debug/libffi.a && \
	strip .build/a.out && \
	.build/a.out
