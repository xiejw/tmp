BUILD = .build
run: | ${BUILD}
	${CC} -std=c11 -o ${BUILD}/a.out main.c

${BUILD}:
	@mkdir -p ${BUILD}

# 	go run cmd/main.go

fmt:
	~/Workspace/y/tools/clang_format_all.sh .
# fmt:
# 	go fmt minitex/...
