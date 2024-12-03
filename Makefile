UNAME           = $(shell uname)
MK              = make

clean:
	go run ~/Workspace/y/tools/delete_unused_dirs.go

test:
ifneq ($(UNAME), Darwin)
	${MK} -C 2024/2024-c23      # macOs does not have good support for c23.
endif
	${MK} -C 2024/2024-pcre2
	${MK} -C 2024/2024-sqlite3

