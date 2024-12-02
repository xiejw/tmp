MK        = make

clean:
	go run ~/Workspace/y/tools/delete_unused_dirs.go

test:
	${MK} -C 2024/2024-pcre2 run

