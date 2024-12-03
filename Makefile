MK              = make

clean:
	go run ~/Workspace/y/tools/delete_unused_dirs.go

test: check_deps
	${MK} -C 2024/2024-c23
	${MK} -C 2024/2024-pcre2
	${MK} -C 2024/2024-sqlite3

check_deps:
	which pkg-config || (echo "install pkg-config, e.g., brew install pkgconfig"; false)

